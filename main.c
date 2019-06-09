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
//#include "f_rtty.h"
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
//    } else {
//      if (tx_on) {
//        send_rtty_status = send_rtty((char *) rtty_buf);
//        if (!disable_armed){
//          if (send_rtty_status == rttyEnd) {
//            GPIO_SetBits(GPIOB, RED);
//            if (*(++rtty_buf) == 0) {
//              tx_on = 0;
//              tx_on_delay = tx_delay / (1000/RTTY_SPEED);
//              tx_enable = 0;
//              radio_disable_tx();
//            }
//          } else if (send_rtty_status == rttyOne) {
//            radio_rw_register(0x73, 0x02, 1);
//            GPIO_SetBits(GPIOB, RED);
//          } else if (send_rtty_status == rttyZero) {
//            radio_rw_register(0x73, 0x00, 1);
//            GPIO_ResetBits(GPIOB, RED);
//          }
//        }
//      }
//      if (!tx_on && --tx_on_delay == 0) {
//        tx_enable = 1;
//        tx_on_delay--;
//      }
//      if (--cun == 0) {
//        if (pun) {
//          GPIO_ResetBits(GPIOB, GREEN);
//          pun = 0;
//        } else {
//          if (flaga & 0x80) {
//            GPIO_SetBits(GPIOB, GREEN);
//          }
//          pun = 1;
//        }
//        cun = 200;
//      }
//      if (ALLOW_DISABLE_BY_BUTTON){
//        if (ADCVal[1] > 1900){
//          button_pressed++;
//          if (button_pressed > (5 * RTTY_SPEED)){
//            disable_armed = 1;
//            GPIO_SetBits(GPIOB, RED);
//            GPIO_SetBits(GPIOB, GREEN);
//          }
//        } else {
//          if (disable_armed){
//            GPIO_SetBits(GPIOA, GPIO_Pin_12);
//          }
//          button_pressed = 0;
//        }
//      }
//    }

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
//  rtty_buf = buf_rtty;
  tx_on = 0;
  tx_enable = 1;

  aprs_init();
  //radio_enable_tx();

  //uint8_t rtty_before_aprs_left = RTTY_TO_APRS_RATIO;

  while (1) {
    if (tx_on == 0 && tx_enable) {

        _delay_ms((APRS_INTERVAL*1000)-1000);
        radio_enable_tx();

        GPSEntry gpsData;
        ublox_get_last_data(&gpsData);
        USART_Cmd(USART1, DISABLE);
        //_delay_ms(200);
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
 //     }

    } else {
      NVIC_SystemLPConfig(NVIC_LP_SEVONPEND, DISABLE);
      __WFI();
    }
  }
}

//void send_rtty_packet() {
//  start_bits = RTTY_PRE_START_BITS;
//  int8_t si4032_temperature = radio_read_temperature();
//
////  voltage = srednia(ADCVal[0] * 600 / 4096);
//  voltage = ADCVal[0] * 600 / 4096;
//  GPSEntry gpsData;
//  ublox_get_last_data(&gpsData);
//  if (gpsData.fix >= 3) {
//        flaga |= 0x80;
//      } else {
//        flaga &= ~0x80;
//      }
//  uint8_t lat_d = (uint8_t) abs(gpsData.lat_raw / 10000000);
//  uint32_t lat_fl = (uint32_t) abs(abs(gpsData.lat_raw) - lat_d * 10000000) / 100;
//  uint8_t lon_d = (uint8_t) abs(gpsData.lon_raw / 10000000);
//  uint32_t lon_fl = (uint32_t) abs(abs(gpsData.lon_raw) - lon_d * 10000000) / 100;
//
//  sprintf(buf_rtty, "$$$$%s,%d,%02u%02u%02u,%s%d.%05ld,%s%d.%05ld,%ld,%d,%d,%d,%02x", callsign, send_cun,
//              gpsData.hours, gpsData.minutes, gpsData.seconds,
//              gpsData.lat_raw < 0 ? "-" : "", lat_d, lat_fl,
//              gpsData.lon_raw < 0 ? "-" : "", lon_d, lon_fl,
//              (gpsData.alt_raw / 1000), si4032_temperature, voltage, gpsData.sats_raw,
//              flaga);
//  CRC_rtty = 0xffff;                 //possibly not neccessary??
//  CRC_rtty = gps_CRC16_checksum(buf_rtty + 4);
//  sprintf(buf_rtty, "%s*%04X\n", buf_rtty, CRC_rtty & 0xffff);
//  //rtty_buf = buf_rtty;
//  //radio_enable_tx();
//  //tx_on = 1;
//
//  send_cun++;
//}


#ifdef  DEBUG
void assert_failed(uint8_t* file, uint32_t line)
{
    while (1);
}
#endif
