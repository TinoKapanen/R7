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

#define MOTOR_DELAY 20
#define MOTOR_DELAY_CY 1
#define MAX_SPEED 255
#define MIN_SPEED -150
int rread(void);

int GetBlack(int l3, int l1, int r1, int r3)
{
    if(l3 != 1 || l1 != 1 || r1  !=1 ||  r3 != 1)
    {
        return 1;
    }else
    {
        return 0;
    }
}
/**
 * @file    main.c
 * @brief   
 * @details  ** You should enable global interrupt for operating properly. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/


//battery level//
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    ADC_Battery_Start();        
 
    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 

    struct sensors_ dig;
    CyGlobalIntEnable; 
    UART_1_Start();
  
    Ultra_Start();                          // Ultra Sonic Start function
    sensor_isr_StartEx(sensor_isr_handler);
    
    reflectance_start();

    IR_led_Write(1);
    
    int start = 0;
    
    //wait before activation
    CyDelay(1500);
    
    // initially go forward
    motor_start();
    motor_forward(150,100);
    
    while(1)
    {
        reflectance_digital(&dig);
        // start sequence - foward till come in contact with the black line - wait for remote activation
        if ( start == 0 )
        {
            //go to first black line
            if(dig.l3 == 0 && dig.l1 == 0 && dig.r1 == 0 &&  dig.r3 == 0)
            {
                //wait
                motor_stop();
                //get remote - then move into the ring
                if (get_IR() != -1)
                {
                    motor_start();
                    motor_forward(200,100);
                    start = 1;
                }
            }   
        // robot is in the ring from this point forward
        }else
        {
            // if in contanct of black line = turn
        if(dig.l3 != 1 || dig.l1 != 1 || dig.r1  !=1 ||  dig.r3 != 1)
        {
            motor_turn1(MAX_SPEED,MAX_SPEED,MOTOR_DELAY_CY);
        }else
        {
            // go forward until sonic sensor detecs another object
            motor_forward(MAX_SPEED, MOTOR_DELAY_CY);
            
            // some object detected
            if(Ultra_GetDistance() < 50)
            {
                // thrust toward the object to remove it from the ring - until we come contact with a black line
                do{
                    motor_forward(255, MOTOR_DELAY_CY);
                    
                }while(dig.l3 != 1 || dig.l1 != 1 || dig.r1  !=1 ||  dig.r3 != 1);
                
            }else{
                // turn away from the black line
                motor_turn1(MAX_SPEED,MAX_SPEED,MOTOR_DELAY_CY);
            }
                //no object detected - go forward
             motor_forward(MAX_SPEED, MOTOR_DELAY_CY);
        }
        }
        CyDelay(1);
    }
    return 0;
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
 