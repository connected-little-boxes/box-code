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


ProgramState programState = PROGRAM_STOPPED;
DeviceState deviceState = EXECUTE_IMMEDIATELY;

uint8_t diagnosticsOutputLevel = 0;

long delayEndTime;

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

void dumpProgramFromEEPROM(int EEPromStart)
{
    int EEPromPos = EEPromStart;

    Serial.println(F("Program: "));

    char uint8_t;
    while (true)
    {
        uint8_t = EEPROM.read(EEPromPos++);

        if (uint8_t == STATEMENT_TERMINATOR)
            Serial.println();
        else
            Serial.print(uint8_t);

        if (uint8_t == PROGRAM_TERMINATOR)
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
        setAllLightsOff();
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

    motorStop();

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


void storeProgramUint8_t(uint8_t b)
{
	storeUint8_tIntoEEPROM(b, programWriteBase++);
}

void clearStoredProgram()
{
	clearProgramStoredFlag();
	storeUint8_tIntoEEPROM(PROGRAM_TERMINATOR, STORED_PROGRAM_OFFSET);
}

// Called to start the download of program code
// each uint8_t that arrives down the serial port is now stored in program memory
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

	startBusyPixel(128, 128, 128);

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print(F("RMOK"));
	}

#endif
}

void endProgramReceive()
{
	stopBusyPixel();

	// enable immediate command receipt

	deviceState = EXECUTE_IMMEDIATELY;
}

// Called when a uint8_t is received from the host when in program storage mode
// Adds it to the stored program, updates the stored position and the counter
// If the uint8_t is the terminator uint8_t (zero) it changes to the "wait for checksum" state
// for the program checksum
void storeReceivedUint8_t(uint8_t b)
{
	// ignore odd characters - except for CR

	if (b < 32 | b>128)
	{
		if (b != STATEMENT_TERMINATOR)
			return;
	}

	switch (lineStoreState)
	{

	case LINE_START:
		// at the start of a line - look for an R command

		if (b == 'r' | b == 'R')
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

			storeProgramUint8_t(PROGRAM_TERMINATOR);

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

		// if we get here we store the uint8_t
		storeProgramUint8_t(b);

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
			// look busy
			updateBusyPixel();
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

#ifdef COMMAND_DEBUG
#define PIXEL_COLOUR_DEBUG
#endif

bool readColour(uint8_t *r, uint8_t *g, uint8_t*b)
{
	int result;

	if (!getValue(&result))
	{
		return false;
	}

	*r = (uint8_t)result;

#ifdef PIXEL_COLOUR_DEBUG
	Serial.print(".  Red: ");
	Serial.println(*r);
#endif

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: mising colour values in readColor"));
		}

#endif
		return false;
	}

	decodePos++;

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: mising colours after red in readColor"));
		}

#endif

		return false;
	}

	if (!getValue(&result))
	{
		return false;
	}

	*g = (uint8_t)result;

#ifdef PIXEL_COLOUR_DEBUG
	Serial.print(".  Green: ");
	Serial.println(*g);
#endif

	decodePos++;

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: mising colours after green in readColor"));
		}

#endif

		return false;
	}

	if (!getValue(&result))
	{
		return false;
	}

	*b = (uint8_t)result;

#ifdef PIXEL_COLOUR_DEBUG
	Serial.print(".  Blue: ");
	Serial.println(*b);
#endif

	return true;
}


// Command PCrrr,ggg,bbb - set a coloured candle with the red, green 
// and blue components as given
// Return OK

void remoteColouredCandle()
{
#ifdef PIXEL_COLOUR_DEBUG
	Serial.println(".**remoteColouredCandle: ");
#endif

	uint8_t r, g, b;


#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print("PC");
	}

#endif

	if (readColour(&r, &g, &b))
	{
		flickeringColouredLights(r, g, b, 0, 200);

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("OK"));
		}
#endif
	}
}

// Command PNname - set a coloured candle with the name as given
// Return OK

void remoteSetColorByName()
{
#ifdef PIXEL_COLOUR_DEBUG
	Serial.println(".**remoteColouredName: ");
#endif

	uint8_t r = 0, g = 0, b = 0;

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print("PN");
	}

#endif

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: mising colour in set colour by name"));
		}

