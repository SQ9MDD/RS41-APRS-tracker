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
 * @brief Implementation of the QAPRSBase class
 */

#include "QAPRSBase.h"
#include "delay.h"
#include "radio.h"
#include "init.h"

QAPRSBase * QAPRSGlobalObject;
/**
 * Can data be sent?
 * @return 1 if transmission is possible, 0 otherwise
 */
uint8_t QAPRSBase::canTransmit(){
	return 1;
}

/**
 * Start broadcasting the ax.25 package
 * Additionally:
 * - enable broadcasting @see enableTransmission
 * - initialize crc and tone
 * The number of bytes for timing is specified by @see ax25HeaderFlagFieldCount
 */
void QAPRSBase::ax25SendHeaderBegin() {
	this->enableTransmission();
	timerInterruptHandler();
	this->ax25CRC = 0xFFFF;
	this->currentTone = QAPRSMark;
	//this->currentTone = QAPRSSpace;

	for (uint8_t i=0;i<this->ax25HeaderFlagFieldCount;i++)
	{
		this->ax25SendByte(this->ax25FlagFieldValue);
	}
}

/**
 *Send packet termination, that is 2 bytes of checksum and packet end flag @see QAPRSBase::ax25FlagFieldValue
 */
void QAPRSBase::ax25SendFooter() {
	/**
	 * 16-bit CRC-CCITT
	 * MBS + LE!!!
	 * @see: http://www.tapr.org/pub_ax25.html#2.2.7
	 * @see: http://www.tapr.org/pub_ax25.html#2.2.
	 */
	static uint8_t tmp;
	tmp = (uint8_t) ((ax25CRC >> 8) ^ 0xff);

	this->ax25SendByte((uint8_t) ((this->ax25CRC) ^ 0xff));
	this->ax25SendByte(tmp);
	this->ax25SendByte(this->ax25FlagFieldValue);
	this->disableTranssmision();
}

/**
 * Sends the ax.25 byte
 * @param byte Byte to send
 */
void QAPRSBase::ax25SendByte(uint8_t byte) {
	static uint8_t i;
	static uint8_t ls_bit;
	static uint8_t is_flag;
	static uint8_t bit_stuffing_counter;


	// we write down whether the transmitted bit is not a flag - because we send it in a special way
	is_flag = (uint8_t) (byte == this->ax25FlagFieldValue);

	for(i=0;i<8;i++){
		// transmit from the least significant bit
		ls_bit = (uint8_t) (byte & 1);

		if (is_flag){
			bit_stuffing_counter = 0;
		} else {
			this->ax25CalcCRC(ls_bit);
		}

		if (ls_bit){
			bit_stuffing_counter++;
			if (bit_stuffing_counter == 5){
				// if we have 5 bits with a value of 1 in a row then
				delayuSeconds(this->_toneSendTime);
				this->toggleTone();
				bit_stuffing_counter = 0;
			}
		} else {
			bit_stuffing_counter = 0;
			this->toggleTone();
		}
		byte >>= 1;
		delayuSeconds(this->_toneSendTime);
	}
}

/**
 * Adds a bit to the frame's checksum
 * @param ls_bit Data bit
 */
void QAPRSBase::ax25CalcCRC(uint8_t ls_bit) {
	static unsigned short	crc_tmp;

	crc_tmp = this->ax25CRC ^ ls_bit;				// XOR lsb of CRC with the latest bit
	this->ax25CRC >>= 1;									// Shift 16-bit CRC one bit to the right

	if (crc_tmp & 0x0001)					// If XOR result from above has lsb set
	{
		this->ax25CRC ^= 0x8408;							// Shift 16-bit CRC one bit to the right
	}
}
/**
 * Switch the currently generated tone. @see QAPRSSendingTone
 */
inline void QAPRSBase::toggleTone() {
	this->currentTone = (this->currentTone == QAPRSSpace) ? QAPRSMark : QAPRSSpace;
  this->timer1StartValue = (this->currentTone == QAPRSMark) ? MarkTimerValue : SpaceTimerValue;
}

/**
 * Initialize Timer1 which is used to generate MARK and SPACE
 */
void QAPRSBase::initializeTimer1() {
    #if defined(__arm__)
    //TODO: rewrite to STM32
    #else
        noInterrupts();
        TIMSK1 |= _BV(TOIE1);
        TCCR1A = 0;
        TCCR1C = 0;
        interrupts();
    #endif

}

/**
 *Initialize radio and pins.
 */
void QAPRSBase::initializeRadio() {
    #if defined(__arm__)
    //TODO: rewrite to STM32
    #else
        if (this->sensePin){
            pinMode(abs(this->sensePin), INPUT);
            digitalWrite(abs(this->sensePin), LOW);
        }
        pinMode(abs(this->txEnablePin), OUTPUT);
    #endif

	this->disableTranssmision();
	this->initializeTimer1();
}

