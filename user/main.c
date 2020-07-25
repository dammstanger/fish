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
#define LED_ON (GPIO_WriteLow(LED_GPIO_PORT, LED_GPIO_PINS))
#define LED_OFF (GPIO_WriteHigh(LED_GPIO_PORT, LED_GPIO_PINS))

#define PWR_LIGHT_GPIO_PORT  (GPIOA)
#define PWR_LIGHT_GPIO_PINS  (GPIO_PIN_3)

#define PWR_FAN_GPIO_PORT  (GPIOA)
#define PWR_FAN_GPIO_PINS  (GPIO_PIN_2)

#define SERVO_GPIO_PORT  (GPIOC)
#define SERVO_GPIO_PINS  (GPIO_PIN_7)

#define S1_PUSH GPIO_ReadInputPin(GPIOD, (GPIO_Pin_TypeDef)GPIO_PIN_4)

#define LIGHT_ON GPIO_WriteHigh(PWR_LIGHT_GPIO_PORT, PWR_LIGHT_GPIO_PINS)
#define LIGHT_OFF GPIO_WriteLow(PWR_LIGHT_GPIO_PORT, PWR_LIGHT_GPIO_PINS)
#define FAN_ON GPIO_WriteHigh(PWR_FAN_GPIO_PORT, PWR_FAN_GPIO_PINS)
#define FAN_OFF GPIO_WriteLow(PWR_FAN_GPIO_PORT, PWR_FAN_GPIO_PINS)

#define CCR1_Val ((uint16_t)15625)
//#define CCR2_Val ((uint16_t)488)
//#define CCR3_Val  ((uint16_t)244)
/* Private function prototypes -----------------------------------------------*/
void Delay (uint32_t nCount);
static void TIM2_Config(void);
static void TIM1_Config(void);

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
    uint8_t i = 0;
    uint8_t buttom_sta_last = 0;
    uint8_t lock_unlock_cnt = 0;
    uint8_t set_mode = 0;
    uint8_t start_hour = 7;             //from0-23
    uint8_t display_loop = 0;
    
    /*High speed internal clock prescaler: 1*/
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    /* Initialize I/Os in Output Mode */
    GPIO_Init(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(PWR_LIGHT_GPIO_PORT, (GPIO_Pin_TypeDef)PWR_LIGHT_GPIO_PINS, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(PWR_FAN_GPIO_PORT, (GPIO_Pin_TypeDef)PWR_FAN_GPIO_PINS, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)GPIO_PIN_4, GPIO_MODE_IN_FL_NO_IT);
    LED_OFF;
//    uart1_config();

//    TIM1_Config();
    TIM2_Config();
    
    enableInterrupts();

    while (1)
    {
//      if(!set_mode){
//        Delay(0xFFF);
//      }        
        //1second update
        if(cnt_updated) {
            cnt_updated = 0;
            //minutes
            if(cnt>=60) {
                cnt = 0;
                min++;
            }
            //hours
            if(min>=60) {
                min = 0;
                hour++;
            }
            //1 day loop
            if(hour>=24) {
                hour = 0;
            }

            //we turn on light when in first 5hours in a day,
            //assume we start system at 7:00 am
            if(hour>=7 && hour<11) {
                LIGHT_ON;
                FAN_OFF;
            }else if(hour>=12 && hour<21) {
                LIGHT_OFF;
                FAN_ON;
            }else {
                LIGHT_OFF;
                FAN_OFF;                
            }
            
//            if(S1_PUSH) {
//               LED_ON;
//            }else{
//              LED_OFF;
//            }
//            //button==============================
            if(S1_PUSH && !set_mode) {
              lock_unlock_cnt++;
              LED_ON;
              if(lock_unlock_cnt>3) {
                lock_unlock_cnt = 0;
                set_mode = 1;
                LED_OFF;
                Delay(0xFFFF);
                LED_ON;
                Delay(0xFFFF);
                LED_OFF;
                Delay(0xFFFF);
                LED_ON;
                Delay(0xFFFF);
                LED_OFF;
                //block to prevent set hour here
                buttom_sta_last = 0;
                while(S1_PUSH);
              }
            }else if(S1_PUSH && set_mode){
              lock_unlock_cnt++;
              display_loop = 0;
              if(lock_unlock_cnt>3) {
                lock_unlock_cnt = 0;
                set_mode = 0;
                LED_OFF;
                Delay(0xFFFF);
                LED_ON;
                Delay(0xFFFF);
                LED_OFF;
                Delay(0xFFFF);
                LED_ON;
                Delay(0xFFFF);
                LED_OFF;
                
                //set
                hour = start_hour;
                min = 0;
                cnt = 0;
                
                buttom_sta_last = 0;
                while(S1_PUSH);
              }
            }else {
              lock_unlock_cnt = 0;
            }
            
            if(set_mode) {
              display_loop++;
              display_loop &= 3;
            }
            
              
              
            //debug===============================
            //led indicate
            if(!set_mode && !S1_PUSH) {
              if(cnt%5==0) {
                  LED_ON;
              }else {
                  LED_OFF;
              }
            }
            
//            sprintf(buf, "%dm%ds\n", min,cnt);
//           buflen = strlen(buf);
//            uart1_send_str(buf, buflen);
//            for(i=0;i<buflen;i++) {
//                buf[i] = 0;
//            }
        }//end if(cnt_updated) 

        
        if(set_mode) {
          if(S1_PUSH && !buttom_sta_last) {
            buttom_sta_last = 1;
          }else if(!S1_PUSH && buttom_sta_last) {
            buttom_sta_last = 0;
            
            start_hour++;
            if(start_hour==24) {
              start_hour = 0;
            }
          }else if(display_loop==3){
              
            for(i=start_hour/5; i>0; i--) {
              LED_ON;
              Delay(0x5FFFF);
              LED_OFF;
              Delay(0x1FFFF);
            }
            
            for(i=start_hour%5; i>0; i--) {
              LED_ON;
              Delay(0x15FFF);
              LED_OFF;
              Delay(0x15FFF);
            }
          }
        }
    }//end while(1)
  
}