#endif

		return;
	}

	char inputCh = toLowerCase(*decodePos);

	switch (inputCh)
	{
	case 'r':
		r = 255;
		break;
	case 'g':
		g = 255;
		break;
	case 'b':
		b = 255;
		break;
	case 'c':
		g = 255;
		b = 255;
		break;
	case 'm':
		r = 255;
		b = 255;
		break;
	case 'y':
		g = 255;
		r = 255;
		break;
	case 'w':
		r = 255;
		g = 255;
		b = 255;
		break;
	case 'k':
		break;
	default:

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("FAIL: invalid colour in set colour by name"));
		}
#endif
		return;
	}

	flickeringColouredLights(r, g, b, 0, 200);

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("OK"));
	}

#endif
}

//#define REMOTE_PIXEL_COLOR_FADE_DEBUG

void remoteFadeToColor()
{
#ifdef PIXEL_COLOUR_DEBUG
	Serial.println(".**remoteFadeToColour: ");
#endif

	int result;

	if (!getValue(&result))
	{
		return;
	}

	uint8_t no = (uint8_t)result;

	if (no < 1)
		no = 1;
	if (no > 20)
		no = 20;

	no = 21 - no;

#ifdef REMOTE_PIXEL_COLOR_FADE_DEBUG
	Serial.print(".  Setting: ");
	Serial.println(no);
#endif

	decodePos++;

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print("PX");
	}

#endif

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("Fail: mising colours after speed"));
		}

#endif

		return;
	}

	uint8_t r, g, b;

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print(F("PX"));
	}

#endif

	if (readColour(&r, &g, &b))
	{
		transitionToColor(no, r, g, b);

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("OK"));
		}

#endif
	}
}

// PFddd - set flicker speed to value given
void remoteSetFlickerSpeed()
{
#ifdef PIXEL_COLOUR_DEBUG
	Serial.println(".**remoteSetFlickerSpeed: ");
#endif

	int result;

	if (!getValue(&result))
	{
		return;
	}

	uint8_t no = (uint8_t)result;

#ifdef PIXEL_COLOUR_DEBUG
	Serial.print(".  Setting: ");
	Serial.println(no);
#endif

	setFlickerUpdateSpeed(no);

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println(F("PFOK"));
	}

#endif
}

// PIppp,rrr,ggg,bbb
// Set individual pixel colour

void remoteSetIndividualPixel()
{

#ifdef PIXEL_COLOUR_DEBUG
	Serial.println(".**remoteSetIndividualPixel: ");
#endif

	int result;

	if (!getValue(&result))
	{
		return;
	}

	uint8_t no = (uint8_t)result;

#ifdef PIXEL_COLOUR_DEBUG
	Serial.print(".  Setting: ");
	Serial.println(no);
#endif

	decodePos++;

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.print("PI");
	}

#endif

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println(F("Fail: mising colours after pixel"));
		}

#endif

		return;
	}

	uint8_t r, g, b;

	if (readColour(&r, &g, &b))
	{
		setLightColor(r, g, b, no);

#ifdef DIAGNOSTICS_ACTIVE

		if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
		{
			Serial.println("OK");
		}

#endif
	}
}

void remoteSetPixelsOff()
{

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println("POOK");
	}

#endif

	setAllLightsOff();
}


void remoteSetRandomColors()
{

#ifdef DIAGNOSTICS_ACTIVE

	if (diagnosticsOutputLevel & STATEMENT_CONFIRMATION)
	{
		Serial.println("PROK");
	}

#endif

	randomiseLights();
}

