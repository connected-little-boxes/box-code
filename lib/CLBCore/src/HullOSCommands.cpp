#include <EEPROM.h>
#include <Arduino.h>
#include "string.h"
#include "errors.h"
#include "settings.h"
#include "debug.h"
#include "ArduinoJson-v5.13.2.h"
#include "utils.h"
#include "sensors.h"
#include "processes.h"
#include "controller.h"
#include "registration.h"
#include "HullOSCommands.h"
#include "HullOSVariables.h"
#include "HullOSScript.h"
#include "HullOS.h"

ProgramState programState = PROGRAM_STOPPED;
DeviceState deviceState = EXECUTE_IMMEDIATELY;

uint8_t diagnosticsOutputLevel = 0;

unsigned long delayEndTime;

char programCommand[COMMAND_BUFFER_SIZE];
char *commandPos;
char *commandLimit;
char *bufferLimit;
char *decodePos;
char *decodeLimit;

char remoteCommand[COMMAND_BUFFER_SIZE];
char *remotePos;
char *remoteLimit;

int CharsAvailable()
{
    return Serial.available();
}

uint8_t GetRawCh()
{
    int ch;
    do
    {
        ch = Serial.read();
    } while (ch < 0);

    return (uint8_t)ch;
}

// Current position in the EEPROM of the execution
int programCounter;

// Start position of the code as stored in the EEPROM
int programBase;

// Write position when downloading and storing program code
int programWriteBase;

// Write position for any incoming program code
int bufferWritePosition;

// Checksum for the download
uint8_t downloadChecksum;

uint8_t readHullOSProgramByte(int address)
{
	return hullosSettings.hullosCode[address];
}

bool storeByteIntoEEPROM(char byte, int pos)
{
	return true;
}

// Stores a program into the eeprom at the stated location
// The program is a string of text which is zero terminated
// The EEPromStart value is the offset in the EEProm into which the program is to be written
// The function returns true if the program was loaded, false if not

bool storeProgramIntoEEPROM(char * programStart, int EEPromStart)
{
//   while (*programStart)
//   {
//     if (!storeByteIntoEEPROM(*programStart, EEPromStart))
//       return false;
//     programStart++;
//     EEPromStart++;
//   }

//   // put the terminator on the end of the program
//   storeByteIntoEEPROM(*programStart, EEPromStart);
  return true;
}

void setProgramStored()
{
//   storeByteIntoEEPROM(PROGRAM_STORED_VALUE1, PROGRAM_STATUS_BYTE_OFFSET);
//   storeByteIntoEEPROM(PROGRAM_STORED_VALUE2, PROGRAM_STATUS_BYTE_OFFSET + 1);
}

void clearProgramStoredFlag()
{
//   storeByteIntoEEPROM(0, PROGRAM_STATUS_BYTE_OFFSET);
//   storeByteIntoEEPROM(0, PROGRAM_STATUS_BYTE_OFFSET + 1);
}

bool isProgramStored()
{
//   if ((EEPROM.read(PROGRAM_STATUS_BYTE_OFFSET) == PROGRAM_STORED_VALUE1) &
//     (EEPROM.read(PROGRAM_STATUS_BYTE_OFFSET + 1) == PROGRAM_STORED_VALUE2))
//     return true;
//   else
//     return false;
return true;
}







void dumpProgramFromEEPROM(int EEPromStart)
{
    int EEPromPos = EEPromStart;

    Serial.println(F("Program: "));

    char byte;
    while (true)
    {
        byte = readHullOSProgramByte(EEPromPos++);

        if (byte == STATEMENT_TERMINATOR)
            Serial.println();
        else
            Serial.print(byte);

        if (byte == PROGRAM_TERMINATOR)
        {
            Serial.print(F("Program size: "));
            Serial.println(EEPromPos - EEPromStart);
            break;
        }

        if (EEPromPos >= EEPROM_SIZE)
        {
            Serial.println(F("Eeprom end"));
            break;
        }
    }
}

