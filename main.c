/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */  
    
    
/*
(Prescaler/72MHz)  * 65535 = 8.1 cek - максимум между импульсами

8.1 сек = 65535              
х сек   = 4045 (намеряли)


     2248(длина окружности колеса) * 65535      36
V = -------------------------------------- * ---------- [km/h] 
      8.1 сек * 4045(намеряли)                  10000

        65535 * 36
Kv = ----------------
       8.1 c * 10000


            65535
Cadence = -------------------  * 60 сек [rpm]
           Capture2 * 8.1 cek

          65535 * 60cek
Kc = -------------------
           8.1cek

*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "display.h"
#include "bmp280_user.h"
#include <math.h>
#include <stdbool.h>
#include "const_var.h"
#include "sEEPROM_SPI.h"


/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

GPIO_InitTypeDef GPIO_InitStructure;



    
/* Private function prototypes -----------------------------------------------*/
__IO void delay(__IO uint32_t nCount);
void GPIO_Setup(void);
void Tim3_Setup(void);
void Tim4_setup(void);



int main(void)
{
  uint32_t resetcntr = 0;  
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32f10x_xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32f10x.c file
  */     
  
  /* Initialize LEDs, Key Button, LCD and COM port(USART) available on
  STM3210X-EVAL board ******************************************************/
  
  
  /* Retarget the C library printf function to the USARTx, can be USART1 or USART2
  depending on the EVAL board you are using ********************************/
  
  /* Add your application code here
  */
  
  
  GPIO_Setup(); //LED PC.13
  InitDisplay(); //I2C1 init
  sEE_Init();
  BMP280_I2C_Setup(&bmp);
 
  
  if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15))
  {
    SSD1306_GotoXY(0, 5);
    SSD1306_Puts("Reset all", &lessPerfectDOSVGA_13ptFontInfo, SSD1306_COLOR_WHITE);
    SSD1306_UpdateScreenDMA();
    while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15))
    {     
      if(resetcntr++ >= 10000000)
      {
        Impulse_wheel_total = 0;
        Impulse_wheel = 0;
        Impulse_crank = 0;
        Capture1_total = 0;
        Capture2_total = 0;
        Velocity_max = 0;
        Cadence_max = 0;
        Velocity_avg = 0;
        Cadence_avg = 0; 
        WriteDataEEPROM();
        
        resetcntr = 10000000;
        SSD1306_GotoXY(0, 5);
        SSD1306_Puts("Reseted!", &lessPerfectDOSVGA_13ptFontInfo, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreenDMA();
        delay(20000000);
        break;
      }
    }
  }
  
    sEE_ReadBuffer((uint8_t*)&Impulse_wheel_total, 0x20, sizeof(Impulse_wheel_total));
  sEE_ReadBuffer((uint8_t*)&pres64_, 0x40, sizeof(pres64_));
  sEE_ReadBuffer((uint8_t*)&Impulse_wheel, 0x44, sizeof(Impulse_wheel));
  sEE_ReadBuffer((uint8_t*)&Capture1, 0x48, sizeof(Capture1));
  sEE_ReadBuffer((uint8_t*)&Capture2, 0x4A, sizeof(Capture2));
  sEE_ReadBuffer((uint8_t*)&Capture1_total, 0x4E, sizeof(Capture1_total));
  sEE_ReadBuffer((uint8_t*)&Capture2_total, 0x52, sizeof(Capture2_total));
  sEE_ReadBuffer((uint8_t*)&Velocity_avg, 0x60, sizeof(Velocity_avg));
  sEE_ReadBuffer((uint8_t*)&Velocity_max, 0x64, sizeof(Velocity_max));
  sEE_ReadBuffer((uint8_t*)&Cadence_avg, 0x68, sizeof(Cadence_avg));
  sEE_ReadBuffer((uint8_t*)&Cadence_max, 0x6C, sizeof(Cadence_max));
  
  
  // ENABLE Wake Up Pin PA.0
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
  PWR_WakeUpPinCmd(ENABLE);
   
  Tim3_Setup();//Input Capture  
  Tim4_setup();//1sec user program
  
  prog_state = 0x00;
  //sEE_WriteBuffer((uint8_t*)&Impulse_wheel, 0x20, 4);
  //
  /* Infinite loop */
  while (1)
  {
    
  }
}


void GPIO_Setup(void)
{
  /* GPIOC Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  /* Configure PC13 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  // Button
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

__IO void delay(__IO uint32_t nCount)
{
 while(nCount--)
  {
  }
   
}

void WriteDataEEPROM(void)
{
  sEE_WriteBuffer((uint8_t*)&Impulse_wheel_total, 0x20, sizeof(Impulse_wheel_total));
  sEE_WriteBuffer((uint8_t*)&pres64_, 0x40, sizeof(pres64_));
  sEE_WriteBuffer((uint8_t*)&Impulse_wheel, 0x44, sizeof(Impulse_wheel));
  sEE_WriteBuffer((uint8_t*)&Capture1, 0x48, sizeof(Capture1));
  sEE_WriteBuffer((uint8_t*)&Capture2, 0x4A, sizeof(Capture2));
  sEE_WriteBuffer((uint8_t*)&Capture1_total, 0x4E, sizeof(Capture1_total));
  sEE_WriteBuffer((uint8_t*)&Capture2_total, 0x52, sizeof(Capture2_total));
  sEE_WriteBuffer((uint8_t*)&Velocity_avg, 0x60, sizeof(Velocity_avg));
  sEE_WriteBuffer((uint8_t*)&Velocity_max, 0x64, sizeof(Velocity_max));
  sEE_WriteBuffer((uint8_t*)&Cadence_avg, 0x68, sizeof(Cadence_avg));
  sEE_WriteBuffer((uint8_t*)&Cadence_max, 0x6C, sizeof(Cadence_max));
}


void Tim3_Setup(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;//Tim3_Ch1, Tim2_Ch4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_SetPriority(TIM3_IRQn, 0x0E);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 8899;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);
  /* TIM enable counter */
  TIM_Cmd(TIM3, ENABLE);
  
  /* Enable the CC2 Interrupt Request */
  TIM_ITConfig(TIM3, TIM_IT_CC3 | TIM_IT_CC4, ENABLE);
}


void Tim4_setup(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_SetPriority(TIM4_IRQn, 0x0F);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  TIM_TimeBaseStructure.TIM_Period = 9999;
  TIM_TimeBaseStructure.TIM_Prescaler = 3599;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM4, ENABLE);
  
  /* Enable the CC2 Interrupt Request */
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
    
  }
}


#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
