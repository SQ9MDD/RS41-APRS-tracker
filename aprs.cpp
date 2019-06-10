//
// Created by Admin on 2017-01-09.
//

#include "aprs.h"
#include "QAPRSBase.h"
#include "stdio.h"
#include "ublox.h"
#include "config.h"

volatile unsigned int proportional_counter = 0;

#if !defined(__OPTIMIZE__)
#error "APRS Works only when optimization enabled at level at least -O2"
#endif
QAPRSBase qaprs;

void aprs_init(){
  qaprs.init(0, 0, (char *) APRS_CALLSIGN, (const uint8_t) APRS_SSID, (char *) "APZQAP", '0', (char *) APRS_PATH);

}

void aprs_timer_handler() {
  qaprs.timerInterruptHandler();
}

uint8_t aprs_is_active() {
  return qaprs.enabled;
}

void calcDMH(long x, int8_t* degrees, uint8_t* minutes, uint8_t* h_minutes) {
  uint8_t sign = (uint8_t) (x > 0 ? 1 : 0);
  if (!sign) {
    x = -(x);
  }
  *degrees = (int8_t) (x / 1000000);
  x = x - (*degrees * 1000000);
  x = (x) * 60 / 10000;
  *minutes = (uint8_t) (x / 100);
  *h_minutes = (uint8_t) (x - (*minutes * 100));
  if (!sign) {
    *degrees = -*degrees;
  }
}

void aprs_send_status_ok(){
	char packet_buffer[30];
	sprintf(packet_buffer,">FIX OK");
	qaprs.sendData(packet_buffer);
}

void aprs_send_status(){
	char packet_buffer[30];
	sprintf(packet_buffer,">NO FIX");
	qaprs.sendData(packet_buffer);
}

void aprs_send_position(GPSEntry gpsData) {
  char packet_buffer[128];
  char altitude[10];
  char sats[7];
  char csespd[8];
  int8_t la_degrees, lo_degrees;
  uint8_t la_minutes, la_h_minutes, lo_minutes, lo_h_minutes;

  calcDMH(gpsData.lat_raw/10, &la_degrees, &la_minutes, &la_h_minutes);
  calcDMH(gpsData.lon_raw/10, &lo_degrees, &lo_minutes, &lo_h_minutes);

  // proportional pathing
  if(APRS_PROPORTIONAL_PATH == 1){
	  if(proportional_counter==0){
		  qaprs.init(0, 0, (char *) APRS_CALLSIGN, (const uint8_t) APRS_SSID, (char *) "APZQAP", '0', (char *) "");
	  }else if(proportional_counter==1){
		  qaprs.init(0, 0, (char *) APRS_CALLSIGN, (const uint8_t) APRS_SSID, (char *) "APZQAP", '0', (char *) "WIDE1-1");
	  }else if(proportional_counter==2){
		  qaprs.init(0, 0, (char *) APRS_CALLSIGN, (const uint8_t) APRS_SSID, (char *) "APZQAP", '0', (char *) "WIDE1-1,WIDE2-1");
	  }
  	proportional_counter++;
  	if(proportional_counter>=3){
  		proportional_counter = 0;
  	}
  }

  if(APRS_SEND_ALT == 1){
	  sprintf(altitude,"/A=%06ld",(gpsData.alt_raw/1000) * 3280 / 1000);
  }

  if(APRS_SEND_SAT == 1){
	  sprintf(sats," Sat%d",gpsData.sats_raw);	//
	  //sprintf(sats," Sat%d",gpsData.speed_raw);
  }

  //if(APRS_SEND_SPD == 1){
	  //sprintf(csespd,"%03ld/%03ld",gpsData.heading_raw,gpsData.speed_raw); // <- do poprawki
  //}
  	  if(APRS_USE_TACTICAL_CALLSIGN == 0){
	  sprintf(packet_buffer,
          ("!%02d%02d.%02u%c%s%03d%02u.%02u%c%s%s%s%s%s"),
		  abs(la_degrees),
		  la_minutes,
		  la_h_minutes,
          la_degrees > 0 ? 'N' : 'S',
          APRS_TABL,
          abs(lo_degrees),
          lo_minutes,
          lo_h_minutes,
          lo_degrees > 0 ? 'E' : 'W',
          APRS_ICON,
          csespd,
          altitude,
          APRS_COMMENT,
          sats);
  	  }else{
  		  sprintf(packet_buffer,
  	          (")%s!%02d%02d.%02u%c%s%03d%02u.%02u%c%s%s%s%s%s"),
  	          APRS_TACTICAL_CALLSIGN,
  			  abs(la_degrees),
  			  la_minutes,
  			  la_h_minutes,
  	          la_degrees > 0 ? 'N' : 'S',
  	          APRS_TACTICAL_TBL,
  	          abs(lo_degrees),
  	          lo_minutes,
  	          lo_h_minutes,
  	          lo_degrees > 0 ? 'E' : 'W',
  	          APRS_TACTICAL_ICON,
  	          csespd,
  	          altitude,
  	          APRS_COMMENT,
  	          sats);
  	  }

	  qaprs.sendData(packet_buffer);
	  //qaprs.sendData(gpsData.speed_raw);
}

void aprs_change_tone_time(uint16_t x) {
  qaprs._toneSendTime = x;
}
