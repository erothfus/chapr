//
// cRIO.cpp  (see cRIO.h too)
//

#include <arduino.h>
#include "VDIP.h"
#include "gamepad.h"
#include "cRIO.h"
#include "ChapREEPROM.h"

// cRIO::CMDCompose() - given a command, and a data set, composes the buffer
//                      to be sent to the cRIO according to the general
//                      structure in the cRIO.h file. The msgbuff will 
//                      be modified to contain the whole message for
//                      sending.
int cRIO::CMDCompose(byte *msgbuff, int cmd, int size)
{
  byte cs = checksum(msgbuff, size);
  extern ChapREEPROM myEEPROM;

  memmove(msgbuff+17,msgbuff,size);	// shift over by the 17 needed to package the message

  msgbuff[0] = 0xff;
  msgbuff[1] = 0xff;
  msgbuff[2] = (byte) cmd;
  msgbuff[3] = myEEPROM.getDigitalInputs();
  msgbuff[4] = 0;
  msgbuff[5] = myEEPROM.getAnalogInput(0); // TODO
  msgbuff[6] = 0;
  msgbuff[7] = 0;
  msgbuff[8] = myEEPROM.getAnalogInput(1);
  msgbuff[9] = 0;
  msgbuff[10] = 0;
  msgbuff[11] = myEEPROM.getAnalogInput(2);
  msgbuff[12] = 0;
  msgbuff[13] = 0;
  msgbuff[14] = myEEPROM.getAnalogInput(3);
  msgbuff[15] = 0;
  msgbuff[16] = 0;
  msgbuff[size+17] = cs;

  return(size+18);			// total size of the message going over BT
}

byte cRIO::checksum(byte *msgbuff, int size)
{
  byte cs = 0;

  for (int i = 0; i < size; i++, msgbuff++){
    cs += *msgbuff;
  }
  
  cs &= 0x7f; // sets the first bit to zero so that the
              // checksum is never FF

  return(cs);
}

int cRIO::translateGamepad(byte *msgbuf, Gamepad *g1) 
{
  msgbuf[0] = g1->x1;				// joystick 1 (left) X axis 
  msgbuf[1] = g1->tophat;                       // joystick 1 tophat (0, 1 to 8)
  msgbuf[2] = g1->y1;				// joystick 1 (left) Y axis 
  msgbuf[3] = g1->buttons&0x3f;                 // joystick 1 B0 to B5
  msgbuf[4] = g1->x2;				// joystick 1 (right) X axis 
  msgbuf[5] = (g1->buttons>>6)&0x3f;            // joystick 1 B6 to B11
  msgbuf[6] = g1->y2;				// joystick 1 (right) Y axis 
  msgbuf[7] = 0;                                // a separator between the two joysticks

  return(8);
}

int cRIO::translate(byte *msgbuf, Gamepad *g1, Gamepad *g2)
{
  int size = 0;

  size += translateGamepad(msgbuf, g1);
  size += translateGamepad(msgbuf+size, g2);

  return (size);
}
    
