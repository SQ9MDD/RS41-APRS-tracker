// STM32F100 and SI4032 RTTY transmitter
// released under GPL v.2 by anonymous developer
// enjoy and have a nice day
// ver 1.5a
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_rcc.h>
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <misc.h>
#include "init.h"
#include "config.h"
#include "radio.h"
#include "ublox.h"
#include "delay.h"
#include "aprs.h"
///////////////////////////// test mode /////////////
static __IO uint32_t DelayCounter;
const unsigned char test = 0; // 0 - normal, 1 - short frame only cunter, height, flag
//char callsign[15] = {CALLSIGN};


#define GREEN  GPIO_Pin_7
#define RED  GPIO_Pin_8

unsigned int send_cun;        //frame counter
char status[2] = {'N'};
int voltage;

volatile char flaga = 0;
uint16_t CRC_rtty = 0x12ab;  //checksum
char buf_rtty[200];

volatile unsigned char pun = 0;
volatile unsigned int cun = 10;
volatile unsigned char tx_on = 0;
volatile unsigned int tx_on_delay;
volatile unsigned char tx_enable = 0;
volatile unsigned int stat_sended = 0;
//rttyStates send_rtty_status = rttyZero;
//volatile char *rtty_buf;
volatile uint16_t button_pressed = 0;
volatile uint8_t disable_armed = 0;

void send_rtty_packet();
uint16_t gps_CRC16_checksum (char *string);
// int srednia (int dana);


/**
 * GPS data processing
 */
void USART1_IRQHandler(void) {
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
    ublox_handle_incoming_byte((uint8_t) USART_ReceiveData(USART1));
      }else if (USART_GetITStatus(USART1, USART_IT_ORE) != RESET) {
    USART_ReceiveData(USART1);
  } else {
    USART_ReceiveData(USART1);
  }
}

void TIM2_IRQHandler(void) {
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    if (aprs_is_active()){
      aprs_timer_handler();
    }
  }
}

int main(void) {
#ifdef DEBUG
  debug();
#endif
  RCC_Conf();
  NVIC_Conf();
  init_port();

  //init_timer(RTTY_SPEED);
  delay_init();
  ublox_init();

  GPIO_SetBits(GPIOB, RED);
  USART_SendData(USART3, 0xc);

  radio_soft_reset();
  // setting TX frequency
  radio_set_tx_frequency(APRS_FREQUENCY);

  // setting TX power
  radio_rw_register(0x6D, 00 | (TX_POWER & 0x0007), 1);

  radio_rw_register(0x71, 0x00, 1);
  radio_rw_register(0x12, 0x20, 1);
  radio_rw_register(0x13, 0x00, 1);
  radio_rw_register(0x12, 0x00, 1);
  radio_rw_register(0x0f, 0x80, 1);
  tx_on = 0;
  tx_enable = 1;

  aprs_init();
  while (1) {
    if (tx_on == 0 && tx_enable) {

        _delay_ms((APRS_INTERVAL*1000)-1000);

        GPIO_SetBits(GPIOB, GREEN);
        GPIO_ResetBits(GPIOB, RED);
        radio_enable_tx();

        GPSEntry gpsData;
        ublox_get_last_data(&gpsData);
        USART_Cmd(USART1, DISABLE);

        //tutaj zlecamy wysylka ramki
        if(gpsData.sats_raw < 4){
        	aprs_send_status();
        	stat_sended = 1;
        }else{
        	if(stat_sended == 1){
        		aprs_send_status_ok();
        		stat_sended = 0;
        	}
        	aprs_send_position(gpsData);
        }
        USART_Cmd(USART1, ENABLE);
        radio_disable_tx();
        GPIO_SetBits(GPIOB, RED);
        GPIO_ResetBits(GPIOB, GREEN);
    } else {
      NVIC_SystemLPConfig(NVIC_LP_SEVONPEND, DISABLE);
      __WFI();
    }
  }
}

#ifdef  DEBUG
void assert_failed(uint8_t* file, uint32_t line)
{
    while (1);
}
#endif
