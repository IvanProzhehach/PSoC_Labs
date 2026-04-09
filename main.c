/* ========================================
 *
 * Лабораторна робота №4
 * Використання зсувних регістрів та семисегментних індикаторів
 *
 * Завдання 1: Відображення номера натиснутої кнопки матричної клавіатури
 * Завдання 2: Відображення символів A, b, C, d, E, F
 *             - кнопка * циклічно перемикає літери A→b→C→d→E→F→A...
 *             - кнопка # відображає прочерк (-)
 *
 * ========================================
*/
#include "project.h"

/* -------------------------------------------------------
 * Індекси в масиві LED_NUM
 * 0-9   : цифри 0..9
 * 10    : -  (прочерк)
 * 11    : A
 * 12    : b
 * 13    : C
 * 14    : d
 * 15    : E
 * 16    : F
 * 17    : .  (крапка)
 * ------------------------------------------------------- */
#define IDX_DASH  10
#define IDX_A     11
#define IDX_b     12
#define IDX_C     13
#define IDX_d     14
#define IDX_E     15
#define IDX_F     16

/* Масив бінарних кодів для семисегментного індикатора (Common Anode, gfedcba) */
static uint8_t LED_NUM[] = {
    0xC0,  // 0
    0xF9,  // 1
    0xA4,  // 2
    0xB0,  // 3
    0x99,  // 4
    0x92,  // 5
    0x82,  // 6
    0xF8,  // 7
    0x80,  // 8
    0x90,  // 9
    0xBF,  // -  (прочерк)
    0x88,  // A
    0x83,  // b
    0xC6,  // C
    0xA1,  // d
    0x86,  // E
    0x8E,  // F
    0x7F   // .
};

/* Порядок літер для циклічного перемикання кнопкою * */
static uint8_t LETTER_CYCLE[] = {IDX_A, IDX_b, IDX_C, IDX_d, IDX_E, IDX_F};
#define LETTER_CYCLE_LEN  (sizeof(LETTER_CYCLE) / sizeof(LETTER_CYCLE[0]))

/* ------------------------------------------------------- */
/* Відправка одного байту у зсувний регістр (MSB першим)   */
/* ------------------------------------------------------- */
static void FourDigit74HC595_sendData(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (0x80 >> i))
            Pin_DO_Write(1);
        else
            Pin_DO_Write(0);

        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

/* ------------------------------------------------------- */
/* Увімкнути один розряд (position) з символом digit       */
/* dot=1 — показати десяткову крапку                       */
/* ------------------------------------------------------- */
static void FourDigit74HC595_sendOneDigit(uint8_t position,
                                          uint8_t digit,
                                          uint8_t dot)
{
    /* position >= 8 — поза діапазоном, гасимо дисплей */
    if (position >= 8)
    {
        FourDigit74HC595_sendData(0xFF);
        FourDigit74HC595_sendData(0xFF);
        return;
    }

    /* Перший байт: вибір розряду (active LOW) */
    FourDigit74HC595_sendData(0xFF & ~(1 << position));

    /* Другий байт: код символу */
    if (dot)
        FourDigit74HC595_sendData(LED_NUM[digit] & 0x7F); /* bit7=0 → DP увімк. */
    else
        FourDigit74HC595_sendData(LED_NUM[digit]);

    /* Фіксуємо дані на виходах зсувного регістра */
    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

/* ------------------------------------------------------- */
/* Матриця клавіатури                                       */
/* keys[row][col] == 0 → кнопка натиснута                  */
/* ------------------------------------------------------- */
static uint8_t keys[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {10, 0, 11},   /* 10 = *, 11 = # */
};

/* Масиви покажчиків на функції роботи з колонками / рядками */
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = {
    COLUMN_0_SetDriveMode,
    COLUMN_1_SetDriveMode,
    COLUMN_2_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8_t value) = {
    COLUMN_0_Write,
    COLUMN_1_Write,
    COLUMN_2_Write
};

static uint8 (*ROW_x_Read[4])() = {
    ROW_0_Read,
    ROW_1_Read,
    ROW_2_Read,
    ROW_3_Read
};

/* Ініціалізація матриці: всі колонки у Hi-Z */
static void initMatrix(void)
{
    for (int c = 0; c < 3; c++)
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_DIG_HIZ);
}