void startProgramExecution(int programPosition)
{
    if (isProgramStored())
    {

#ifdef PROGRAM_DEBUG
        Serial.print(F(".Starting program execution at: "));
        Serial.println(programPosition);
#endif
        clearVariables();
        programCounter = programPosition;
        programBase = programPosition;
        programState = PROGRAM_ACTIVE;
    }
}

// RH - remote halt
void haltProgramExecution()
{
#ifdef PROGRAM_DEBUG
    Serial.print(F(".Ending program execution at: "));
    Serial.println(programCounter);
#endif

    programState = PROGRAM_STOPPED;
}

// RP - pause program
void pauseProgramExecution()
{
#ifdef PROGRAM_DEBUG
    Serial.print(".Pausing program execution at: ");
    Serial.println(programCounter);
#endif

    programState = PROGRAM_PAUSED;

#ifdef DIAGNOSTICS_ACTIVE

    if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
    {
        Serial.print(F("RPOK"));
    }

#endif
}

// RR - resume running program
void resumeProgramExecution()
{
#ifdef PROGRAM_DEBUG
	Serial.print(".Resuming program execution at: ");
	Serial.println(programCounter);
#endif

	if (programState == PROGRAM_PAUSED)
	{
		// Can resume the program
		programState = PROGRAM_ACTIVE;

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.print(F("RROK"));
		}
#endif


	}
	else
	{
#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.print(F("RRFail:"));
			Serial.println(programState);
		}
#endif
	}
}

lineStorageState lineStoreState;

void resetLineStorageState()
{
	lineStoreState = LINE_START;
}


void storeProgramByte(uint8_t b)
{
	storeByteIntoEEPROM(b, programWriteBase++);
}

void clearStoredProgram()
{
	clearProgramStoredFlag();
	storeByteIntoEEPROM(PROGRAM_TERMINATOR, STORED_PROGRAM_OFFSET);
}

// Called to start the download of program code
// each byte that arrives down the serial port is now stored in program memory
//
void startDownloadingCode(int downloadPosition)
{
#ifdef PROGRAM_DEBUG
	Serial.println(".Starting code download");
#endif

	// Stop the current program
	haltProgramExecution();

	// clear the existing program so that
	// partially stored programs never get executed on power up

	clearStoredProgram();

	deviceState = STORE_PROGRAM;

	programWriteBase = downloadPosition;

	resetLineStorageState();

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print(F("RMOK"));
	}

#endif
}

void endProgramReceive()
{
	// enable immediate command receipt

	deviceState = EXECUTE_IMMEDIATELY;
}

// Called when a byte is received from the host when in program storage mode
// Adds it to the stored program, updates the stored position and the counter
// If the byte is the terminator byte (zero) it changes to the "wait for checksum" state
// for the program checksum
void storeReceivedByte(uint8_t b)
{
	// ignore odd characters - except for CR

	if ((b < 32) || (b>128))
	{
		if (b != STATEMENT_TERMINATOR)
			return;
	}

	switch (lineStoreState)
	{

	case LINE_START:
		// at the start of a line - look for an R command

		if ((b == 'r') || (b == 'R'))
		{
			lineStoreState = GOT_R;
		}
		else
		{
			lineStoreState = STORING;
		}
		break;

	case GOT_R:
		// Last character was an R - is this an X or an A?

		switch (b)
		{
		case 'x':
		case 'X':
			endProgramReceive();

			// put the terminator on the end

			storeProgramByte(PROGRAM_TERMINATOR);

			setProgramStored();

#ifdef DIAGNOSTICS_ACTIVE

			if (diagnosticsOutputLevel & DUMP_DOWNLOADS)
			{
				dumpProgramFromEEPROM(STORED_PROGRAM_OFFSET);
			}

#endif

			startProgramExecution(STORED_PROGRAM_OFFSET);

			break;

		case 'A':
		case 'a':
			Serial.println("RA");
			endProgramReceive();

			clearStoredProgram();

			break;

		default:

			// Not an X jor A - but we never store R commands
			// skip to the next line
			lineStoreState = SKIPPING;
		}

		break;

	case SKIPPING:
		// we are skipping an R command - look for a statement terminator
		if (b == STATEMENT_TERMINATOR)
		{
			// Got a terminator, look for the command character
			lineStoreState = LINE_START;
		}
		break;

	case STORING:
		// break out- storing takes place next
		break;
	}

	if (lineStoreState == STORING)
	{
		// get here if we are storing or just got a line start

		// if we get here we store the byte
		storeProgramByte(b);

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & ECHO_DOWNLOADS)
		{
			Serial.print((char)b);
		}

#endif

		if (b == STATEMENT_TERMINATOR)
		{
			// Got a terminator, look for the command character

#ifdef DIAGNOSTICS_ACTIVE

			if (diagnosticsOutputLevel & ECHO_DOWNLOADS)
			{
				Serial.println();
			}

#endif
			lineStoreState = LINE_START;
		}
	}
}

