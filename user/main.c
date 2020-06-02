/**
  ******************************************************************************
  * @file    Project/main.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    16-June-2017
  * @brief   Main program body
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 


/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stdio.h"
#include "string.h"
/* Private defines -----------------------------------------------------------*/
#define LED_GPIO_PORT  (GPIOB)
#define LED_GPIO_PINS  (GPIO_PIN_5)

#define PWR_LIGHT_GPIO_PORT  (GPIOA)
#define PWR_LIGHT_GPIO_PINS  (GPIO_PIN_3)

#define PWR_FAN_GPIO_PORT  (GPIOA)
#define PWR_FAN_GPIO_PINS  (GPIO_PIN_2)

#define LIGHT_ON GPIO_WriteHigh(PWR_LIGHT_GPIO_PORT, PWR_LIGHT_GPIO_PINS)
#define LIGHT_OFF GPIO_WriteLow(PWR_LIGHT_GPIO_PORT, PWR_LIGHT_GPIO_PINS)
#define FAN_ON GPIO_WriteHigh(PWR_FAN_GPIO_PORT, PWR_FAN_GPIO_PINS)
#define FAN_OFF GPIO_WriteLow(PWR_FAN_GPIO_PORT, PWR_FAN_GPIO_PINS)

#define CCR1_Val ((uint16_t)15625)
//#define CCR2_Val ((uint16_t)488)
//#define CCR3_Val  ((uint16_t)244)
/* Private function prototypes -----------------------------------------------*/
void Delay (uint16_t nCount);
static void TIM2_Config(void);

void uart1_config(void);
void uart1_send_str(char *str, uint16_t len);

/* Private functions ---------------------------------------------------------*/

uint16_t cnt = 0;
uint8_t min = 0;
uint8_t hour = 0;
uint8_t cnt_updated = 0;
//char buf[20]={0};

void main(void)
{
  
//    uint8_t buflen = 0;
//    uint8_t i = 0;
    
    
    /*High speed internal clock prescaler: 1*/
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    /* Initialize I/Os in Output Mode */
    GPIO_Init(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(PWR_LIGHT_GPIO_PORT, (GPIO_Pin_TypeDef)PWR_LIGHT_GPIO_PINS, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(PWR_FAN_GPIO_PORT, (GPIO_Pin_TypeDef)PWR_FAN_GPIO_PINS, GPIO_MODE_OUT_PP_LOW_FAST);

//    uart1_config();

    TIM2_Config();
    
    enableInterrupts();

    while (1)
    {
        Delay(0xFFF);
        
        //1second update
        if(cnt_updated) {
            cnt_updated = 0;
            //minutes
            if(cnt==60) {
                cnt = 0;
                min++;
            }
            //hours
            if(min==60) {
                min = 0;
                hour++;
            }
            //1 day loop
            if(hour==24) {
                hour = 0;
            }

            //we turn on fan when in first 6hours in a day,
            //assume we start system at noon
            if(hour<6) {
                LIGHT_OFF;
                FAN_ON;
            }else if(hour>=18 && hour<24) {
                LIGHT_ON;
                FAN_OFF;
            }else {
                LIGHT_OFF;
                FAN_OFF;                
            }
            
          
            //debug===============================
            //led indicate
            if(cnt%5==0) {
                GPIO_WriteLow(LED_GPIO_PORT, LED_GPIO_PINS);
            }else {
                GPIO_WriteHigh(LED_GPIO_PORT, LED_GPIO_PINS);
            }
          
//            sprintf(buf, "%dm%ds\n", min,cnt);
//            buflen = strlen(buf);
//            uart1_send_str(buf, buflen);
//            for(i=0;i<buflen;i++) {
//                buf[i] = 0;
//            }
        }//end if(cnt_updated) 
    }//end while(1)
  
}


/**
  * @brief Delay
  * @param nCount
  * @retval None
  */
void Delay(uint16_t nCount)
{
    /* Decrement nCount value */
    while (nCount != 0)
    {
    nCount--;
    }
}


void uart1_config()
{
    UART1_DeInit();
    /* UART1 configuration ------------------------------------------------------*/
    /* UART1 configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Receive and transmit enabled
        - UART1 Clock disabled
    */
    UART1_Init((uint32_t)115200, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
  
}


void uart1_send_str(char *str, uint16_t len)
{
    uint16_t i;
    for(i=0;i<len;i++) {
        UART1_SendData8(str[i]);
        while(!UART1_GetFlagStatus(UART1_FLAG_TC));
    }
}

/**
  * @brief  Configure Output Compare Active Mode for TIM2 Channel1, Channel2 and 
  *         channel3  
  * @param  None
  * @retval None
  */
static void TIM2_Config(void)
{
  /* Time base configuration */
  /*system clk defualt 2M/8=250kHz  250k/16 = 15625*/
  TIM2_TimeBaseInit(TIM2_PRESCALER_256, 61920);

  /* Prescaler configuration */
  TIM2_PrescalerConfig(TIM2_PRESCALER_256, TIM2_PSCRELOADMODE_IMMEDIATE);

  /* Output Compare Active Mode configuration: Channel1 */
  /*
	  TIM2_OCMode = TIM2_OCMODE_INACTIVE
       TIM2_OCPolarity = TIM2_OCPOLARITY_HIGH
       TIM2_Pulse = CCR1_Val=15625
	*/
  TIM2_OC1Init(TIM2_OCMODE_INACTIVE, TIM2_OUTPUTSTATE_ENABLE,CCR1_Val, TIM2_OCPOLARITY_HIGH); 
  TIM2_OC1PreloadConfig(DISABLE);

//  /* Output Compare Active Mode configuration: Channel2 */
//  
//  /*TIM2_Pulse = CCR2_Val;  */
//  TIM2_OC2Init(TIM2_OCMODE_INACTIVE, TIM2_OUTPUTSTATE_ENABLE,CCR2_Val, TIM2_OCPOLARITY_HIGH); 
//  TIM2_OC2PreloadConfig(DISABLE);
//
//  /* Output Compare Active Mode configuration: Channel3 */
//  /*TIM2_Pulse = CCR3_Val  */
//  TIM2_OC3Init(TIM2_OCMODE_INACTIVE, TIM2_OUTPUTSTATE_ENABLE,CCR3_Val, TIM2_OCPOLARITY_HIGH); 
//  TIM2_OC3PreloadConfig(DISABLE);

  TIM2_ARRPreloadConfig(ENABLE);
  
  /* TIM IT enable */
  TIM2_ITConfig(TIM2_IT_CC1, ENABLE);
//  TIM2_ITConfig(TIM2_IT_CC2, ENABLE);
//  TIM2_ITConfig(TIM2_IT_CC3, ENABLE);
  
  /* TIM2 enable counter */
  TIM2_Cmd(ENABLE);
}


#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
