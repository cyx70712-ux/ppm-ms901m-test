#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "PPM.h"
#include "ms901.h"
#include "motor.h"
#include "servo.h"


#include <stdio.h>



int map_value(int value);

int main(void)
{
	
	
	
//    uint8_t page = 0;
//    uint16_t page_cnt = 0;

//    char buf[16];

//    atk_ms901m_attitude_data_t att;
//    atk_ms901m_quaternion_data_t quat;
//    atk_ms901m_gyro_data_t gyro;
//    atk_ms901m_accelerometer_data_t acc;
//    atk_ms901m_magnetometer_data_t mag;
//    atk_ms901m_barometer_data_t baro;
//    atk_ms901m_port_data_t port;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    OLED_Init();
    PPM_Init();
		atk_ms901m_uart_init(115200);
		Motor_TIM2_Init(20000 - 1, 72 - 1);
		Servo_TIM3_Init(20000 - 1, 72 - 1);
		
		Motor_calibration();
		
		
		while(1)
		{
		Motor_Control();
		Servo_Control();
		OLED_ShowNum(1,5,ppm_pwm_values[0],4);
    OLED_ShowNum(2,5,ppm_pwm_values[1],4);
    OLED_ShowNum(3,5,ppm_pwm_values[2],4);
    OLED_ShowNum(4,5,ppm_pwm_values[3],4);

    OLED_ShowNum(1,10,ppm_pwm_values[4],4);
    OLED_ShowNum(2,10,ppm_pwm_values[5],4);
    OLED_ShowNum(3,10,ppm_pwm_values[6],4);
    OLED_ShowNum(4,10,ppm_pwm_values[7],4);
        

    
		}
		
		
		 
//    OLED_Clear();
//    OLED_ShowString(1,1,"MS901 Boot...");
//    Delay_ms(1000);

//    atk_ms901m_uart_init(115200);

//    Delay_ms(500);

//    atk_ms901m_rx_fifo_flush();

//    OLED_Clear();
//    OLED_ShowString(1,1,"MS901 Ready");
//    Delay_ms(1000);

//    while(1)
//    {
//        atk_ms901m_get_attitude(&att,50);

//        atk_ms901m_get_quaternion(&quat,50);

//        atk_ms901m_get_gyro_accelerometer(
//            &gyro,
//            &acc,
//            50);

//        atk_ms901m_get_magnetometer(
//            &mag,
//            50);

//        atk_ms901m_get_barometer(
//            &baro,
//            50);

//        atk_ms901m_get_port(
//            &port,
//            50);

//        page_cnt++;

//        if(page_cnt >= 20)
//        {
//            page_cnt = 0;

//            page++;

//            if(page > 5)
//            {
//                page = 0;
//            }

//            OLED_Clear();
//        }

//        switch(page)
//        {
//            /* =====================
//               姿态角
//               ===================== */
//            case 0:

//                sprintf(buf,"R:%6.1f",att.roll);
//                OLED_ShowString(1,1,buf);

//                sprintf(buf,"P:%6.1f",att.pitch);
//                OLED_ShowString(2,1,buf);

//                sprintf(buf,"Y:%6.1f",att.yaw);
//                OLED_ShowString(3,1,buf);

//                OLED_ShowString(4,1,"ATTITUDE");

//            break;

//            /* =====================
//               陀螺仪
//               ===================== */
//            case 1:

//                sprintf(buf,"GX:%6.1f",gyro.x);
//                OLED_ShowString(1,1,buf);

//                sprintf(buf,"GY:%6.1f",gyro.y);
//                OLED_ShowString(2,1,buf);

//                sprintf(buf,"GZ:%6.1f",gyro.z);
//                OLED_ShowString(3,1,buf);

//                OLED_ShowString(4,1,"GYRO");

//            break;

//            /* =====================
//               加速度
//               ===================== */
//            case 2:

//                sprintf(buf,"AX:%5.2f",acc.x);
//                OLED_ShowString(1,1,buf);

//                sprintf(buf,"AY:%5.2f",acc.y);
//                OLED_ShowString(2,1,buf);

//                sprintf(buf,"AZ:%5.2f",acc.z);
//                OLED_ShowString(3,1,buf);

//                OLED_ShowString(4,1,"ACC");

//            break;

//            /* =====================
//               四元数
//               ===================== */
//            case 3:

//                sprintf(buf,"Q0:%4.2f",quat.q0);
//                OLED_ShowString(1,1,buf);

//                sprintf(buf,"Q1:%4.2f",quat.q1);
//                OLED_ShowString(2,1,buf);

//                sprintf(buf,"Q2:%4.2f",quat.q2);
//                OLED_ShowString(3,1,buf);

//                sprintf(buf,"Q3:%4.2f",quat.q3);
//                OLED_ShowString(4,1,buf);

//            break;

//            /* =====================
//               磁力计
//               ===================== */
//            case 4:

//                sprintf(buf,"MX:%d",mag.x);
//                OLED_ShowString(1,1,buf);

//                sprintf(buf,"MY:%d",mag.y);
//                OLED_ShowString(2,1,buf);

//                sprintf(buf,"MZ:%d",mag.z);
//                OLED_ShowString(3,1,buf);

//                sprintf(buf,"T:%2.1f",mag.temperature);
//                OLED_ShowString(4,1,buf);

//            break;

//            /* =====================
//               气压计
//               ===================== */
//            case 5:

//                sprintf(buf,"P:%ld",(long)baro.pressure);
//                OLED_ShowString(1,1,buf);

//                sprintf(buf,"H:%ld",(long)baro.altitude);
//                OLED_ShowString(2,1,buf);

//                sprintf(buf,"T:%2.1f",baro.temperature);
//                OLED_ShowString(3,1,buf);

//                OLED_ShowString(4,1,"BARO");

//            break;
//        }

        
       


    }


int map_value(int value)
{
    int original_min = 1000;
    int original_max = 2000;

    int target_min = 500;
    int target_max = 2500;

    return (value - original_min)
            * (target_max - target_min)
            / (original_max - original_min)
            + target_min;
}