void resetCommand()
{
#ifdef COMMAND_DEBUG
	Serial.println(".**resetCommand");
#endif
	commandPos = programCommand;
	bufferLimit = commandPos + COMMAND_BUFFER_SIZE;
}


// Command CDddd - delay time
// Command CD    - previous delay
// Return OK

#ifdef COMMAND_DEBUG
#define COMMAND_DELAY_DEBUG
#endif

void remoteDelay()
{
	int delayValueInTenthsIOfASecond;

#ifdef COMMAND_DELAY_DEBUG
	Serial.println(".**remoteDelay");
#endif

	if (*decodePos == STATEMENT_TERMINATOR)
	{

#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("CDFail: no delay"));
		}
#endif

		return;
	}

	if (!getValue(&delayValueInTenthsIOfASecond))
	{
		return;
	}

#ifdef COMMAND_DELAY_DEBUG
	Serial.print(".  Delaying: ");
	Serial.println(delayValueInTenthsIOfASecond);
#endif

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print(F("CDOK"));
	}
#endif

	delayEndTime = millis() + delayValueInTenthsIOfASecond * 100;

	programState = PROGRAM_AWAITING_DELAY_COMPLETION;
}

// Command CLxxxx - program label
// Ignored at execution, specifies the destination of a branch
// Return OK

void declareLabel()
{
#ifdef COMMAND_DELAY_DEBUG
	Serial.println(".**declareLabel");
#endif

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println("CLOK");
	}
#endif
}

int findNextStatement(int programPosition)
{

	while (true)
	{
		char ch = readHullOSProgramByte(programPosition);

		if ((ch == PROGRAM_TERMINATOR) || (programPosition == EEPROM_SIZE))
			return -1;

		if (ch == STATEMENT_TERMINATOR)
		{
			programPosition++;
			if (programPosition == EEPROM_SIZE)
				return -1;
			else
				return programPosition;
		}
		programPosition++;
	}
}


// Find a label in the program
// Returns the offset into the program where the label is declared
// The first parameter is the first character of the label
// (i.e. the character after the instruction code that specifies the destination)
// This might not always be the same command (it might be a branch or a subroutine call)
// The second parameter is the start position of the search in the program.
// This is always the start of a statement, and usually the start of the program, to allow
// branches up the code.

//#define FIND_LABEL_IN_PROGRAM_DEBUG

