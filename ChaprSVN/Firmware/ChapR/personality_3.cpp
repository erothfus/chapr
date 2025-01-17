//
// personality_3.cpp
//
//   Implements the personality:  RIO-LabView
//

#include <Arduino.h>
#include "VDIP.h"
#include "BT.h"
#include "gamepad.h"
#include "nxt.h"
#include "RIO.h"
#include "matchmode.h"
#include "personality.h"
#include "personality_3.h"
#include "robotc.h"
#include "sound.h"
#include "settings.h"
#include "debug.h"

RIO   RIO;                    // the container for all of the RIO calls (makes life pretty)
extern settings myEEPROM;
extern sound beeper;

//
// matchStateProcess() - the matchmode callback that is used by matchmode to do
//		  whatever needs to be done with the state changes during
//		  match mode.  The NEW state that was just entered is
//		  passed to the callback routine.
//
//	NOTE: this routine is handed the "rock" that was given to it to, so that
//		it could be handed back during callbacks.
//
//	NOTE: Processing of the MM_OFF state needs to be "idempotent" - it should
//		be callable any number of times.
//
bool Personality_3::matchStateProcess(mmState mmState, void *rock)
{
     switch(mmState) {
     case MM_OFF:
	  enabled = false;	// ensure we are always disabled when in OFF state
	  mode = MODE_AUTO; 
	  break;

     case MM_AUTO_PREP:		// prepare for autonomous - allow the standard "true" to be returned
	  mode = MODE_AUTO;	//   because we don't wait for anything to get going in autonomous
	  break;

     case MM_AUTO_START:
          beeper.beep();
	  enabled = true;
	  break;

     case MM_AUTO_END:		// autonomous is ending
          enabled = false;
          beeper.boop();
	  break;

     case MM_TELEOP_PREP:      	// prepare for teleop
	  mode = MODE_TELEOP;		// Note that this does the start program rising sound too
	  return(false);		// return false to cause wait for button press to start teleop

     case MM_TELEOP_START:	// teleop is starting
	  enabled = true;
	  beeper.beep();
	  break;

     case MM_ENDGAME_START:	// endgame (within teleop) is starting
	  // need an endgame start sound here
          beeper.beep();
	  break;

     case MM_TELEOP_END:	// teleop is ending
          enabled = false;
	  beeper.boop();
	  // need a end of game sound here
	  // FALL THROUGH to next case - 'cause we need to kill the program at the end of Teleop

     case MM_KILL:		// the match as been killed
          // nothing to do because the program is always running for FRC bots
	  break;

     default:
	  // we don't need MM_ENDGAME_END at this point
	  break;
     }
     return(true);
}

Personality_3::Personality_3()
{

}

//
// Loop() - for the RIO pesonality, a message is sent out for each
//		loop through the Arduino code.  The message is simply the
//		appropriately formatted BT message with the translation of
//		the Gamepads and inclusion of the button.
//
void Personality_3::Loop(BT *bt, Gamepad *g1, Gamepad *g2)
{
     byte	msgbuff[64];	// max size of a BT message
     int	size;

     bool isRoboRIO = true;

     // if we're not connected to Bluetooth, then ingore the loop
     if (!bt->connected()) {
       enabled = false;
       if (isMatchActive()){
	 MatchReset();
       }
       return;
     }
       
     // only deal with matchmode when it is active

     if (isMatchActive()){
	     MatchLoopProcess((void *)bt);	// mode is set in the match callback above
     } else {
	     mode = myEEPROM.getMode();		// mode is set by the EEPROM setting
     }

       // first create a packet using the RIO structure
       size = RIO.createPacket(msgbuff,enabled,g1,g2,mode,isRoboRIO);
       // then send it over BT, again, operating on the message buffer
       (void)bt->btWrite(msgbuff,size);
}

void Personality_3::Kill(BT *bt)
{  
  // it is the Kill() that will turn matchmode active, so process all kills when enabled
  if(isMatchEnabled()) {
    MatchKillProcess((void *)bt);
  } else {
    enabled = false;
  }
}

void Personality_3::ChangeInput(BT *bt, int device, Gamepad *old, Gamepad *gnu)
{
     // nothing happepns here for this personality
}

void Personality_3::ChangeButton(BT *bt, bool button)
{ 
  if (button){
    if (isMatchActive()){ // normal operation   
      MatchButtonProcess((void *)bt);
    } else {
      enabled = !enabled;
      (enabled && bt->connected())?beeper.beep():beeper.boop();
    }
  }
}
