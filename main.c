/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This
    library has basic methods of various sensors and communications so that you can make what you want with them. <br>
    <br><br>
    </p>

    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>

    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "I2C_made.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "IR.h"
#include "Ambient.h"
#include "Beep.h"

#define DELAY_MOTOR 5
#define LOW_SPEED 10
#define HIGH_SPEED 160
#define HIGH_SPEED_MORE 255

extern struct sensors_ threshold;

int rread(void);
int whiteL2;
int whiteL1;
int whiteR1;
int whiteR2;
int blackL2;
int blackL1;
int blackR1;
int blackR2;
// Calibrate sensors on white part and black part then calculate the average
int calculateAvg(int x, int y){
    int z = x + (y-x) / 2;
    return z;
}
void calibrate() {
    reflectance_set_threshold(
            calculateAvg(whiteL2, blackL2),
            calculateAvg(whiteL1, blackL1),
            calculateAvg(whiteR1, blackR1),
            calculateAvg(whiteR2, blackR2)
    );
}

/**
 * @file    main.c
 * @brief
 * @details  ** You should enable global interrupt for operating properly. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/


//battery level//
/*void CheckWay(int l3, int l1, int r1, int r3)
{

}*/

int main()
{
    CyGlobalIntEnable;
    UART_1_Start();
    ADC_Battery_Start();
    //int16 adcresult =0;
    //float volts = 0.0;

    struct sensors_ ref;
    struct sensors_ dig;

    sensor_isr_StartEx(sensor_isr_handler);
    reflectance_start();
    IR_led_Write(1);
    BatteryLed_Write(1); // Switch led on
    //BatteryLed_Write(0); // Switch led off
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    int wasOnLine = 0;
    int flag = 0;
    int stopValue = 0;
    //int button; // SW1
    int remote = 1; // get_IR()
    //func = 1; // current func of SW1
    //ready = 0; // calibrated or not
    //release = 1;
    int start = 1;
    CyDelay(1500);
    motor_start();

    while(1) {

        reflectance_read(&ref);
        reflectance_digital(&dig);

        // Press SW1 button for calibration -> Sets new values of threshold
        //button = SW1_Read();
        //CALIBRATION : not used
        /* while (button == 0 && release == 1){
                release = 0;
            if (func == 2){
            blackL2 = ref.l3;
            blackL1 = ref.l1;
            blackR1 = ref.r1;
            blackR2 = ref.r3;
                calibrate();
                func = 1;
                button = 1;
                ready = 1;
            }
            if (func == 1){
            whiteL2 = ref.l3;
            whiteL1 = ref.l1;
            whiteR1 = ref.r1;
            whiteR2 = ref.r3;
                func = 2;
                button = 1;

            }

             CyDelay(500);
             release = 1;
        }
        */
        if(dig.l3 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r3 == 0)
        {
            if (start == 1){
                motor_stop();
                start = 0;
                remote = get_IR();
                if(remote != 1){
                    motor_start();
                }
                wasOnLine = 0;
            }
            if (wasOnLine == 1){
                if (stopValue == 0 || stopValue == 1 || stopValue == 2){
                    stopValue++;
                }
                if (stopValue >= 2){
                    motor_stop();
                }
                wasOnLine = 0;
            }
        }
        else if(dig.l3 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r3 == 1)
        {
            flag = 1;
            motor_turn(LOW_SPEED,HIGH_SPEED_MORE,DELAY_MOTOR); // turn left more
            wasOnLine = 1;
            //     printf("Turn left more \n");
        }else if (dig.l3 == 1 && dig.l1 == 0 && dig.r1 == 1 && dig.r3 == 1)
        {
            flag = 2;
            wasOnLine = 1;
            motor_turn(LOW_SPEED,HIGH_SPEED,DELAY_MOTOR); // turn left
            //  printf("Turn left \n");
        }else if(dig.l3 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r3 == 0)
        {
            flag = 3;
            wasOnLine = 1;
            motor_turn(HIGH_SPEED_MORE,LOW_SPEED,DELAY_MOTOR); // turn right more
            //    printf("Turn right more \n");
        }else if (dig.l3 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r3 == 1)
        {
            flag = 4;
            motor_turn(HIGH_SPEED,LOW_SPEED,DELAY_MOTOR); // turn right
            //    printf("Turn right \n");
        }else if(dig.l3 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r3 == 1)
        {
            wasOnLine = 1;
            //motor_turn(HIGH_SPEED_MORE,HIGH_SPEED_MORE,DELAY_MOTOR); //go straight on
            motor_forward(HIGH_SPEED_MORE,DELAY_MOTOR);
            //  printf("Go straight on");
        }
        else if(dig.l3 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r3 == 1)
        {
            switch (flag)
            {
                case 1:
                    motor_turn(LOW_SPEED,HIGH_SPEED_MORE,DELAY_MOTOR); // turn left more
                    // printf("flag turn left more %d", flag);
                    break;
                case 2:
                    motor_turn(LOW_SPEED,HIGH_SPEED,DELAY_MOTOR); // turn left
                    //  printf("flag turn left %d", flag);
                    break;
                case 3:
                    motor_turn(HIGH_SPEED_MORE,LOW_SPEED,DELAY_MOTOR); // turn right more
                    //    printf("flag turn right more %d", flag);
                    break;
                case 4:
                    motor_turn(HIGH_SPEED,LOW_SPEED,DELAY_MOTOR); // turn right
                    //    printf("flag turn right %d", flag);
                    break;
            }

        }
    }
    CyDelay(1);
}
#if 0
int rread(void)
{
    SC0_SetDriveMode(PIN_DM_STRONG);
    SC0_Write(1);
    CyDelayUs(10);
    SC0_SetDriveMode(PIN_DM_DIG_HIZ);
    Timer_1_Start();
    uint16_t start = Timer_1_ReadCounter();
    uint16_t end = 0;
    while(!(Timer_1_ReadStatusRegister() & Timer_1_STATUS_TC)) {
        if(SC0_Read() == 0 && end == 0) {
            end = Timer_1_ReadCounter();
        }
    }
    Timer_1_Stop();

    return (start - end);
}
#endif

/* Don't remove the functions below */
int _write(int file, char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
    int n;
    for(n = 0; n < len; n++) {
        if(*ptr == '\n') UART_1_PutChar('\r');
        UART_1_PutChar(*ptr++);
    }
    return len;
}

int _read (int file, char *ptr, int count)
{
    int chs = 0;
    char ch;

    (void)file; /* Parameter is not used, suppress unused argument warning */
    while(count > 0) {
        ch = UART_1_GetChar();
        if(ch != 0) {
            UART_1_PutChar(ch);
            chs++;
            if(ch == '\r') {
                ch = '\n';
                UART_1_PutChar(ch);
            }
            *ptr++ = ch;
            count--;
            if(ch == '\n') break;
        }
    }
    return chs;
}
/* [] END OF FILE */