int findLabelInProgram(char *label, int programPosition)
{
	// Assume we are starting at the beginning of the program

	while (true)
	{
		// Spin down the statements

		int statementStart = programPosition;

		char programByte = readHullOSProgramByte(programPosition++);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
		Serial.print("Statement at: ");
		Serial.print(statementStart);
		Serial.print(" starting: ");
		Serial.println(programByte);
#endif
		if ((programByte != 'C') && (programByte != 'c'))
		{

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.println("Not a statement that starts with C");
#endif

			programPosition = findNextStatement(programPosition);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.print("Spin to statement at: ");
			Serial.println(programPosition);
#endif

			// Check to see if we have reached the end of the program in EEPROM
			if (programPosition == -1)
			{
				// Give up if the end of the code has been reached
				return -1;
			}
			else
			{
				// Check this statement
				continue;
			}
		}

		// If we get here we have found a C

		programByte = readHullOSProgramByte(programPosition++);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG

		Serial.print("Second statement character: ");
		Serial.println(programByte);

#endif

		// if we get here we have a control command - see if the command is a label
		if ((programByte != 'L') && (programByte != 'l'))
		{

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.println("Not a loop command");
#endif

			programPosition = findNextStatement(programPosition);
			if (programPosition == -1)
			{
				return -1;
			}
			else
			{
				continue;
			}
		}

		//if we get here we have a CL command

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
		Serial.println("Got a CL command");
#endif

		// Set start position for label comparison
		char *labelTest = label;

		// Now spin down the label looking for a match

		while ((*labelTest != STATEMENT_TERMINATOR) && (programPosition < EEPROM_SIZE))
		{
			programByte = readHullOSProgramByte(programPosition);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.print("Destination byte: ");
			Serial.print(*labelTest);
			Serial.print(" Program byte: ");
			Serial.println(programByte);
#endif

			if (*labelTest == programByte)
			{

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
				Serial.println("Got a match");
#endif
				// Move on to the next byte
				labelTest++;
				programPosition++;
			}
			else
			{
#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
				Serial.println("Fail");
#endif
				break;
			}
		}

		// get here when we reach the end of the statement or we have a mismatch

		// Get the byte at the end of the destination statement

		programByte = readHullOSProgramByte(programPosition);

		if (*labelTest == programByte)
		{
			// If the end of the label matches the end of the statement code we have a match
			// Note that this means that if the last line of the program is a label we can only
			// find this line if it has a statement terminator on the end.
			// Which is fine by me.

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.println("label match");
#endif
			return statementStart;
		}
		else
		{
			programPosition = findNextStatement(programPosition);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.print("Spin to statement at: ");
			Serial.println(programPosition);
#endif

			// Check to see if we have reached the end of the program in EEPROM
			if (programPosition == -1)
			{
				// Give up if the end of the code has been reached
				return -1;
			}
			else
			{
				// Check this statement
				continue;
			}
		}
	}
}

// Command CJxxxx - jump to label
// Jumps to the specified label
// Return CJOK if the label is found, error if not.

void jumpToLabel()
{
#ifdef JUMP_TO_LABEL_DEBUG
	Serial.println(".**jump to label");
#endif

	int labelStatementPos = findLabelInProgram(decodePos, programBase);

#ifdef JUMP_TO_LABEL_DEBUG
	Serial.print("Label statement pos: ");
	Serial.println(labelStatementPos);
#endif

	if (labelStatementPos >= 0)
	{
		// the label has been found - jump to it
		programCounter = labelStatementPos;

#ifdef JUMP_TO_LABEL_DEBUG
		Serial.print("New Program Counter: ");
		Serial.println(programCounter);
#endif

#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println("CJOK");
		}
#endif
	}
	else
	{
#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println("CJFAIL: no dest");
		}
#endif
	}
}

//#define JUMP_TO_LABEL_COIN_DEBUG

// Command CCxxxx - jump to label on a coin toss
// Jumps to the specified label
// Return CCOK if the label is found, error if not.

