#include "project.h"
#include "string.h"

/* ---------- Вказівники на функції пінів ---------- */

static void (*Column_x_SetDriveMode[3])(uint8_t mode) = {
    Column_0_SetDriveMode,
    Column_1_SetDriveMode,
    Column_2_SetDriveMode
};

static void (*Column_x_Write[3])(uint8_t value) = {
    Column_0_Write,
    Column_1_Write,
    Column_2_Write
};

static uint8 (*Row_x_Read[4])() = {
    row_0_Read,
    row_1_Read,
    row_2_Read,
    row_3_Read
};

/* ---------- Дані клавіатури ---------- */

static uint8_t keys[4][3];
static uint8_t prev_keys[4][3];

static const uint8_t keyMap[4][3] = {
    {1,  2,  3},
    {4,  5,  6},
    {7,  8,  9},
    {10, 0,  11}
};

/* ---------- Пароль ---------- */

#define PASSWORD        "20"
#define PASSWORD_LEN    2u
#define BUFFER_SIZE     16u

static char    inputBuffer[BUFFER_SIZE];
static uint8_t inputLen = 0u;

/* ---------- Макроси LED (active-low) ---------- */

#define LED_WHITE()   do { LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); } while(0)
#define LED_BLACK()   do { LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); } while(0)
#define LED_RED()     do { LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); } while(0)
#define LED_GREEN()   do { LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); } while(0)
#define LED_BLUE()    do { LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); } while(0)
#define LED_YELLOW()  do { LED_R_Write(0); LED_G_Write(0); LED_B_Write(1); } while(0)
#define LED_PURPLE()  do { LED_R_Write(0); LED_G_Write(1); LED_B_Write(0); } while(0)
#define LED_CYAN()    do { LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); } while(0)

/* ---------- Ініціалізація ---------- */

static void initMatrix(void)
{
    for (int col = 0; col < 3; col++)
        Column_x_SetDriveMode[col](Column_0_DM_DIG_HIZ);

    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 3; c++)
            keys[r][c] = prev_keys[r][c] = 1u;
}

/* ---------- Зчитування матриці ---------- */

static void readMatrix(void)
{
    for (int col = 0; col < 3; col++)
    {
        Column_x_SetDriveMode[col](Column_0_DM_STRONG);
        Column_x_Write[col](0);

        for (int row = 0; row < 4; row++)
        {
            prev_keys[row][col] = keys[row][col];
            keys[row][col]      = Row_x_Read[row]();
        }

        Column_x_SetDriveMode[col](Column_0_DM_DIG_HIZ);
    }
}

/* ---------- Назва кнопки ---------- */

static const char* getKeyName(uint8_t v)
{
    switch (v)
    {
        case 0:  return "0";
        case 1:  return "1";
        case 2:  return "2";
        case 3:  return "3";
        case 4:  return "4";
        case 5:  return "5";
        case 6:  return "6";
        case 7:  return "7";
        case 8:  return "8";
        case 9:  return "9";
        case 10: return "*";
        case 11: return "#";
        default: return "?";
    }
}

/* ---------- Колір за кнопкою ---------- */

static void setColorByKey(uint8_t v)
{
    switch (v)
    {
        case 1: case 7:  LED_RED();    SW_Tx_UART_PutString("Color: RED");    break;
        case 2: case 8:  LED_GREEN();  SW_Tx_UART_PutString("Color: GREEN");  break;
        case 3: case 9:  LED_BLUE();   SW_Tx_UART_PutString("Color: BLUE");   break;
        case 4: case 10: LED_YELLOW(); SW_Tx_UART_PutString("Color: YELLOW"); break;
        case 5: case 0:  LED_PURPLE(); SW_Tx_UART_PutString("Color: PURPLE"); break;
        case 6: case 11: LED_CYAN();   SW_Tx_UART_PutString("Color: CYAN");   break;
        default: break;
    }
}

/* ---------- Обробка пароля ---------- */

static void handlePassword(uint8_t keyVal)
{
    if (keyVal == 10) /* '*' — скидання буфера */
    {
        inputLen = 0u;
        memset(inputBuffer, 0, BUFFER_SIZE);
        SW_Tx_UART_PutString("Input cleared");
        SW_Tx_UART_PutCRLF();
        return;
    }

    if (keyVal == 11) /* '#' — підтвердження */
    {
        inputBuffer[inputLen] = '\0';

        SW_Tx_UART_PutString("Entered: ");
        SW_Tx_UART_PutString(inputBuffer);
        SW_Tx_UART_PutCRLF();

        if (inputLen == PASSWORD_LEN && strcmp(inputBuffer, PASSWORD) == 0)
            SW_Tx_UART_PutString("\n---------- Access allowed ----------\n");
        else
            SW_Tx_UART_PutString("Access denied");

        SW_Tx_UART_PutCRLF();
        inputLen = 0u;
        memset(inputBuffer, 0, BUFFER_SIZE);
        return;
    }

    /* Цифра — додаємо до буфера */
    if (inputLen < PASSWORD_LEN)
        inputBuffer[inputLen++] = getKeyName(keyVal)[0];
}

/* ---------- main ---------- */

int main(void)
{
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("=== Matrix Keyboard ===");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Password: enter digits, confirm with #, clear with *");
    SW_Tx_UART_PutCRLF();

    initMatrix();
    LED_WHITE();

    uint8_t anyPressed = 0u;

    for (;;)
    {
        readMatrix();

        uint8_t stillPressed = 0u;

        for (int row = 0; row < 4; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                uint8_t keyVal = keyMap[row][col];

                if (prev_keys[row][col] == 1u && keys[row][col] == 0u) /* натиснуто */
                {
                    SW_Tx_UART_PutString("Button ");
                    SW_Tx_UART_PutString(getKeyName(keyVal));
                    SW_Tx_UART_PutString(" pressed -> ");

                    setColorByKey(keyVal);
                    handlePassword(keyVal);

                    SW_Tx_UART_PutCRLF();
                    anyPressed = 1u;
                }

                if (prev_keys[row][col] == 0u && keys[row][col] == 1u) /* відпущено */
                {
                    SW_Tx_UART_PutString("Button ");
                    SW_Tx_UART_PutString(getKeyName(keyVal));
                    SW_Tx_UART_PutString(" released");
                    SW_Tx_UART_PutCRLF();
                }

                if (keys[row][col] == 0u)
                    stillPressed = 1u;
            }
        }

        if (anyPressed && !stillPressed)
        {
            anyPressed = 0u;
            LED_BLACK();
        }
    }
}