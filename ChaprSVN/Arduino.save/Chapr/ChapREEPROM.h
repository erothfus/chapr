#include "config.h"

#ifndef CHAPREEPROM_H
#define CHAPREEPROM_H

//Memory Map for Arduino EEPROM for ChapR
//
// bytes(inclusive)   description
// ------------------------------
// 0 - 15              null-terminated ChapR name
// 16                  minutes before timeout (0 means it never times out and the maximum is 255)
// 17                  personality (numbered based on information from the website)
// 18 - 23             null-terminated "ChapR" to verify EEPROM has been written

#define EEPROM_NAME             0
#define EEPROM_TIMEOUT         16
#define EEPROM_PERSONALITY     17
#define EEPROM_MAGIC           18
#define EEPROM_RSTATUS         24
#define EEPROM_SPEED           25 //23 - 33 is 0
#define EEPROM_MODE            26


//constants
#define EEPROM_NAMELENGTH      15 //without null terminator (15 - 0)
#define EEPROM_MAGICLENGTH      5 //without null terminator (23 - 18)
#define EEPROM_MAXSTRINGLENGTH (max(EEPROM_NAMELENGTH, EEPROM_MAGICLENGTH) + 1)
#define EEPROM_MAXTIMEOUT      120 //says timeout cannot be longer than two hours
#define EEPROM_LASTPERSON      PERSONALITYCOUNT //last personality that has been coded
#define EEPROM_MAXLAG          255
#define EEPROM_MAXMODE         1
#define EEPROM_MAGICSTRING   "Chap2" //number shows a new set of EEPROM settings

class ChapREEPROM
{
public:
     ChapREEPROM();

     void setName(char*);
     char *getName();
     void setTimeout(byte);
     byte getTimeout();
     void setPersonality(byte);
     byte getPersonality();
     void setSpeed(byte);
     byte getSpeed();
     void setMode(byte);
     byte getMode();
     bool isInitialized();
     void setFromConsole(char *,byte, byte, byte, byte);
     void boardBringUp();
     void setResetStatus(byte);
     byte getResetStatus();
     
private:
     int  getStringFromMonitor(char*, int);
     void setString(int, int, char*);
     char *getString(int, int);
     void markInitialized();
     void flushSerial();
};


#endif CHAPREEPROM_H
