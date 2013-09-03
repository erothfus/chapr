//
// nxt.h
//
//   This file (and the associated nxt.cpp) supports communicating with the NXT as a
//   ChapR target device.
//   
//   The LEGO NXT device uses a communication protocol that provides control and data
//   query over both USB and Bluetooth (BT).  In fact, it is the same protocol with the
//   exception that BT comm requires an extra length field.
//
//   The protocol is broken up into two different sections.  Each has a separate
//   document dedicated for it, that is available from LEGO.
//
//	1 - System Commands
//	2 - Direct Commands
//
//   This file provides defines for most of the commands available.
//
//   When specifying a command, the first bytes specifies which type of command and
//   determines if a reply is requested:
//
//	0x00 - Direct Command, reply required
//	0x01 - System Command, replay required
//	0x02 - Replay command
//
//	0x80 - Direct Command, no reply required
//	0x81 - System Command, no reply required
// 
//   A command can have up to 64 bytes in it.  Though this must take into account the
//   lead-in command bytes.  Also, BT has further restrictions.
//
//   Command format - here is the format implied by all of the above:
//
//    
//   ,-----,-----,------,-----,------,-----
//   | LSB | MSB | type | cmd | data |...
//   '-----'-----'------'-----'------'-----
//   |           |  0      1     2     ...
//   | BT length |
//   | not used  |
//   | for USB   |
//   |  0     1     2      3     4     ....
//

//
// COMMAND TYPE (first byte)
//
#define NXT_DIR_CMD	'\x00'		// Direct command - reply required
#define NXT_SYS_CMD	'\x01'		// System command - reply required
#define NXT_REPLY_CMD	'\x02'		// Reply command
#define NXT_DIR_CMD_NR	'\x80'		// Direct command - NO REPLY
#define NXT_SYS_CMD_NR	'\x81'		// System command - NO REPLY

//
// SYSTEM COMMANDS (second byte) - preceeded by 0x01 or 0x81
//
#define NXT_SYS_OPEN_R	'\x80'		// Opens a file for reading from the NXT
#define NXT_SYS_OPEN_W	'\x81'		// Opens a file for writing to the NXT
#define NXT_SYS_READ	'\x82'		// Reads data from an open file
#define NXT_SYS_WRITE	'\x83'		// Writes data to an open file
#define NXT_SYS_CLOSE	'\x84'		// Closes a file previously opened for reading/writing
#define NXT_SYS_DELETE	'\x85'		// Delete the named file
#define NXT_SYS_FIND	'\x86'		// Finds the first file on the NXT matching a pattern
#define NXT_SYS_NEXT	'\x87'		// Finds the next file matching the previous pattern
#define NXT_SYS_VERSION	'\x88'		// Returns version of the NXT firmware running
#define NXT_SYS_OPENL_W	'\x89'		// Open Write Linear
#define NXT_SYS_OPENL_R	'\x8A'		// Open Read Linear
#define NXT_SYS_OPEND_W	'\x8B'		// Open Write Data
#define NXT_SYS_APPD_W	'\x8C'		// Open Append Data
#define NXT_SYS_BOOT	'\x97'		// Reboot the NXT
#define NXT_SYS_NAME	'\x98'		// Set the NXT name
#define NXT_SYS_INFO	'\x9B'		// Get system information
#define NXT_SYS_CLEAR	'\xA0'		// Delete user flash
#define NXT_SYS_POLL_L	'\xA1'		// Poll command length
#define NXT_SYS_POLL_C	'\xA2'		// Poll command
#define NXT_SYS_BTRST	'\xA4'		// Factory reset on the Bluetooth

//
// SYSTEM COMMAND CONSTANTS
//
#define NXT_REBOOT_MAGICSTRING  "Let's dance: SAMBA"  //string for booting NXT

//
// DIRECT COMMANDS (second byte) - preceeded by 0x00 or 0x80
//
#define NXT_DIR_START	'\x00'		// Start the named program
#define NXT_DIR_STOP	'\x01'		// Stop the named program
#define NXT_DIR_SOUND	'\x02'		// Play the named sound file
#define NXT_DIR_TONE	'\x03'		// Play the given tone
#define NXT_DIR_SET_OUT	'\x04'		// Set the given output state for motors (OUTPUT STATE)
#define NXT_DIR_SET_IN	'\x05'		// Set the given state for inputs (INPUT MODE)
#define NXT_DIR_GET_OUT	'\x06'		// Get the output states
#define NXT_DIR_GET_IN	'\x07'		// Get the input values
#define NXT_DIR_SCALE_R	'\x08'		// Reset the scaled values on input
#define NXT_DIR_SEND	'\x09'		// Send a direct message
#define NXT_DIR_MOTOR_R	'\x0A'		// Reset motor position
#define NXT_DIR_BATTERY	'\x0B'		// Get battery level
#define NXT_DIR_SOUND_X	'\x0C'		// Stop sound playback
#define NXT_DIR_KEEP	'\x0D'		// Keep Alive
#define NXT_DIR_LS_GET	'\x0E'		// Get LS status
#define NXT_DIR_LS_W	'\x0F'		// LS write
#define NXT_DIR_LS_R	'\x10'		// LS read
#define NXT_DIR_CURRENT	'\x11'		// Get current program name
#define NXT_DIR_RECV	'\x13'		// receive a direct message

// these need to be replaced in the code with the defines above

#define NXT_GET_DEV_INFO NXT_SYS_INFO
#define NXT_REBOOT       NXT_SYS_BOOTH




extern int nxtMsgCompose(byte *output, 		// the output buffer to scribble things to - min 22 bytes
			 byte UserMode,		// the usermode value
			 byte StopPgm,		// the wait-for-start value
			 byte *USBJoystick1,	// buffers for the two joysticks
			 byte *USBJoystick2);

extern bool nxtQueryDevice(VDIP *, int, char **, char **, long *);

extern int nxtBTMailboxMsgCompose(int,byte *,int);
extern int nxtBTKillCommand(byte *);
