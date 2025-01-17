//
// robotc.cpp  (see robotc.h too)
//

#include <Arduino.h>
#include "VDIP.h"
#include "gamepad.h"
#include "robotc.h"

//
// robotcGamepadTranslate() - given a structure with gamepad data, fill in the given buffer
//				with the binary	data that is compatible with robotc.  The
//				buffer is assumed to have at least 7 bytes.  Returns the
//				number of bytes used.  ONLY USED INTERNALLY

int robotcGamepadTranslate(byte *buffer, Gamepad *gp)
{
     // because the FCS only sends up to -127 and NOT -128, we cap the values at -127
     // note: LabView also caps these values, but RobotC does not
  
     buffer[0] = max(gp->x1, -127);				// joystick 1 (left) X axis 
     buffer[1] = max(gp->y1, -127);				// joystick 1 (left) Y axis 
     buffer[2] = max(gp->x2, -127);				// joystick 2 (right) X axis 
     buffer[3] = max(gp->y2, -127);				// joystick 2 (right) Y axis 

     buffer[4] = (byte) (gp->buttons & 0x00ff);		// buttons 1-8
     buffer[5] = (byte) ((gp->buttons & 0x0f00)>>8);	// buttons 9-12

     
     if (gp->tophat == 0) {				//   expects -1 if no tophat pressed
	  buffer[6] = 0xff;
     } else {
         buffer[6] = gp->tophat-1; //change from canonical gamepad form to what RobotC expects
     }
     return(7);
}

//
// robotcTranslate() - given 2 structures with gamepad data, along with the setting of
//			the WFS/pairing button, fill in the given buffer with the binary
//			data is compatible with robotc.  The buffer is assumed to have
//			at least 18 bytes.  Returns the number of bytes consumed in the
//			output buffer (18).

int robotcTranslate(byte *buffer, bool button, Gamepad *g1, Gamepad *g2, int mode)
{
     buffer[0] = 0;		// this isn't actually used in the robotC Joystick.c code
     buffer[1] = mode;		// this is the UserMode
     buffer[2] = (button)?0:1;	// false to turn OFF wait for start (the StopPgm)

     robotcGamepadTranslate(&buffer[3],g1);
     robotcGamepadTranslate(&buffer[10],g2);

     buffer[17] = 0;		// NULL terminate (everyone needs this anyway)

     return(18);
}
