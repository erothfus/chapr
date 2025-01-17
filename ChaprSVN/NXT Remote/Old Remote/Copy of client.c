#pragma config(Sensor, S1,     ,               sensorTouch)
#pragma config(Sensor, S2,     ,               sensorTouch)
#pragma config(Sensor, S3,     ,               sensorTouch)
#pragma config(Sensor, S4,     ,               sensorTouch)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "remoteincl.c"

// getmessage() - get a message from bluetooth - true if got one, false otherwise

int getmessage(ubyte &left, ubyte &right)
{
  int  msgsz;
  ubyte buffer[2];

    msgsz = cCmdMessageGetSize(mailbox1);
    if (msgsz <= 0)
    {
      return(false);
    }

    cCmdMessageRead(buffer,2,mailbox1);

    left = buffer[0];
    right = buffer[1];

    return(true);
}

task main()
{
  ubyte left, right;

  while (true) {
    if (getmessage(left,right)) {
      remoteDisplay(right,75,53,0x1f);
      remoteDisplay(left,0,53,0x2f);
    }
    wait1Msec(1);
  }
}
