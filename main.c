#include "project.h"
int main(void)
{
    CyGlobalIntEnable;

    for(;;)
    {
        if(Button_Read() == 0) // Button pressed -> Cyan (G+B on)
        {
            LED_R_Write(1);
            LED_G_Write(0);
            LED_B_Write(0);
            CyDelay(500);
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(1);
            CyDelay(500);
        }
        else // Button released -> Red (R on)
        {
            LED_R_Write(0);
            LED_G_Write(1);
            LED_B_Write(1);
            CyDelay(500);
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(1);
            CyDelay(500);
        }
    }
}