void remotePixelControl()
{
	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
	{

#ifdef DIAGNOSTICS_ACTIVE
		Serial.println(F("FAIL: mising pixel control command character"));
#endif
		return;
	}

#ifdef PIXEL_COLOUR_DEBUG
	Serial.println(".**remotePixelControl: ");
#endif

	char commandCh = *decodePos;

#ifdef PIXEL_COLOUR_DEBUG
	Serial.print(".  Pixel Command code : ");
	Serial.println(commandCh);
#endif

	decodePos++;

	switch (commandCh)
	{
	case 'a':
	case 'A':
		flickerOn();
		break;
	case 's':
	case 'S':
		flickerOff();
		break;
	case 'i':
	case 'I':
		remoteSetIndividualPixel();
		break;
	case 'o':
	case 'O':
		remoteSetPixelsOff();
		break;
	case 'c':
	case 'C':
		remoteColouredCandle();
		break;
	case 'f':
	case 'F':
		remoteSetFlickerSpeed();
		break;
	case 'x':
	case 'X':
		remoteFadeToColor();
		break;
	case 'r':
	case 'R':
		remoteSetRandomColors();
		break;
	case 'n':
	case 'N':
		remoteSetColorByName();
		break;
	}
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
		char ch = EEPROM.read(programPosition);

		if (ch == PROGRAM_TERMINATOR | programPosition == EEPROM_SIZE)
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

		char programUint8_t = EEPROM.read(programPosition++);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
		Serial.print("Statement at: ");
		Serial.print(statementStart);
		Serial.print(" starting: ");
		Serial.println(programUint8_t);
#endif
		if (programUint8_t != 'C' & programUint8_t != 'c')
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

		programUint8_t = EEPROM.read(programPosition++);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG

		Serial.print("Second statement character: ");
		Serial.println(programUint8_t);

#endif

		// if we get here we have a control command - see if the command is a label
		if (programUint8_t != 'L' & programUint8_t != 'l')
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

		while (*labelTest != STATEMENT_TERMINATOR & programPosition < EEPROM_SIZE)
		{
			programUint8_t = EEPROM.read(programPosition);

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
			Serial.print("Destination uint8_t: ");
			Serial.print(*labelTest);
			Serial.print(" Program uint8_t: ");
			Serial.println(programUint8_t);
#endif

			if (*labelTest == programUint8_t)
			{

#ifdef FIND_LABEL_IN_PROGRAM_DEBUG
				Serial.println("Got a match");
#endif
				// Move on to the next uint8_t
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

		// Get the uint8_t at the end of the destination statement

		programUint8_t = EEPROM.read(programPosition);

		if (*labelTest == programUint8_t)
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

	char *labelPos = decodePos;
	char *labelSearch = decodePos;

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

		char *labelPos = decodePos;
	char *labelSearch = decodePos;

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

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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

	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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
	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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
	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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

	Serial.println(version);
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
	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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
	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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
	while (*decodePos != STATEMENT_TERMINATOR & decodePos != decodeLimit)
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
	if (*decodePos == STATEMENT_TERMINATOR | decodePos == decodeLimit)
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
	case 'M':
	case 'm':
		remoteMoveControl();
		break;
	case 'P':
	case 'p':
		remotePixelControl();
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
	case 's':
	case 'S':
		remoteSoundPlay();
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

void processCommandUint8_t(uint8_t b)
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

void interpretSerialUint8_t(uint8_t b)
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

void processSerialUint8_t(uint8_t b)
{
#ifdef COMMAND_DEBUG
	Serial.print(F(".**processSerialUint8_t: "));
	Serial.println((char)b);
#endif

	switch (deviceState)
	{
	case EXECUTE_IMMEDIATELY:
		decodeScriptChar(b, interpretSerialUint8_t);
		break;
	case STORE_PROGRAM:
		decodeScriptChar(b, storeReceivedUint8_t);
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
	char programUint8_t;

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
		programUint8_t = EEPROM.read(programCounter++);

		if (programCounter >= EEPROM_SIZE || programUint8_t == PROGRAM_TERMINATOR)
		{
			haltProgramExecution();
			return false;
		}

#ifdef PROGRAM_DEBUG
		Serial.print(F(".    program uint8_t: "));
		Serial.println(programUint8_t);
#endif

		processCommandUint8_t(programUint8_t);

		if (programUint8_t == STATEMENT_TERMINATOR)
			return true;
	}
}

void updateHullOS()
{

	// If we recieve serial data the program that is running
	// must stop.
	while (CharsAvailable())
	{
		uint8_t b = GetRawCh();
		processSerialUint8_t(b);
	}

	switch (programState)
	{
	case PROGRAM_STOPPED:
	case PROGRAM_PAUSED:
		break;
	case PROGRAM_ACTIVE:
		exeuteProgramStatement();
		break;
	case PROGRAM_AWAITING_MOVE_COMPLETION:
		if (!motorsMoving())
		{
			programState = PROGRAM_ACTIVE;
		}
		break;
	case PROGRAM_AWAITING_DELAY_COMPLETION:
		if (millis() > delayEndTime)
		{
			programState = PROGRAM_ACTIVE;
		}
		break;
	}
}

bool commandsNeedFullSpeed()
{
	return deviceState != EXECUTE_IMMEDIATELY;
}
