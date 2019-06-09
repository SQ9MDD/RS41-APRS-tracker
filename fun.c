#include <stm32f10x_usart.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_gpio.h>

const char ascii[] = "0123456789ABCDEF";
int srednia_u[5] = {0, 0, 0, 0, 0};



int HexCharToInt(char ch) {
  if (ch < 48 || (ch > 57 && ch < 65) || ch > 70) return 0;
  return (ch < 58) ? ch - 48 : ch - 55;
}

void print(char *s) {
  #ifdef DEBUG
  while (*s) {
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {
    }
    USART_SendData(USART3, *(s++));
  }
  #endif
}

void send_hex(unsigned char data) {
  #ifdef DEBUG
  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {
  }
  USART_SendData(USART3, ascii[data >> 4]);
  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {
  }
  USART_SendData(USART3, ascii[data & 0x0f]);
  #endif
}



uint16_t gps_CRC16_checksum(char *string) {
  uint16_t crc = 0xffff;
  char i;
  while (*(string) != 0) {
    crc = crc ^ (*(string++) << 8);
    for (i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = (uint16_t) ((crc << 1) ^ 0x1021);
      else
        crc <<= 1;
    }
  }
  return crc;
}

int srednia(int dana) {
  static uint8_t nr_pom = 0;
  static uint8_t first = 1;
  uint8_t i;
  int sr = 0;
  if (first) {
    for (i = 0; i < 5; i++) {
      srednia_u[i] = dana;
    }
    first = 0;
  }
  srednia_u[nr_pom] = dana;
  if (++nr_pom > 4) {
    nr_pom = 0;
  }
  for (i = 0; i < 5; i++) {
    sr += srednia_u[i];
  }
  sr = sr / 5;
  return sr;
}
