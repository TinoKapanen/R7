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

#define DELAY_MOTOR 1
#define LOW_SPEED 200
#define LOW_SPEED_MORE 5
#define HIGH_SPEED 250
#define HIGH_SPEED_MORE 255
#define MAX_SPEED 255

float checkBattery(){
    int16 adcresult = 0;
    float volts = 0.0;
    float battery = 0.0;
    ADC_Battery_StartConvert();
    if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for get ADC converted value
    adcresult = ADC_Battery_GetResult16();
    volts = ADC_Battery_CountsTo_Volts(adcresult);                  // convert value to Volts
    battery = volts*1.5;
    }
    return battery;
}

int rread(void);

int main()
{
    CyGlobalIntEnable;
    UART_1_Start();
    ADC_Battery_Start();
    float battery = 0.0;
    battery = checkBattery();
    struct sensors_ dig;
    sensor_isr_StartEx(sensor_isr_handler);
    reflectance_start();
    IR_led_Write(1);
    BatteryLed_Write(0); 
    
    int check = 0; // check for stop sign
    int wasOnLine = 0; // check if robot was on line previously
    int flag = 0; // last known track position
    int stopValue = 0; // number of stop signs encountered
    int remote = 1; // remote value
    int start = 1; // start condition
    
    //initially wait
    CyDelay(1500);
    //go forward
    motor_start();

    //while battery health is good
    while(battery >= 4) {
        
        // get sensor reading
        reflectance_digital(&dig);

        // all stop sign == all black
        if(dig.l3 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r3 == 0)
                {
                    // robot in line - wait for remote - after activation go forward
                    if (start == 1){
                        motor_stop();
                        start = 0;
                        remote = get_IR();
                        if(remote != 1){
                            motor_start();
                        }
                        // first stop sign - initialize it to zero
                        wasOnLine = 0;
                    }
                    // increase stop condition value if robot encounters a NEW stop sign
                    if (wasOnLine == 1){
                        if (stopValue == 0 || stopValue == 1 || stopValue == 2){
                        stopValue++;
                        }
                        // second stop sign encountered - stop here
                        if (stopValue >= 2){
                        motor_stop();
                        }
                        // robot is in stop sign - set this to zero so stop condition will not increase
                        wasOnLine = 0;
                    }
                }
                
                // Turn logic for every kind of sensor reading - ie. 0111 robot goes left, 1001 robot goes straight
                // 0111 will turn left more than 0011
                
                // also keep the position of the track in memory - robot knows where to turn if it goes out of bounds (flag)
                // if robot is in line wasOnLine will be set to 1
                else if(dig.l3 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r3 == 1) // left
                {
                    flag = 1;
                    motor_turn(LOW_SPEED_MORE,HIGH_SPEED_MORE,DELAY_MOTOR);
                    wasOnLine = 1;
                }else if (dig.l3 == 1 && dig.l1 == 0 && dig.r1 == 1 && dig.r3 == 1) // left
                {
                    flag = 2;
                    wasOnLine = 1;
                    motor_turn(LOW_SPEED,HIGH_SPEED,DELAY_MOTOR);
                }else if(dig.l3 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r3 == 0) // right
                {
                    flag = 3;
                    wasOnLine = 1;
                    motor_turn(HIGH_SPEED_MORE,LOW_SPEED_MORE,DELAY_MOTOR);
                }else if (dig.l3 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r3 == 1) // right
                {
                    flag = 4;
                    motor_turn(HIGH_SPEED,LOW_SPEED,DELAY_MOTOR); 
                }else if(dig.l3 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r3 == 1) // straight
                {
                    wasOnLine = 1;
                    motor_forward(MAX_SPEED,DELAY_MOTOR);
                }
                else if(dig.l3 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r3 == 1) // robot out of track, get last known position from flag value
                {
                    switch (flag)
                    {
                        case 1:
                            motor_turn(LOW_SPEED_MORE,HIGH_SPEED_MORE,DELAY_MOTOR); // turn left
                            break;
                        case 2:
                            motor_turn(LOW_SPEED,HIGH_SPEED,DELAY_MOTOR); // turn left
                            break;
                        case 3:
                            motor_turn(HIGH_SPEED_MORE,LOW_SPEED_MORE,DELAY_MOTOR); // turn right
                            break;
                        case 4:
                            motor_turn(HIGH_SPEED,LOW_SPEED,DELAY_MOTOR); // turn right
                            break;
                    }

        }
        // check battery health after every 5000 cycles
        check++;
        if (check == 5000){
        battery = checkBattery();
        check = 0;
        }
        CyDelay(1);
    }
    // battery health low - stop and flash LED
    while (battery < 4){
        BatteryLed_Write(1);
        CyDelay(500);
        BatteryLed_Write(0);
        CyDelay(500);
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