void jumpToLabelCoinToss()
{
#ifdef JUMP_TO_LABEL_COIN_DEBUG
	Serial.println(F(".**jump to label coin toss"));
	send

#endif

	int labelStatementPos = findLabelInProgram(decodePos, programBase);

#ifdef JUMP_TO_LABEL_COIN_DEBUG
	Serial.print("  Label statement pos: ");
	Serial.println(labelStatementPos);
#endif

	if (labelStatementPos >= 0)
	{
		// the label has been found - jump to it

		if (random(0, 2) == 0)
		{
			programCounter = labelStatementPos;
#ifdef DIAGNOSTICS_ACTIVE
			if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
			{
				Serial.print(F("CCjump"));
			}
#endif
		}
		else
		{
#ifdef DIAGNOSTICS_ACTIVE
			if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
			{
				Serial.print(F("CCcontinue"));
			}
#endif
		}

#ifdef JUMP_TO_LABEL_COIN_DEBUG
		Serial.print(F("New Program Counter: "));
		Serial.println(programCounter);
#endif
	}
	else
	{
#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("CCFail: no dest"));
		}
#endif
	}
}


//#define COMPARE_CONDITION_DEBUG

void compareAndJump(bool jumpIfTrue)
{

#ifdef COMPARE_CONDITION_DEBUG
	Serial.println(F(".**test condition and jump to label"));
#endif

	bool result;

	if (!testCondition(&result))
	{
		return;
	}

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		if (jumpIfTrue)
			Serial.print(F("CC"));
		else
			Serial.print(F("CN"));
	}
#endif

	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: mising dest"));
		}
#endif
		return;
	}

	decodePos++;

	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: mising dest"));
		}
#endif
		return;
	}

	int labelStatementPos = findLabelInProgram(decodePos, programBase);

#ifdef COMPARE_CONDITION_DEBUG
	Serial.print("Label statement pos: ");
	Serial.println(labelStatementPos);
#endif

	if (labelStatementPos < 0)
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: label not found"));
#endif
		return;
	}

	if (result == jumpIfTrue)
	{
#ifdef COMPARE_CONDITION_DEBUG
		Serial.println(F("Condition true - taking jump"));
#endif
		// the label has been found - jump to it
		programCounter = labelStatementPos;

#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("jump"));
		}
#endif
	}
	else
	{
#ifdef COMPARE_CONDITION_DEBUG
		Serial.println("condition failed - continuing");
#endif
#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("continue"));
		}
#endif
	}

	// otherwise do nothing
}

void programControl()
{
	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: missing program control command character"));
#endif
		return;
	}

#ifdef COMMAND_DEBUG
	Serial.println(F(".**remoteProgramControl: "));
#endif

	char commandCh = *decodePos;

#ifdef COMMAND_DEBUG
	Serial.print(F(".   Program command : "));
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case 'D':
	case 'd':
		remoteDelay();
		break;
	case 'L':
	case 'l':
		declareLabel();
		break;
	case 'J':
	case 'j':
		jumpToLabel();
		break;
	case 'C':
	case 'c':
		jumpToLabelCoinToss();
		break;
	case 'T':
	case 't':
		compareAndJump(true);
		break;
	case 'F':
	case 'f':
		compareAndJump(false);
		break;
	}
}

//#define REMOTE_DOWNLOAD_DEBUG

// RM - start remote download

void remoteDownload()
{

#ifdef REMOTE_DOWNLOAD_DEBUG
	Serial.println(F(".**remote download"));
#endif

	if (deviceState != EXECUTE_IMMEDIATELY)
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("RMFAIL: not accepting commands"));
#endif
		return;
	}

	startDownloadingCode(STORED_PROGRAM_OFFSET);
}

void startProgramCommand()
{
	startProgramExecution(STORED_PROGRAM_OFFSET);

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("RSOK"));
	}
#endif
}

void haltProgramExecutionCommand()
{
	haltProgramExecution();

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("RHOK"));
	}
#endif
}

void clearProgramStoreCommand()
{
	haltProgramExecution();

	clearStoredProgram();

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("RCOK"));
	}
#endif
}

void remoteManagement()
{
	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: missing remote control command character"));
#endif
		return;
	}

