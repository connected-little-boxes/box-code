
//#define DIAGNOSTICS_ACTIVE
//#define STORE_RECEIVED_UINT8_T_DEBUG

// Stored program management

#define STATEMENT_CONFIRMATION 1
#define LINE_NUMBERS 2
#define ECHO_DOWNLOADS 4
#define DUMP_DOWNLOADS 8

enum ProgramState
{
	PROGRAM_STOPPED,
	PROGRAM_PAUSED,
	PROGRAM_ACTIVE,
	PROGRAM_AWAITING_MOVE_COMPLETION,
	PROGRAM_AWAITING_DELAY_COMPLETION,
	SYSTEM_CONFIGURATION_CONNECTION // will never enter this state
};

enum DeviceState
{
	EXECUTE_IMMEDIATELY,
	STORE_PROGRAM
};

#define COMMAND_BUFFER_SIZE 60

// Set command terminator to CR

#define STATEMENT_TERMINATOR 0x0D

// Set program terminator to string end
// This is the EOT character
#define PROGRAM_TERMINATOR 0x00

#ifdef COMMAND_DEBUG
#define READ_INTEGER_DEBUG
#endif

#include "HullOSVariables.h"

///////////////////////////////////////////////////////////
/// Serial comms
///////////////////////////////////////////////////////////
int CharsAvailable();
uint8_t GetRawCh();

void dumpProgramFromEEPROM(int EEPromStart);
void startProgramExecution(int programPosition);
// RH - remote halt
void haltProgramExecution();

// RP - pause program
void pauseProgramExecution();

// RR - resume running program
void resumeProgramExecution();

enum lineStorageState
{
	LINE_START,
	GOT_R,
	STORING,
	SKIPPING
};

void resetLineStorageState();
void storeProgramUint8_t(uint8_t b);
void clearStoredProgram();
void startDownloadingCode(int downloadPosition);
void endProgramReceive();
void storeReceivedUint8_t(uint8_t b);
void resetCommand();

bool readColour(uint8_t *r, uint8_t *g, uint8_t *b);

// Command PCrrr,ggg,bbb - set a coloured candle with the red, green
// and blue components as given
// Return OK
void remoteColouredCandle();

// Command PNname - set a coloured candle with the name as given
// Return OK
void remoteSetColorByName();

void remoteFadeToColor();

// PFddd - set flicker speed to value given
void remoteSetFlickerSpeed();

// PIppp,rrr,ggg,bbb
// Set individual pixel colour
void remoteSetIndividualPixel();
void remoteSetPixelsOff();
void remoteSetRandomColors();
void remotePixelControl();

// Command CDddd - delay time
// Command CD    - previous delay
// Return OK
void remoteDelay();

// Command CLxxxx - program label
// Ignored at execution, specifies the destination of a branch
// Return OK
void declareLabel()

	int findNextStatement(int programPosition);

// Find a label in the program
// Returns the offset into the program where the label is declared
// The first parameter is the first character of the label
// (i.e. the character after the instruction code that specifies the destination)
// This might not always be the same command (it might be a branch or a subroutine call)
// The second parameter is the start position of the search in the program.
// This is always the start of a statement, and usually the start of the program, to allow
// branches up the code.
int findLabelInProgram(char *label, int programPosition);

// Command CJxxxx - jump to label
// Jumps to the specified label
// Return CJOK if the label is found, error if not.
void jumpToLabel();

// Command CCxxxx - jump to label on a coin toss
// Jumps to the specified label
// Return CCOK if the label is found, error if not.
void jumpToLabelCoinToss();

void compareAndJump(bool jumpIfTrue);

void programControl();

// RM - start remote download
void remoteDownload();

void haltProgramExecutionCommand();
void clearProgramStoreCommand();
void remoteManagement();

// IV - information display version
void displayVersion();

void printStatus();

// IMddd - set the debugging diagnostics level
void setMessaging();

void printProgram();
void information();
void doClearVariables();
void variableManagement();
void doRemoteWriteText();
void doRemoteWriteLine();
void doRemotePrintValue();
void remoteWriteOutput();

void actOnCommand(char *commandDecodePos, char *comandDecodeLimit);

void processCommandUint8_t(uint8_t b);
void resetSerialBuffer();
void interpretSerialUint8_t(uint8_t b);
void processSerialUint8_t(uint8_t b);
void setupRemoteControl();

// Executes the statement in the EEPROM at the current program counter
// The statement is assembled into a buffer by interpretCommandUint8_t
bool exeuteProgramStatement();

void updateHullOS();
bool commandsNeedFullSpeed();

