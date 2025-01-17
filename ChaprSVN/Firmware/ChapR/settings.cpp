#include <Arduino.h>
#include <EEPROM.h>
#include "settings.h"
#include "config.h"
#include "button.h"
#include "VDIP.h"
#include "blinky.h"
#include "BT.h"
#include "sound.h"

extern button theButton;

extern blinky powerLED;
extern blinky indicateLED;

extern VDIP vdip;

extern BT bt;

settings::settings()
{
}

bool settings::isInitialized()
{
  char *currentMagic = getString(EEPROM_MAGIC,EEPROM_MAGICLENGTH);
  return (strcmp(currentMagic, EEPROM_MAGICSTRING) == 0);
}

void settings::markInitialized()
{
  setString(EEPROM_MAGIC,EEPROM_MAGICLENGTH,EEPROM_MAGICSTRING);
}

void settings::setResetStatus(byte stat)
{
  EEPROM.write(EEPROM_RSTATUS, stat);
}

byte settings::getResetStatus()
{
  return EEPROM.read(EEPROM_RSTATUS);
}

void settings::flushSerial()
{
  while (Serial.available() > 0){
    if (Serial.read() == '\r'){
      break;
    }
  }
}

void settings::hitReturn()
{
     Serial.println(F("Hit RET to continue"));
}

//
// getStringFromMonitor() - read a string from the serial monitor.  The data
//			    is put into the given buffer.  The given size is
//			    the size of the BUFFER not of the string.  The
//			    string will have a '\0' termination so the maximum
//			    string length will be size - 1.
//
//			    The neat thing about this routine is that you can
//			    use a real small buffer and it will only read that
//			    number of characters from the serial monitor and
//			    just flush the rest...very efficient!
//
int settings::getStringFromMonitor(char *buffer, int size)
{
  int index = 0;
  char incoming;
  
  while (index < size){			// only loop through for "size" loops
       incoming = '\0';
       if (Serial.available() > 0){
	    incoming = Serial.read();
	    if (incoming == '\r'){	// we stop when the user hits return
		 buffer[index] = '\0';
		 return index;
	    }
	    buffer[index++] = incoming;
       }
  }
  buffer[index - 1] = '\0';		// or we stop when we've run out of buffer
  flushSerial();			// need to flush until we get the return in this case
  return size;
}

void settings::boardBringUp()
{
  extern void software_Reset();

  flushSerial();
  
  char buf[25];
  extern sound beeper;
  buf[0] = ' ';
  Serial.print("Test prog v");
  Serial.println(BOARDBRINGUPVERSION);
  indicateLED.off();

  Serial.println("Power LED...");
  powerLED.on();
  hitReturn();
  getStringFromMonitor(buf, 2);

  Serial.println("BT LED...");
  powerLED.off();
  indicateLED.on();
  hitReturn();
  getStringFromMonitor(buf, 2);

  indicateLED.off();
  Serial.println("RET to squeep");
  getStringFromMonitor(buf, 2);

  beeper.squeep();
  Serial.println("WFS to cont.");
  while (theButton.check() != true){
  }

  while(true) {
       Serial.println("VDIP ver (3.69)...?");
       for (int i = 0; i < sizeof(buf); i++){
	    buf[i] ='\0';
       }
       vdip.cmd(VDIP_FWV, buf, DEFAULTTIMEOUT, 15); //expects 15 bytes back see pg 23 of Viniculum Firmware reference
       Serial.print(F("v"));
       Serial.println(buf);
       Serial.print(F("Enter \"!\" to flash"));
       hitReturn();

       getStringFromMonitor(buf, 25);
       if(buf[0] == '\0' || buf[0] != '!') {
	    break;		// if return or something other than !, go on with life
       }

       Serial.println("Put flash in USB 2; press RET.");
       getStringFromMonitor(buf, 25);

       Serial.println("wait 15 sec...");
       vdip.reset();
       delay(5000);
       vdip.flush(10000);
       Serial.print("Remove flash; ");
       hitReturn();
       getStringFromMonitor(buf, 25);
  }

  Serial.println("RN-42 ver (want 6.15)...?");
  bt.checkVersion();
  Serial.println(F("Done."));
}