#ifdef COMMAND_DEBUG
	Serial.println(F(".**remoteProgramDownload: "));
#endif

	char commandCh = *decodePos;

#ifdef COMMAND_DEBUG
	Serial.print(F(".   Download command : "));
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case 'M':
	case 'm':
		remoteDownload();
		break;
	case 'S':
	case 's':
		startProgramCommand();
		break;
	case 'H':
	case 'h':
		haltProgramExecutionCommand();
		break;
	case 'P':
	case 'p':
		pauseProgramExecution();
		break;
	case 'R':
	case 'r':
		resumeProgramExecution();
		break;
	case 'C':
	case 'c':
		clearProgramStoreCommand();
		break;
	}
}

// IV - information display version
void displayVersion()
{
#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("IVOK"));
	}
#endif

	Serial.println(Version);
}

void printStatus()
{
#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("ISOK"));
	}
#endif
	Serial.print(programState);
	Serial.println(diagnosticsOutputLevel);
}

// IMddd - set the debugging diagnostics level

//#define SET_MESSAGING_DEBUG

void setMessaging()
{

#ifdef SET_MESSAGING_DEBUG
	Serial.println(F(".**informationlevelset: "));
#endif
	int result;

	if (!getValue(&result))
	{
		return;
	}

	uint8_t no = (uint8_t)result;

#ifdef SET_MESSAGING_DEBUG
	Serial.print(F(".  Setting: "));
	Serial.println(no);
#endif

	diagnosticsOutputLevel = no;

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("IMOK"));
	}
#endif
}

void printProgram()
{
	dumpProgramFromEEPROM(STORED_PROGRAM_OFFSET);

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("IPOK"));
	}
#endif
}

void information()
{
	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: missing information command character"));
#endif
		return;
	}

#ifdef COMMAND_DEBUG
	Serial.println(F(".**remoteProgramDownload: "));
#endif

	char commandCh = *decodePos;

#ifdef COMMAND_DEBUG
	Serial.print(F(".   Download command : "));
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case 'V':
	case 'v':
		displayVersion();
		break;
	case 'S':
	case 's':
		printStatus();
		break;
	case 'M':
	case 'm':
		setMessaging();
		break;
	case 'P':
	case 'p':
		printProgram();
		break;
	}
}

void doClearVariables()
{
	clearVariables();

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print(F("VCOK"));
	}
}

void variableManagement()
{
	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: missing variable command character"));
#endif
		return;
	}

#ifdef COMMAND_DEBUG
	Serial.println(F(".**variable management: "));
#endif

	char commandCh = *decodePos;

#ifdef COMMAND_DEBUG
	Serial.print(F(".   Download command : "));
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case 'C':
	case 'c':
		doClearVariables();
		break;

	case 'S':
	case 's':
		setVariable();
		break;

	case 'V':
	case 'v':
		viewVariable();
		break;
	}
}

void doRemoteWriteText()
{
	while ((*decodePos != STATEMENT_TERMINATOR) && (decodePos != decodeLimit))
	{
		Serial.print(*decodePos);
		decodePos++;
	}
}

void doRemoteWriteLine()
{
	Serial.println();
}

void doRemotePrintValue()
{
	int valueToPrint;

	if (getValue(&valueToPrint))
	{
		Serial.print(valueToPrint);
	}
}

void remoteWriteOutput()
{
	if ((*decodePos == STATEMENT_TERMINATOR) || (decodePos == decodeLimit))
	{
#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: missing write output command character"));
#endif
		return;
	}

#ifdef COMMAND_DEBUG
	Serial.println(F(".**write output: "));
#endif

	char commandCh = *decodePos;

#ifdef COMMAND_DEBUG
	Serial.print(F(".   Download command : "));
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case 'T':
	case 't':
		doRemoteWriteText();
		break;

	case 'L':
	case 'l':
		doRemoteWriteLine();
		break;

	case 'V':
	case 'v':
		doRemotePrintValue();
		break;
	}
}