/* Опитування матриці клавіатури */
static void readMatrix(void)
{
    uint8_t rows = sizeof(ROW_x_Read)    / sizeof(ROW_x_Read[0]);
    uint8_t cols = sizeof(COLUMN_x_Write)/ sizeof(COLUMN_x_Write[0]);

    for (int c = 0; c < cols; c++)
    {
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_STRONG);
        COLUMN_x_Write[c](0);

        for (int r = 0; r < rows; r++)
            keys[r][c] = ROW_x_Read[r]();

        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_DIG_HIZ);
    }
}

/* Виведення стану матриці у UART */
static void printMatrix(void)
{
    SW_Tx_UART_PutCRLF();
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            SW_Tx_UART_PutHexInt(keys[r][c]);
            SW_Tx_UART_PutString(" ");
        }
        SW_Tx_UART_PutCRLF();
    }
    SW_Tx_UART_PutCRLF();
}

/* ------------------------------------------------------- */
/* Таблиця відповідності: значення last_state → індекс     */
/* у LED_NUM для кнопок 0-9                                */
/* last_state: 0-9 — цифра, 10 — *, 11 — #, 12 — idle     */
/* ------------------------------------------------------- */

int main(void)
{
    CyGlobalIntEnable;

    /* Початкове відображення: показати "8" з крапкою (тест сегментів) */
    FourDigit74HC595_sendOneDigit(0, 8, 1);

    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("=== Lab 4: 7-segment + Matrix Keyboard ===");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Buttons 0-9: show digit on display");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Button *   : cycle letters A,b,C,d,E,F");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Button #   : show dash (-)");
    SW_Tx_UART_PutCRLF();

    initMatrix();

    uint8_t last_state   = 12;   /* 12 = жодна кнопка не натиснута */
    uint8_t letter_index = 0;    /* індекс у LETTER_CYCLE для кнопки * */

    SW_Tx_UART_PutString("All buttons released");
    SW_Tx_UART_PutCRLF();
    printMatrix();

    for (;;)
    {
        readMatrix();

        /* =====================================================
         * ЗАВДАННЯ 1: Відображення номера кнопки (цифри 0-9)
         * ===================================================== */

        /* --- Кнопка 1 --- */
        if (keys[0][0] == 0 && last_state != 1)
        {
            last_state = 1;
            FourDigit74HC595_sendOneDigit(0, 1, 0);
            SW_Tx_UART_PutString("Button 1 pressed -> display: 1");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(1); LED_B_Write(1);
            printMatrix();
        }
        if (keys[0][0] == 1 && last_state == 1)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 1 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 2 --- */
        if (keys[0][1] == 0 && last_state != 2)
        {
            last_state = 2;
            FourDigit74HC595_sendOneDigit(0, 2, 0);
            SW_Tx_UART_PutString("Button 2 pressed -> display: 2");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(0); LED_B_Write(1);
            printMatrix();
        }
        if (keys[0][1] == 1 && last_state == 2)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 2 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 3 --- */
        if (keys[0][2] == 0 && last_state != 3)
        {
            last_state = 3;
            FourDigit74HC595_sendOneDigit(0, 3, 0);
            SW_Tx_UART_PutString("Button 3 pressed -> display: 3");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(0);
            printMatrix();
        }
        if (keys[0][2] == 1 && last_state == 3)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 3 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 4 --- */
        if (keys[1][0] == 0 && last_state != 4)
        {
            last_state = 4;
            FourDigit74HC595_sendOneDigit(0, 4, 0);
            SW_Tx_UART_PutString("Button 4 pressed -> display: 4");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(1);
            printMatrix();
        }
        if (keys[1][0] == 1 && last_state == 4)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 4 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 5 --- */
        if (keys[1][1] == 0 && last_state != 5)
        {
            last_state = 5;
            FourDigit74HC595_sendOneDigit(0, 5, 0);
            SW_Tx_UART_PutString("Button 5 pressed -> display: 5");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(1); LED_B_Write(0);
            printMatrix();
        }
        if (keys[1][1] == 1 && last_state == 5)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 5 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 6 --- */
        if (keys[1][2] == 0 && last_state != 6)
        {
            last_state = 6;
            FourDigit74HC595_sendOneDigit(0, 6, 0);
            SW_Tx_UART_PutString("Button 6 pressed -> display: 6");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(0); LED_B_Write(0);
            printMatrix();
        }
        if (keys[1][2] == 1 && last_state == 6)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 6 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 7 --- */
        if (keys[2][0] == 0 && last_state != 7)
        {
            last_state = 7;
            FourDigit74HC595_sendOneDigit(0, 7, 0);
            SW_Tx_UART_PutString("Button 7 pressed -> display: 7");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
            printMatrix();
        }
        if (keys[2][0] == 1 && last_state == 7)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 7 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 8 --- */
        if (keys[2][1] == 0 && last_state != 8)
        {
            last_state = 8;
            FourDigit74HC595_sendOneDigit(0, 8, 0);
            SW_Tx_UART_PutString("Button 8 pressed -> display: 8");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
            printMatrix();
        }
        if (keys[2][1] == 1 && last_state == 8)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 8 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 9 --- */
        if (keys[2][2] == 0 && last_state != 9)
        {
            last_state = 9;
            FourDigit74HC595_sendOneDigit(0, 9, 0);
            SW_Tx_UART_PutString("Button 9 pressed -> display: 9");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
            printMatrix();
        }
        if (keys[2][2] == 1 && last_state == 9)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 9 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка 0 --- */
        if (keys[3][1] == 0 && last_state != 0)
        {
            last_state = 0;
            FourDigit74HC595_sendOneDigit(0, 0, 0);
            SW_Tx_UART_PutString("Button 0 pressed -> display: 0");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
            printMatrix();
        }
        if (keys[3][1] == 1 && last_state == 0)   /* виправлено: було keys[1][1] */
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button 0 released");
            SW_Tx_UART_PutCRLF();
        }

        /* =====================================================
         * ЗАВДАННЯ 2: Відображення літер A, b, C, d, E, F
         *
         * Кнопка * — циклічно перемикає літери:
         *             A → b → C → d → E → F → A → ...
         * Кнопка # — показує прочерк (-)
         * ===================================================== */

        /* --- Кнопка * (keys[3][0]) --- */
        if (keys[3][0] == 0 && last_state != 10)
        {
            last_state = 10;

            /* Отримуємо поточну літеру та просуваємо індекс */
            uint8_t cur_letter_idx = LETTER_CYCLE[letter_index];
            letter_index = (letter_index + 1) % LETTER_CYCLE_LEN;

            FourDigit74HC595_sendOneDigit(0, cur_letter_idx, 0);

            /* Вивід у UART: яка літера показана */
            SW_Tx_UART_PutString("Button * pressed -> display: ");
            switch (cur_letter_idx)
            {
                case IDX_A: SW_Tx_UART_PutString("A"); break;
                case IDX_b: SW_Tx_UART_PutString("b"); break;
                case IDX_C: SW_Tx_UART_PutString("C"); break;
                case IDX_d: SW_Tx_UART_PutString("d"); break;
                case IDX_E: SW_Tx_UART_PutString("E"); break;
                case IDX_F: SW_Tx_UART_PutString("F"); break;
                default:    SW_Tx_UART_PutString("?"); break;
            }
            SW_Tx_UART_PutCRLF();

            LED_R_Write(0); LED_G_Write(1); LED_B_Write(0);
            printMatrix();
        }
        if (keys[3][0] == 1 && last_state == 10)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button * released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* --- Кнопка # (keys[3][2]) — показує прочерк (-) --- */
        if (keys[3][2] == 0 && last_state != 11)
        {
            last_state = 11;
            FourDigit74HC595_sendOneDigit(0, IDX_DASH, 0);
            SW_Tx_UART_PutString("Button # pressed -> display: -");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(0); LED_B_Write(0);
            printMatrix();
        }
        if (keys[3][2] == 1 && last_state == 11)
        {
            last_state = 12;
            SW_Tx_UART_PutString("Button # released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

    } /* for(;;) */
}

/* [] END OF FILE */
