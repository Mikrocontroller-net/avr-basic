/*
 ********************************************************
 *         Zugriffsroutinen auf RTC DS1307 
 *         =============================== 
 *              Uwe Berger; 2011
 * 
 * DS1307-Register
 * ---------------
 * 0 --> Sekunde
 * 1 --> Minute
 * 2 --> Stunde
 * 3 --> Tag in der Woche
 * 4 --> Tag
 * 5 --> Monat
 * 6 --> Jahr (2-stellig)
 *   (bis hier sind die Daten BCD-codiert)
 * ...
 * ..--> Rest siehe Datenblatt ;-)
 * 
 * Have fun!
 * ---------
 * 
 ********************************************************
 */

#include "i2cmaster.h"
#include "ds1307.h"


unsigned char get_DS1307(unsigned char reg) { 
	unsigned char ret; 
	i2c_start(DS1307+I2C_WRITE); 
	i2c_write(reg); 
	i2c_stop(); 
	i2c_start(DS1307+I2C_READ); 
	ret = i2c_readNak(); 
	i2c_stop();
	return ((ret>>4)*10 + (ret&15)); // BCD2INT
 }