/**
 * Turn on broadcast. The method additionally performs a transmit delay, if set. @see QAPRSBase :: setTxDelay
 */
void QAPRSBase::enableTransmission() {
    #if defined(__arm__)
    //TODO: rewrite to STM32
    #else
        digitalWrite(abs(this->txEnablePin), (this->txEnablePin > 0) ? HIGH : LOW);
    #endif

  radio_set_tx_frequency(APRS_FREQUENCY);
  radio_rw_register(0x72, 5, 1);
  GPIO_SetBits(GPIOC, radioNSELpin);
  radio_rw_register(0x71, 0b00010010, 1);
  spi_deinit();
  this->enabled = 1;
	this->doTxDelay();
}

/**
 * Turn off broadcasting.
 */
void QAPRSBase::disableTranssmision() {
    #if defined(__arm__)
    //TODO: rewrite to STM32
    #else
	    digitalWrite(abs(this->txEnablePin), (this->txEnablePin > 0) ? LOW : HIGH);
    #endif
  spi_init();
  this->enabled = 0;
  radio_rw_register(0x71, 0x00, 1);
  init_timer(100);
}

/**
 * Send data buffer. @warning This method will exit with a byte of 0 if it occurs.
 * @param buffer Buffer with data to be sent
 * @return
 */
QAPRSReturnCode QAPRSBase::send(char * buffer) {
	return this->send(buffer, strlen(buffer));
}

/**
 * Send APRS details.
 * @param from_addr Source [character] address. Max 6 characters!
 * @param from_ssid Source station SSID: bytes from '0' to 'F'
 * @param to_addr Destination [character] address. Max 6 characters!
 * @param to_ssid Destination station SSID: bytes '0' to 'F'
 * @param packet_content Buffer with APRS packet data
 * @return
 */
QAPRSReturnCode QAPRSBase::send(char * from_addr, uint8_t from_ssid, char * to_addr, uint8_t to_ssid, char * packet_content) {
	ax25BaseFrame * bf;
	bf = (ax25BaseFrame *)tmpData;
	memset(bf->from, ' ', 6);
	strncpy(bf->from, from_addr,6);
	memset(bf->to, ' ', 6);
	strncpy(bf->to, to_addr, 6);
	bf->to_ssid = to_ssid;
	bf->from_ssid = (uint8_t) (from_ssid > '@' ? from_ssid - 6 : from_ssid);;

	strcpy(bf->packet_content, packet_content);

	bf->control_field = 0x03;
	bf->protocolID = 0xf0;


	uint8_t i;
	for(i=0;i<14;i++){
		tmpData[i] = SHIFT_BYTE(tmpData[i]);
	}
	tmpData[13] |= 1;

	return this->send((char*)tmpData);
}

/**
 * Sends a data buffer of the marked length.
 * @param buffer Buffer with data to be sent
 * @param length The length of the buffer
 * @return
 */
QAPRSReturnCode QAPRSBase::send(char* buffer, size_t length) {
	int16_t timeout = this->channelFreeWaitingMS;

	while(timeout > 0){
		// jesli mozna nadawac to nadajemy
		if (this->canTransmit()){
			this->ax25SendHeaderBegin();
			while(length--){
				this->ax25SendByte((uint8_t) *buffer);
				buffer++;
			}
			this->ax25SendFooter();
			return QAPRSReturnOK;
		} else {
			// if not possible, wait 100ms and check again
			// max wait time is channelFreeWaitingMS
            #if defined(__arm__)
                _delay_ms(100);
            #else
                delay(100);
            #endif
			timeout -= 100;
		}
	}
	return QAPRSReturnErrorTimeout;
}

/**
 * Send APRS details.
 * @param from_addr Source [character] address. Max 6 characters!
 * @param from_ssid Source station SSID: bytes from '0' to 'F'
 * @param to_addr Destination [character] address. Max 6 characters!
 * @param to_ssid Destination station SSID: bytes '0' to 'F'
 * @param relays Buffer with relays data. Eg "WIDE1 1" @warning If 2 or more items are to be given then write them WITHOUT commas etc. eg "WIDE1 1WIDE2 1"
 * @param packet_content Buffer with APRS packet data
 * @return
 */
QAPRSReturnCode QAPRSBase::send(char* from_addr, uint8_t from_ssid, char* to_addr, uint8_t to_ssid, char* relays, char* packet_content) {
	return this->send(from_addr, from_ssid, to_addr, to_ssid, relays, packet_content, strlen(packet_content));
}