//
// Here is a new way to read settings from the user.  It should save a TON of space.
// The structure below is an ordered list of settings that are asked of the user.
// The different types cause different "reading" behavior from the user.  The data
// is then written straight to EEPROM.
//
// NOTE - the prompt/help components are meant to be in PROGMEM, specified with F().
//	  The __FlashStringHelper declaration comes from Print.ccp in the Arduino libraries.

#define PROMPT_STRING	0
#define PROMPT_BYTE	1
#define PROMPT_BITS	2
#define PROMPT_SHORT	3
void settings::printCurrentValue(int eAddress, unsigned int max, uint8_t type)
{
  short num;
     switch(type) {

       case PROMPT_STRING:
	  Serial.print(getString(eAddress,max));
	  break;

       case PROMPT_BYTE:
	  Serial.print(EEPROM.read(eAddress));
	  break;

       case PROMPT_SHORT:
	  Serial.print(getShort(eAddress));
	  break;

       case PROMPT_BITS:
	  for (unsigned int i=0, d=EEPROM.read(eAddress); i < max; i++) {
	       Serial.print((d&0x01)?"1":"0");
	       d = d>>1;
	  }
     }
}

//
// doSetting() - given a pointer to a particular prompt, present it to the user and write
//		the data to the appropriate spot.  The current data (or default) is always
//		given as either a string or int, with interpretation of which one by the
//		type of the prompt.  The general format for the output is (for example):
//
//			Enter ChapR Name [ChapR-7]:
//
//		Note that the default is presented in the []'s - assuming that the user
//		understands that they just hit the RETURN for the default.  In other words,
//		there are no extra words telling them to "hit return for the default".
//
void settings::doSetting(int			 eAddress, 	// EEPROM address of this setting
			    const __FlashStringHelper *prompt,	// the general name or "prompt" for the given setting
			    const __FlashStringHelper *help,		// the "help" string - used only during setting, not confirmation
			    unsigned int   	 min,		// minimum value for error checking
			    unsigned int   	 max,		// maximum value for error checking
			    uint8_t	  	 type)		// how the input is read from the user
{
#define MAXLINE		50
     char	lineBuffer[MAXLINE];		// just statically set - will cover all read types
     long	num = 0;
     double     dnum;
     bool	invalid;
     unsigned int	i;

     while(true) {
	  Serial.print(F("Enter ChapR "));
	  Serial.print(prompt);
	  Serial.print(F(" ("));
	  Serial.print(help);
	  Serial.print(F(") ["));
	  printCurrentValue(eAddress,max,type);
	  Serial.println("]: ");

	  // now read - if RETURN is pressed with something, check and set the value
	  // appropriately.  If the user just presses RETURN do nothing.

	  getStringFromMonitor(lineBuffer, MAXLINE);	// only get MAXLINE characters - flushes the rest

	  if (lineBuffer[0] == '\0'){
	       // user just pressed RETURN with no data
	       return;
	  }

	  // go ahead and validate the values, write them later (saves a bit of space)

	  switch(type) {
	    case PROMPT_BYTE:
	    case PROMPT_SHORT:
		 num = atol(lineBuffer);
		 invalid = (num < min || num > max);
		 break;

	    case PROMPT_BITS:
	    case PROMPT_STRING:
		 invalid = (strlen(lineBuffer) < min || strlen(lineBuffer) > max);
		 break;
	  }

	  if(invalid) {
	       Serial.print(help);
	       Serial.println(F(". Try again."));
	       continue;				// jump back to the top of the loop
	  }

	  // if the value IS valid, store it

	  switch(type) {
	    case PROMPT_STRING:
		 setString(eAddress,max,lineBuffer);
		 break;

	    case PROMPT_BYTE:
		 EEPROM.write(eAddress,(byte)num);
		 break;

	    case PROMPT_BITS:
		 num = 0;
		 for(i=0; i < max; i++) {
		      switch(lineBuffer[i]) {
		      case '1':	num |= 1<<i; break;
		      case '0':	break;
		      default:	Serial.print(help); continue;
		      }
		 }
		 EEPROM.write(eAddress,(byte)num);
		 break;
	  }
	  
	  // if we got here, then we need to exit the loop
	  break;
     } 

     // before returning, print out the current value of the given address

     Serial.print(F("ChapR "));
     Serial.print(prompt);
     Serial.print(F(" is now \""));
     printCurrentValue(eAddress,max,type);
     Serial.println(F("\""));
}
     