/**
  * @brief Delay
  * @param nCount
  * @retval None
  */
void Delay(uint32_t nCount)
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

/**
  * @brief  Configure TIM1 to generate 7 PWM signals with 4 different duty cycles
  * @param  None
  * @retval None
  */
static void TIM1_Config(void)
{

   TIM1_DeInit();

  /* Time Base configuration */
  /*
  TIM1_Period = 4095
  TIM1_Prescaler = 0
  TIM1_CounterMode = TIM1_COUNTERMODE_UP
  TIM1_RepetitionCounter = 0
  */
   
  //预分频=0时，默认 16M/8 = 2M,
  //预分频=n时16M/(n+1)    
  //16M/(15+1) = 1MHz, 1M/(19999+1-0) = 50Hz
  TIM1_TimeBaseInit(15, TIM1_COUNTERMODE_UP, 19999, 0);                

  /* Channel 1, 2,3 and 4 Configuration in PWM mode */
  
  /*
  TIM1_OCMode = TIM1_OCMODE_PWM2
  TIM1_OutputState = TIM1_OUTPUTSTATE_ENABLE
  TIM1_OutputNState = TIM1_OUTPUTNSTATE_ENABLE
  TIM1_Pulse = CCR1_Val
  TIM1_OCPolarity = TIM1_OCPOLARITY_LOW
  TIM1_OCNPolarity = TIM1_OCNPOLARITY_HIGH
  TIM1_OCIdleState = TIM1_OCIDLESTATE_SET
  TIM1_OCNIdleState = TIM1_OCIDLESTATE_RESET
  */

  /*TIM1_Pulse = CCR2_Val*/
  TIM1_OC2Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_ENABLE, 1000,
               TIM1_OCPOLARITY_LOW, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_SET, 
               TIM1_OCNIDLESTATE_RESET);

  /*enable interrupt*/
  TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
  
  /* TIM1 counter enable */
  TIM1_Cmd(ENABLE);

  /* TIM1 Main Output Enable */
  TIM1_CtrlPWMOutputs(ENABLE);
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