/**
* Send APRS details.
 * @param from_addr Source [character] address. Max 6 characters!
 * @param from_ssid Source station SSID: bytes from '0' to 'F'
 * @param to_addr Destination [character] address. Max 6 characters!
 * @param to_ssid Destination station SSID: bytes '0' to 'F'
 * @param relays Buffer with relays data. Eg "WIDE1 1" @warning If 2 or more items are to be given then write them WITHOUT commas etc. eg "WIDE1 1WIDE2 1"
 * @param packet_content Buffer with APRS packet data
 * @param length The length of packet_content
 * @return
 */
QAPRSReturnCode QAPRSBase::send(char* from_addr, uint8_t from_ssid, char* to_addr, uint8_t to_ssid, char* relays, char* packet_content, size_t length) {
	ax25CustomFrameHeader * bf;
	bf = (ax25CustomFrameHeader *)tmpData;
	memset(bf->from, ' ', 6);
	strncpy(bf->from, from_addr,6);
	memset(bf->to, ' ', 6);
	strncpy(bf->to, to_addr, 6);
	bf->to_ssid = to_ssid;
	bf->from_ssid = (uint8_t) (from_ssid > '@' ? from_ssid - 6 : from_ssid);

	uint8_t relay_size = (uint8_t) strlen(relays);
	strcpy((char*)(tmpData+sizeof(ax25CustomFrameHeader)), relays);

	uint8_t i;
	for(i=0;i<sizeof(ax25CustomFrameHeader)+relay_size;i++){
		tmpData[i] = SHIFT_BYTE(tmpData[i]);
	}
	// ostatni bit w adresach musi byc ustawiony na 1
	tmpData[sizeof(ax25CustomFrameHeader)+relay_size - 1] |= 1;

	// control_field
	tmpData[(sizeof(ax25CustomFrameHeader)+relay_size)] = 0x03;
	// protocolID
	tmpData[(sizeof(ax25CustomFrameHeader)+relay_size+1)] = 0xf0;


	strncpy((char*)(tmpData+sizeof(ax25CustomFrameHeader)+relay_size+2), packet_content, length);

	return this->send((char*)tmpData, (sizeof(ax25CustomFrameHeader)+relay_size+2+length));
}

/**
 * Sends APRS @warning data. This method will exit with a byte of 0 if present.
 * @param buffer Buffer with the APRS part of the frame - i.e. only pure APRS data, no SSIDs, etc.
 * @return Status of the operation
 */
QAPRSReturnCode QAPRSBase::sendData(char* buffer) {
	return this->send((char*)this->from_addr, this->from_ssid, (char*)this->to_addr, this->to_ssid, (char*)this->relays, buffer);
}

/**
 * Sends APRS data
 * @param buffer Buffer with the APRS part of the frame - i.e. only pure APRS data, no SSIDs, etc.
 * @param length The length of the buffer
 * @return Status of the operation
 */
QAPRSReturnCode QAPRSBase::sendData(char* buffer, size_t length) {
	return this->send((char*)this->from_addr, this->from_ssid, (char*)this->to_addr, this->to_ssid, (char*)this->relays, buffer, length);
}

/**
 * Library initialization
 * @param sensePin Pin [we] Arduino on which 0 means consent to transmit and 1 means lack of it.
 * Specifying 0 as the pin number turns broadcast detection OFF and forces the programmer to handle it himself!
 * @param txEnablePin Pin [out] of the Arduino with 1 being transmit
 */
void QAPRSBase::init(int8_t sensePin, int8_t txEnablePin) {
	QAPRSGlobalObject = this;
	this->sensePin = sensePin;
	this->txEnablePin = txEnablePin;
	this->txDelay = this->defaultTxDelay;
	this->setVariant();
  this->timer1StartValue = MarkTimerValue;

	this->initializeRadio();
}

/**
* Initialize the library for the sendData @see method QAPRSBase :: sendData
 * @param sensePin sensePin Pin [we] Arduino on which 0 means consent to send and 1 means lack of it.
 * Specifying 0 as the pin number turns broadcast detection OFF and forces the programmer to handle it himself!
 * @param txEnablePin Pin [out] of the Arduino with 1 being transmit
 * @param from_addr Source [character] address. Max 6 characters!
 * @param from_ssid Source station SSID: bytes from '0' to 'F'
 * @param to_addr Destination [character] address. Max 6 characters!
 * @param to_ssid Destination station SSID: bytes '0' to 'F'
 * @param relays Relays eg "WIDE1-1, WIDE2-1". max. 3 elements, separated by commas!
 */
void QAPRSBase::init(int8_t sensePin, int8_t txEnablePin, char* from_addr, uint8_t from_ssid, char* to_addr, uint8_t to_ssid, char* relays) {
	this->init(sensePin, txEnablePin);
	this->setFromAddress(from_addr, from_ssid);
	this->setToAddress(to_addr, to_ssid);
	this->setRelays(relays);
}