void settings::setFromConsole()
{
     flushSerial();
     const __FlashStringHelper	*from00to50 = F("from 0.0 to 5.0");
     const __FlashStringHelper	*from0to255secs = F("from 0 to 255 secs");

     Serial.println(F("--- Enter Settings ---"));

     doSetting(EEPROM_NAME,		F("Name"),           F("max 15 chars"),           1, 15,    PROMPT_STRING);
     doSetting(EEPROM_TIMEOUT,		F("Timeout"),        F("0 (none) - 120 min"),     0, 120,   PROMPT_BYTE  );
     doSetting(EEPROM_PERSONALITY,	F("Personality"),    F("1 - 4"),                  1, 4,     PROMPT_BYTE  );
     doSetting(EEPROM_SPEED,		F("Lag"),            F("0 is none"),              0, 255,   PROMPT_BYTE  );
     doSetting(EEPROM_MODE,		F("Mode"),           F("0 or 1"),                 0, 1,     PROMPT_BYTE  );
     doSetting(EEPROM_AUTOLEN,		F("Auto Len"),       from0to255secs,              0, 255,   PROMPT_BYTE  );
     doSetting(EEPROM_TELELEN,		F("TeleOp Len"),     from0to255secs,              0, 255,   PROMPT_BYTE  );
     doSetting(EEPROM_ENDLEN,		F("Endgame Len"),    from0to255secs,              0, 255,   PROMPT_BYTE  );
     doSetting(EEPROM_MATCHMODE,	F("MatchMode Enabled"),  F("0 for false"),            0,   1,   PROMPT_BYTE  );

     markInitialized();
     loadCache();
     setResetStatus(0); //makes sure the ChapR knows it has not been (software) reset

     Serial.println(F("--- Done ---"));
     flushSerial();
}

void settings::setDefaults(char *name, 
			      unsigned int   timeout, 
			      unsigned int   personality, 
			      unsigned int   speed, 		// also known as lag
			      unsigned int   mode,
			      unsigned int   autoLen,
			      unsigned int   teleLen,
			      unsigned int   endLen,
                              unsigned int   matchmode)
{
     setName(name);
     setTimeout((byte)timeout);
     setPersonality((byte)personality);
     setSpeed((byte)speed);
     setMode((byte)mode);
     setAutoLen((byte)autoLen);
     setTeleLen((byte)teleLen);
     setEndLen((byte)endLen);
     setMatchModeEnable(matchmode);
}

//
// setString() - given a string and a starting position, sets the string in EEPROM, up to
//               the maxLength, which is the size of the EEPROM area for that string - 1. This
//               means the maximum string that can be written is maxLength, because
//               maxLength does not include the null terminator.

void settings::setString(int start, int maxLength, char *thing)
{
  int i = 0;
  while(maxLength--){
    EEPROM.write(start, thing[i]);
    start++;
    if (thing[i] != '\0'){
      i++;
    }
  }
  EEPROM.write(start, '\0'); //null terminates the string
}

