/*
	Copyright (C) 2013 Lukasz Nidecki SQ5RWU

    This file is part of ArduinoQAPRS.

    ArduinoQAPRS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ArduinoQAPRS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArduinoQAPRS; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */

/**
 * @file
 * @brief Headers for the QAPRSBase class
 */

#ifndef QAPRSBASE_H_
#define QAPRSBASE_H_

#include "QAPRSCommon.h"

/**
 * @brief Base class for APRS broadcasting
 * @details The class serves as the basis for the sub-implementation of AFSK generation.
 */
class QAPRSBase {
private:
	/**
         * @brief checksum of the package
	 */
	uint16_t ax25CRC;
	/**
	 * @brief amount of sync bytes to send before packet content
	 */
	static const uint8_t ax25HeaderFlagFieldCount1200 = 25;
	/**
	 * @brief amount of sync bytes to send before packet content
	 */
	static const uint8_t ax25HeaderFlagFieldCount300 = 90; //45;
	/**
	 * @brief amount of sync bytes to send before packet content
	 */
	uint8_t ax25HeaderFlagFieldCount;
	/**
	 * @brief Flag
	 * @details <http://www.tapr.org/pub_ax25.html#2.2.1>
	 */
	static const uint8_t ax25FlagFieldValue = 0x7E;
	/**
         * @brief Sending time of a single tone. In us.
         * @details Total time should be 833us. The value given here takes into account the delay associated with calling
         * functions etc.
	 */
#if F_CPU == 16000000L
	static const uint16_t toneSendTime1200 = 815;
#elif F_CPU == 8000000UL
	static const uint16_t toneSendTime1200 = 785;
#else
	//static const uint16_t toneSendTime1200 = 1000000/1200;
#endif
	/**
         * @brief Sending time of a single tone. In ms.
         * @details Total time should be 4 * 833ms. The value given here takes into account the delay associated with calling
         * functions etc.
	 */
	static const uint16_t toneSendTime300 = 815 + 3 * 833;
	/**
         * @brief Waiting time for the channel to be released.
         * @details Every 100ms we check if @see canTransmit can be sent
	 */
	static const uint16_t channelFreeWaitingMS = 1; // 2000 ms
	/**
	 * @brief Default time between transmitting and starting AFSK generation
	 */
	static const uint16_t defaultTxDelay = 1; // 300 ms
	/**
	 * @brief Arduino Pin on which we set the logical 1 when transmitting
	 */
	int8_t txEnablePin;
	/**
	 * @brief Temporary buffer
	 */
	uint8_t tmpData[255];
	/**
	 * @brief Delay in ms between setting txEnablePin output high and starting AFSK generation
	 */
	uint16_t txDelay;
	/**
	 * @brief
	 */
	char from_addr[8];
	/**
	 * @brief
	 */
	uint8_t from_ssid;
	/**
	 * @brief
	 */
	char to_addr[6];
	/**
	 * @brief
	 */
	uint8_t to_ssid;
	/**
	 * @brief
	 */
	char* relays[3*7];

	uint8_t canTransmit();
	void ax25SendHeaderBegin();
	void ax25SendByte(uint8_t byte);
	void ax25SendFooter();
	void ax25CalcCRC(uint8_t ls_bit);
	void parseRelays(const char * relays, char * dst);
protected:
	/**
	 * @brief The Arduino pin [in] that must be low (or high if the number is negative) to start transmitting.
	 */
	int8_t sensePin;
	/**
	 * @brief Tone currently generated
	 */
	QAPRSSendingTone currentTone;
	/**
	 * @brief Currently used variant
	 */
	QAPRSVariant variant;

	void initializeRadio();
	void enableTransmission();
	void disableTranssmision();

	void toggleTone();
	void initializeTimer1();
	void delayuSeconds(uint16_t us);
	void doTxDelay();
public:
	QAPRSBase() {};
	QAPRSReturnCode send(char * buffer);
	QAPRSReturnCode send(char * buffer, size_t length);
	QAPRSReturnCode send(char * from_addr, uint8_t from_ssid, char * to_addr, uint8_t to_ssid, char * packet_content);
	QAPRSReturnCode send(char * from_addr, uint8_t from_ssid, char * to_addr, uint8_t to_ssid, char * relays, char * packet_content);
	QAPRSReturnCode send(char * from_addr, uint8_t from_ssid, char * to_addr, uint8_t to_ssid, char * relays, char * packet_content, size_t length);
	QAPRSReturnCode sendData(char * buffer);
	QAPRSReturnCode sendData(char * buffer, size_t length);

	void init(int8_t sensePin, int8_t txEnablePin);
	void init(int8_t sensePin, int8_t txEnablePin, char * from_addr, uint8_t from_ssid, char * to_addr, uint8_t to_ssid, char * relays);

	void setTxDelay(uint16_t txDelay);
	void timerInterruptHandler();
	void setFromAddress(char * from_addr, uint8_t from_ssid);
	void setToAddress(char * to_addr, uint8_t to_ssid);
	void setRelays(char * relays);
	void setVariant(QAPRSVariant variant = QAPRSVHF);

private:
  static const uint16_t toneSendTime = 833;

	static const uint16_t toneSendTime1200 = 795;
  static const uint16_t MarkTimerValue = 393;
  static const uint16_t SpaceTimerValue = 202;
  void togglePin();
  uint8_t pin;
public:
  uint8_t enabled;
  uint16_t timer1StartValue;
	uint16_t _toneSendTime;
};

/**
 * @brief Shift byte 1 bit to the left. Used in the header ax.25
 */
#define SHIFT_BYTE(x) (x << 1)

#endif /* QAPRSBASE_H_ */