/**
 * Set the destination address for the sendData method
 * @param from_addr Source [character] address. Max 6 characters!
 * @param from_ssid Source station SSID: bytes from '0' to 'F'
 */
void QAPRSBase::setFromAddress(char* from_addr, uint8_t from_ssid) {
	memset(this->from_addr, ' ',6);
	strcpy(this->from_addr, from_addr);
	this->from_ssid = from_ssid;
}

/**
 * Set the destination address for the sendData method
 * @param to_addr Destination [character] address. Max 6 characters!
 * @param to_ssid Destination station SSID: bytes '0' to 'F'
 */
void QAPRSBase::setToAddress(char* to_addr, uint8_t to_ssid) {
	memset(this->to_addr, ' ', 6);
	strcpy(this->to_addr, to_addr);
	this->to_ssid = to_ssid;
}

/**
 * Convert track records (relays) from human format to ax.25 format
 * @param relays eg "WIDE1-1, WIDE2-1". max. 3 elements, separated by commas! @see http://www.aprs.pl/sciezka.htm
 * @param dst
 */
void QAPRSBase::parseRelays(const char* relays, char* dst) {
	uint8_t relays_len = (uint8_t) strlen(relays);
	uint8_t relays_ptr = 0;
	uint8_t dst_ptr = 0;
	uint8_t fill_length = 0;

	for(relays_ptr=0;relays_ptr<relays_len;relays_ptr++){
		if (relays[relays_ptr] == ',' || relays_ptr == relays_len-1){
			if (relays[relays_ptr] != ','){
				dst[dst_ptr] = relays[relays_ptr] == '-' ? ' ' :  relays[relays_ptr]; // exchange ',' for ' '
				dst_ptr++;
			}
			// koniec elementu
			if (dst_ptr < 7){
				fill_length = (uint8_t) (7 - dst_ptr);
			} else if (dst_ptr > 7 && dst_ptr < 7+7){
				fill_length = (uint8_t) (7 + 7 - dst_ptr);
			} else if(dst_ptr > 7+7 && dst_ptr < 7+7+7){
				fill_length = (uint8_t) (7 + 7 + 7 - dst_ptr);
			}
			while(fill_length){
				dst[dst_ptr] = ' ';
				fill_length--;
				dst_ptr++;
			}
		} else {
			dst[dst_ptr] = relays[relays_ptr] == '-' ? ' ' :  relays[relays_ptr]; // exchange ',' for ' '
			dst_ptr++;
		}
	}
	dst[dst_ptr] = 0;
}

/**
 * Implements a delay, if set
 */
void QAPRSBase::doTxDelay() {
	if (this->txDelay){
        #if defined(__arm__)
            _delay_ms(this->txDelay);
        #else
            delay(this->txDelay);
        #endif
	}
}

void QAPRSBase::setVariant(QAPRSVariant variant) {
	this->variant = variant;
	switch(variant){
	case QAPRSVHF:
		this->_toneSendTime = this->toneSendTime1200;
		this->ax25HeaderFlagFieldCount = this->ax25HeaderFlagFieldCount1200;
		break;
	case QAPRSHF:
		this->_toneSendTime = this->toneSendTime300;
		this->ax25HeaderFlagFieldCount = this->ax25HeaderFlagFieldCount300;
		break;
	}
}

/**
 * Set relay paths for send Data @see QAPRS Base :: send Data 
 * @param relays
 */
void QAPRSBase::setRelays(char* relays) {
	this->parseRelays(relays, (char*)this->relays);
}


/**
 * Delay function. Unlike delayMicroseconds with Arduino, this function checks the real time.
 * @param us
 */
void QAPRSBase::delayuSeconds(uint16_t us) {
    #if defined(__arm__)
        _delay_us(us, 1);
    #else
        unsigned long time = micros();
        while(micros() - time < us){
            //asm("nop");
        }
    #endif
}


/**
 * Set a delay time between transmitting and starting the signal generation
 * @param txDelay Time in ms
 */
void QAPRSBase::setTxDelay(uint16_t txDelay) {
	this->txDelay = txDelay;
}

void QAPRSBase::timerInterruptHandler() {
  this->togglePin();
  TIM2->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));

  TIM2->ARR = this->timer1StartValue;
  TIM2->CNT = 0;

  TIM2->CR1 |= TIM_CR1_CEN;
}

void QAPRSBase::togglePin() {
  if (this->pin){
    this->pin = 0;
    GPIO_ResetBits(GPIOB, radioSDIpin);
  } else {
    this->pin = 1;
    GPIO_SetBits(GPIOB, radioSDIpin);
  }
}

#if defined(__arm__)
//TODO: rewrite to STM32
#else
ISR (TIMER1_OVF_vect)  // timer1 overflow interrupt
{
	QAPRSGlobalObject->timerInterruptHandler();
}
#endif