//
// getString() - read in a string from EEPROM into a static  buffer, which needs to be
//		 copied before this routine is called again.  
//
//		 NOTE - this routine always reads ONE EXTRA than what length specified,
//		 and it is assumed that this one extra is a \0, but just in case it is
//		 automatically added at the last byte
//
char* settings::getString(int start, int length)
{
  int i;
  static char buffer[EEPROM_MAXSTRINGLENGTH];
  
  for (i = 0; i < length + 1; i++){
    buffer[i] = EEPROM.read(start);
    start++;
  }

  buffer[i-1] = '\0';
  
  return (buffer);
}

//
// setShort() - writes in little endian, starting from the index given
//
void settings::setShort(int start, short value)
{
  EEPROM.write(start,(byte) (value&0x00FF));
  EEPROM.write(start + 1, (byte) ((value&0xFF00)>>8));
}

//
// getShort() - read in little endian, starting from the index given
//
short settings::getShort(int start)
{
  short value = 0x00FF&((short) EEPROM.read(start));
  value |= 0xFF00&(((short) EEPROM.read(start+1))<<8);
  return value;
}

void settings::setName(char *name)
{
  // not cached
  setString(EEPROM_NAME,EEPROM_NAMELENGTH,name);
}

char* settings::getName()
{
  // still not cached
  return getString(EEPROM_NAME,EEPROM_NAMELENGTH);
}

void settings::setTimeout(byte time)
{
  timeout = time;
  EEPROM.write(EEPROM_TIMEOUT, time);
}

byte settings::getTimeout()
{
  return (timeout);
}

void settings::setPersonality(byte p)
{
  personality = p;
  EEPROM.write(EEPROM_PERSONALITY, p);
}

byte settings::getPersonality()
{
  return (personality);
}

void settings::setSpeed(byte s)
{
  speed = s;
  EEPROM.write(EEPROM_SPEED, s);
}

byte settings::getSpeed()
{
  return (speed);
}

void settings::setMode(byte m)
{
  mode = m;
  EEPROM.write(EEPROM_MODE, m);
}

byte settings::getMode()
{
  return (mode);
}

void settings::setAutoLen(byte a)
{
  autoLen = a;
  EEPROM.write(EEPROM_AUTOLEN, a);
}

byte settings::getAutoLen()
{
  return(autoLen);
}

void settings::setTeleLen(byte t)
{
  teleLen = t;
  EEPROM.write(EEPROM_TELELEN, t);
}

byte settings::getTeleLen()
{
  return(teleLen);
}

void settings::setEndLen(byte t)
{
  endLen = t;
  EEPROM.write(EEPROM_ENDLEN, t);
}

byte settings::getEndLen()
{
  return(endLen);
}

void settings::setMatchModeEnable(int m)
{
  matchModeEnable = m;
  EEPROM.write(EEPROM_MATCHMODE, m);
}

//
// matchModeIsEnabled() - returns true if matchMode can be used.
//                        See matchmode.cpp (or www.thechapr.com)
//                        for more information on matchMode.
bool settings::matchModeIsEnabled()
{
  return(matchModeEnable);
}

//
// loadCache() - reads in all of the settings from EEPROM and stores them
//               in runtime memory. This way the EEPROM won't be read
//               repeatedly during the loop (eventuall it would run out).
//
void settings::loadCache()
{
  timeout =  EEPROM.read(EEPROM_TIMEOUT);
  personality =  EEPROM.read(EEPROM_PERSONALITY);
  speed =  EEPROM.read(EEPROM_SPEED);
  mode = EEPROM.read(EEPROM_MODE);
  autoLen =  EEPROM.read(EEPROM_AUTOLEN);
  teleLen =  EEPROM.read(EEPROM_TELELEN);
  endLen =  EEPROM.read(EEPROM_ENDLEN);
  matchModeEnable = EEPROM.read(EEPROM_MATCHMODE);
}
