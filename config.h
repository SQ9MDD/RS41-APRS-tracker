//
// Created by SQ5RWU on 2016-12-24.
// Modified by SQ9MDD on 2019.05.09
//
// **************************** config ***************************
#define APRS_CALLSIGN 					"SQ9MDD" 				// put your APRS callsign here, 6 characters. If your callsign is shorter add spaces
#define APRS_SSID 						'B' 					// put your APRS SSID here allowed numbers are from 0-F (SSID 0-15)
#define APRS_TABL 						"/"						// icon table look at: http://www.aprs.org/symbols/symbolsX.txt
#define APRS_ICON						"["						// icon symbol
#define APRS_COMMENT 					"Na spacerze z psem"	// comment
//
#define APRS_USE_TACTICAL_CALLSIGN		1						// enable/disable tactical callsign insted callsign, remember you sended this using primary callsign and SSID
#define APRS_TACTICAL_CALLSIGN			"ABRA"					// optional tactical name for your item up 9 characters (This is an ITEM from APRS spec. in this case my pet dog's name)
#define APRS_TACTICAL_TBL				"/"						// icon table for tactical call sign look at: http://www.aprs.org/symbols/symbolsX.txt
#define APRS_TACTICAL_ICON				"p"						// icon symbol for tacti
#define APRS_TACTICAL_COMMENT			""						// comment for tactical
//
#define APRS_PATH						""						// APRS DIGI PATH
#define APRS_PROPORTIONAL_PATH 			1						// enable proportional path algorythm this option override main path see: http://www.aprs.org/newN/ProportionalPathing.txt
#define APRS_SEND_ALT					0						// enable/disable sending altitude
#define APRS_SEND_SAT					0						// enable/disable sending SAT condition
//#define APRS_SEND_SPD					0						// course and speed
#define APRS_INTERVAL					60						// number of seconds between packets, DO NOT SETUP LESS THAN 60s
//#define APRS_SMARTBIKON				0						// enable/disable smart bikoning option
#define APRS_FREQUENCY  				432.500f 				// Mhz middle frequency usually 432.500MHz
#define TX_POWER  						0 						// PWR 0...7 0- MIN ... 7 - MAX

// !!!!! PLEASE FOLLOW THE RULES !!!!!

// SSID:
// 0 --> Your primary station usually fixed and message capable
// 1 --> generic additional station, digi, mobile, wx, etc.
// 2 --> generic additional station, digi, mobile, wx, etc.
// 3 --> generic additional station, digi, mobile, wx, etc.
// 4 --> generic additional station, digi, mobile, wx, etc.
// 5 --> Other network sources (Dstar, Iphones, Blackberry's etc)
// 6 --> Special activity, Satellite ops, camping or 6 meters, etc.
// 7 --> walkie talkies, HT's or other human portable
// 8 --> boats, sailboats, RV's or second main mobile
// 9 --> Primary Mobile (usually message capable)
// A (10) --> internet, Igates, echolink, winlink, AVRS, APRN, etc.
// B (11) --> balloons, aircraft, spacecraft, etc.
// C (12) --> APRStt, DTMF, RFID, devices, one-way trackers*, etc.
// D (13) --> Weather stations
// E (14) --> Truckers or generally full time drivers
// F (15) --> generic additional station, digi, mobile, wx, etc.
//
// APRS DIGI recommended path:
// mobile: WIDE1-1,WIDE2-1
// fixed: WIDE2-1
// flying objects: no path leave: ""
//
// Power levels:
// 0 --> -1dBm
// 1 --> 2dBm
// 2 --> 5dBm
// 3 --> 8dBm
// 4 --> 11dBm
// 5 --> 14dBm
// 6 --> 17dBm
// 7 --> 20dBm