void actOnCommand(char *commandDecodePos, char *comandDecodeLimit)
{
	decodePos = commandDecodePos;
	decodeLimit = comandDecodeLimit;

	*decodeLimit = 0;

#ifdef COMMAND_DEBUG
	Serial.print(F(".**processCommand:"));
	Serial.println((char *)decodePos);
#endif

	char commandCh = *decodePos;

#ifdef COMMAND_DEBUG
	Serial.print(F(".  Command code : "));
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case '#':
		// Ignore comments
		break;
	case 'I':
	case 'i':
		information();
		break;
	case 'C':
	case 'c':
		programControl();
		break;
	case 'R':
	case 'r':
		remoteManagement();
		break;
	case 'V':
	case 'v':
		variableManagement();
		break;
	case 'w':
	case 'W':
		remoteWriteOutput();
		break;
	default:
#ifdef COMMAND_DEBUG
		Serial.println(F(".  Invalid command : "));
#endif
#ifdef DIAGNOSTICS_ACTIVE
		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.print(F("Invalid Command: "));
			Serial.print(commandCh);
			Serial.print(F(" code: "));
			Serial.println((int)commandCh);
		}
#endif
		break;
	}
}

void processCommandByte(uint8_t b)
{
	if (commandPos == bufferLimit)
	{
#ifdef COMMAND_DEBUG
		Serial.println(F(".  Command buffer full - resetting"));
#endif
		resetCommand();
		return;
	}

	*commandPos = b;

	commandPos++;

	if (b == STATEMENT_TERMINATOR)
	{
#ifdef COMMAND_DEBUG
		Serial.println(F(".  Command end"));
#endif
		actOnCommand(programCommand, commandPos);
		resetCommand();
		return;
	}
}

void resetSerialBuffer()
{
	remotePos = remoteCommand;
	remoteLimit = remoteCommand + COMMAND_BUFFER_SIZE;
}

void interpretSerialByte(uint8_t b)
{
	if (remotePos == remoteLimit)
	{
		resetSerialBuffer();
		return;
	}

	*remotePos = b;
	remotePos++;

	if (b == STATEMENT_TERMINATOR)
	{
#ifdef COMMAND_DEBUG
		Serial.println(F(".  Command end"));
#endif
		actOnCommand(remoteCommand, remotePos);
		resetSerialBuffer();
		return;
	}
}

void processHullOSSerialByte(uint8_t b)
{
#ifdef COMMAND_DEBUG
	Serial.print(F(".**processSerialByte: "));
	Serial.println((char)b);
#endif

	switch (deviceState)
	{
	case EXECUTE_IMMEDIATELY:
		decodeScriptChar(b, interpretSerialByte);
		break;
	case STORE_PROGRAM:
		decodeScriptChar(b, storeReceivedByte);
		break;
	}
}

void setupRemoteControl()
{
#ifdef COMMAND_DEBUG
	Serial.println(F(".**setupRemoteControl"));
#endif
	resetCommand();
	resetSerialBuffer();
}

bool exeuteProgramStatement()
{
	char programByte;

#ifdef PROGRAM_DEBUG
	Serial.println(F(".Executing statement"));
#endif

#ifdef DIAGNOSTICS_ACTIVE
	if (diagnosticsOutputLevel & LINE_NUMBERS)
	{
		Serial.print(F("Offset: "));
		Serial.println((int)programCounter);
	}
#endif

	while (true)
	{
		programByte = readHullOSProgramByte(programCounter++);

		if (programCounter >= EEPROM_SIZE || programByte == PROGRAM_TERMINATOR)
		{
			haltProgramExecution();
			return false;
		}

#ifdef PROGRAM_DEBUG
		Serial.print(F(".    program byte: "));
		Serial.println(programByte);
#endif

		processCommandByte(programByte);

		if (programByte == STATEMENT_TERMINATOR)
			return true;
	}
}


