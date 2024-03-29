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

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "display.h"
#include "si7021.h"
#include "bmp180.h"

// addresses of registers
volatile uint32_t *DWT_CONTROL = (uint32_t *)0xE0001000;
volatile uint32_t *DWT_CYCCNT = (uint32_t *)0xE0001004; 
volatile uint32_t *DEMCR = (uint32_t *)0xE000EDFC; 

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

GPIO_InitTypeDef GPIO_InitStructure;
BMP180_measurements PressAndTemp;
Si7021_measurments RelativeHumidityAndTemperature;
/* Private function prototypes -----------------------------------------------*/
__IO void delay(__IO uint32_t nCount);
void GPIO_Setup(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

int main(void)
{

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
  
 for(volatile uint32_t delay=0; delay<1000000; delay++);   
  
  GPIO_Setup(); //LED PC.13
  InitDisplay(); //I2C1 init
  //SI7021 placed on I2C1, no need to setup
  BMP180_Setup();
  
  SSD1306_GotoXY(0, 0);
  SSD1306_Puts("H ", &palatinoLinotype_12ptFontInfo, SSD1306_COLOR_WHITE);
  SSD1306_GotoXY(0, 20);
  SSD1306_Puts("T ", &palatinoLinotype_12ptFontInfo, SSD1306_COLOR_WHITE);
  SSD1306_GotoXY(0, 39);
  SSD1306_Puts("P ", &palatinoLinotype_12ptFontInfo, SSD1306_COLOR_WHITE);
  
  /* Infinite loop */
  while (1)
  {
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    delay(700000);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    delay(500000);
    
    Si7021_Read_RH_Temp(&RelativeHumidityAndTemperature);
    BMP180_get_T_P(&PressAndTemp);
    float mmHG = PressAndTemp.P / 0.1333;
    
    //SSD1306_Fill(SSD1306_COLOR_BLACK);  // clear entire screen
    SSD1306_DrawFilledRectangle(15,0,127,63, SSD1306_COLOR_BLACK);// clean area to prevent screen artifacts due variable character width
    
    SSD1306_GotoXY(15, 0);
    SSD1306_printf(&palatinoLinotype_12ptFontInfo, "%d \%", RelativeHumidityAndTemperature.RH);
    SSD1306_GotoXY(15, 20);
    SSD1306_printf(&palatinoLinotype_12ptFontInfo, "%.1f degC",  RelativeHumidityAndTemperature.Temperature_f);
    SSD1306_GotoXY(15, 39);
    SSD1306_printf(&palatinoLinotype_12ptFontInfo, "%.1f mmHg",  mmHG);
    //    *DEMCR = *DEMCR | 0x01000000; // enable the use DWT
    //    *DWT_CYCCNT = 0; // Reset cycle counter  
    //    *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable cycle counter
    //     count = 0;
    //   SSD1306_UpdateScreen();
    SSD1306_UpdateScreenDMA();
    //   count = *DWT_CYCCNT;
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
}

__IO void delay(__IO uint32_t nCount)
{
 while(nCount--)
  {
  }
   
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
