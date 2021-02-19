
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

// command numbers:             0    1 2  3  4    5  6     7      8        9    10    11  12    13   14   15    16    17    18    19    20     
const char commandNames[] = "delay#set#if#do#while#endif#forever#endwhile#until#clear#run#else#wait#stop#begin#end#print#println#break#continue#"; // don't forget the # on the end

char scriptInputBuffer[SCRIPT_INPUT_BUFFER_LENGTH];

int scriptInputBufferPos;

// The line number in the script
// Used when reporting errors

int scriptLineNumber;

// Flag to indicate an error has been detected
// Used for error reporting

bool programError;

// Flag to indicate that a program is being compiled - i.e. a begin keyword has been detected

bool compilingProgram = false;

// The start position of the command in the input buffer
// Set by decodeCommand
// Shared with all the functions below
char * commandStartPos;

// THIS MEANS THAT THIS COMPILER IS NOT REENTRANT

// The position in the input buffer 
// Set to the start of the command buffer by decodeScriptLine
// Shared with all the functions below and updated by them

// THIS MEANS THAT THIS COMPILER IS NOT REENTRANT

char * bufferPos;

// The position in the command names
// Set to the start of the command names by decodeScriptLine
// Shared with all the functions below and updated by some

// THIS ALSO MEANS THAT THIS COMPILER IS NOT REENTRANT

int scriptCommandPos;

// The indent level of the current statement
// Starts at 0 and increases with each block construction
// Shared with all the functions below and updated by some

// THIS ALSO MEANS THAT THIS COMPILER IS NOT REENTRANT

byte currentIndentLevel;

// True if the previous statement started a block
// This statement is allowed to set a new indent level
//

bool previousStatementStartedBlock;

bool displayErrors = true;

// The function to be used to send out comipiled bytes. 
// Set at the start of the line by decodeScriptLine
// Shared with all the functions below

// YET ANOTHER REASON THAT THIS COMPILER IS NOT REENTRANT

void(*outputFunction) (byte);

bool spinToCommandEnd()
{
	while (true)
	{
		char ch = pgm_read_byte_near(commandNames + scriptCommandPos);

		if (ch == 0)
			// end of the string in memory
			return false;

		if (ch == COMMAND_NAME_TERMINATOR)
		{
			// move past the terminator
			scriptCommandPos++;
			return true;
		}

		// move to the next character
		scriptCommandPos++;
	}
}

unsigned char skipInputSpaces()
{
	unsigned char result = 0;

	while (*bufferPos == ' ')
	{
		result++;
		bufferPos++;
	}
	return result;
}

void writeBytesFromBuffer(int length)
{
	for (int i = 0; i < length; i++)
	{
		outputFunction(*bufferPos);
		bufferPos++;
	}
}

void writeMatchingStringFromBuffer(char * string)
{
	while (*string)
	{
		outputFunction(*bufferPos);
		bufferPos++;
		string++;
	}
}

//#define COMPARE_COMMAND_DEBUG

ScriptCompareCommandResult compareCommand()
{
	// Start at the buffer position

	char * comparePos = bufferPos;

	while (true)
	{
		char ch = pgm_read_byte_near(commandNames + scriptCommandPos);

#ifdef COMPARE_COMMAND_DEBUG
		Serial.print(ch);
#endif

		if (ch == 0)
			return END_OF_COMMANDS;

		char inputCh = toLowerCase(*comparePos);

		// If we have reached the end of the command and the end of the input at the same time
		// we have a match. End of the input is a space or the end of the line
		if ((ch == COMMAND_NAME_TERMINATOR) && (inputCh == ' ' || inputCh == 0))
		{
			// Set the buffer position to the end of the command
			bufferPos = comparePos;

#ifdef COMPARE_COMMAND_DEBUG
			Serial.println("..match");
#endif
			return COMMAND_MATCHED;
		}

		if (ch != inputCh)
		{
#ifdef COMPARE_COMMAND_DEBUG
			Serial.println("..fail");
#endif
			return COMMAND_NOT_MATCHED;
		}
		scriptCommandPos++;
		comparePos++;
	}
}

// Decodes the command held in the area of memory referred to by bufferPos
int decodeCommandName()
{
	// Set the position in the command list to the start of the list
	scriptCommandPos = 0;

	// Set the command counter to 0
	int commandNumber = 0;

	skipInputSpaces();

	// ignore empty lines
	if (*bufferPos==0)
		return COMMAND_EMPTY_LINE;

	// Set commandStartPos to point to the start of the statement being decoded
	// Used when decoding colour names

	commandStartPos = bufferPos;

	// it is a system command - just return this immediately

	if (*bufferPos == '*')
	{
		// skip past the *
		bufferPos++;
		// return the command type
		return COMMAND_SYSTEM_COMMAND;
	}

	while (true)
	{
		ScriptCompareCommandResult result = compareCommand();

		switch (result)
		{
		case COMMAND_MATCHED:
			return commandNumber;

		case COMMAND_NOT_MATCHED:
			if (!spinToCommandEnd())
				return -1;
			commandNumber++;
			break;

		case END_OF_COMMANDS:
			return -1;
			break;
		}
	}
}

//#define SCRIPT_DEBUG

#ifdef SCRIPT_DEBUG

char dumpBuffer[DUMP_BUFFER_SIZE];

int dumpBufferPos = 0;

boolean addDumpByte(unsigned char b)
{
	if (dumpBufferPos < DUMP_BUFFER_LIMIT)
	{
		dumpBuffer[dumpBufferPos] = b;
		dumpBufferPos++;
		return true;
	}
	return false;
}

void displayDump()
{
	dumpBuffer[dumpBufferPos] = 0;
	Serial.println(dumpBuffer);
	dumpBufferPos = 0;
}

void dumpByte(unsigned char b)
{
	storeProgramByte(b);
	if (b == STATEMENT_TERMINATOR)
	{
		displayDump();
	}
	else
	{
		addDumpByte((char)b);
	}
}

#endif

int processSingleValue()
{
	skipInputSpaces();

	if (isVariableNameStart(bufferPos))
	{
		// its a variable
		int position;

		if (findVariable(bufferPos,&position) == VARIABLE_NOT_FOUND)
		{
			return VARIABLE_USED_BEFORE_IT_WAS_CREATED;
		}

		// copy the variable into the instruction

		int variableLength = getVariableNameLength(position);

		for (int i = 0; i < variableLength; i++)
		{
			outputFunction(*bufferPos);
			bufferPos++;
		}
		return ERROR_OK;
	}

	if (isdigit(*bufferPos) | (*bufferPos == '+') | (*bufferPos == '-'))
	{
		bool firstch = true;

		while (true)
		{
			char ch = *bufferPos;

			if ((ch<'0') | (ch>'9'))
			{
				if (firstch)
				{
					if (ch != '+' && ch != '-')
					{
						return ERROR_INVALID_DIGIT_IN_NUMBER;
					}
				}
				else
				{
					return ERROR_OK;
				}
			}
			outputFunction(ch);
			firstch = false;
			bufferPos++;
		}
	}

	if (*bufferPos == READING_START_CHAR)
	{
		// Move past the start character

		bufferPos++;

		struct reading * reader = getReading(bufferPos);

		if (reader == NULL)
		{
			return ERROR_INVALID_HARDWARE_READ_DEVICE;
		}

		// Drop out the char to start the hardware name

		outputFunction(READING_START_CHAR);

		// copy the variable into the instruction

		int readerLength = strlen(reader->name);

		for (int i = 0; i < readerLength; i++)
		{
			outputFunction(*bufferPos);
			bufferPos++;
		}
		return ERROR_OK;
	}
    return ERROR_MISSING_SINGLE_VALUE;
}

int processValue()
{
	int result = processSingleValue();

	if (result != ERROR_OK)
		return result;

	skipInputSpaces();

	if (*bufferPos == 0)
		// Just a single value - no expression 
		return ERROR_OK;

	if (validOperator(*bufferPos))
	{
		// write out the operator
		outputFunction(*bufferPos);

		// move past the operator
		bufferPos++;

		skipInputSpaces();

		// process the second value
		return processSingleValue();
	}

	previousStatementStartedBlock = false;

	return ERROR_OK;
}

void sendCommand( const char *command)
{
	int pos = 0;

	while (true)
	{
		unsigned char b = command[pos];

		if (b == 0)
			break;

		outputFunction(b);
		pos++;
	}
}

void endCommand()
{
	outputFunction(STATEMENT_TERMINATOR);
}

void abandonCompilation()
{
	programError = true;
}

const char delayCommand[] = "CD";

int compileDelay()
{
#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling delay: "));
#endif // SCRIPT_DEBUG

	skipInputSpaces();

	if (*bufferPos == 0)
	{
		return ERROR_MISSING_TIME_IN_DELAY;
	}

	sendCommand(delayCommand);

	previousStatementStartedBlock = false;

	return processValue();
}
const char setCommand[] = "VS";

int compileAssignment()
{
#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling set: "));
#endif // SCRIPT_DEBUG

	// Not allowed to indent after a set
	previousStatementStartedBlock = false;

	skipInputSpaces();

	if (checkIdentifier(bufferPos) != VARIABLE_NAME_OK)
		return ERROR_INVALID_VARIABLE_NAME_IN_SET;

	int position ;

	if (findVariable(bufferPos, &position) == VARIABLE_NOT_FOUND)
	{
		if (createVariable(bufferPos, &position) == NO_ROOM_FOR_VARIABLE)
		{
			return ERROR_TOO_MANY_VARIABLES;
		}
	}

	sendCommand(setCommand);

	writeBytesFromBuffer(getVariableNameLength(position));

	skipInputSpaces();

	if (*bufferPos != '=')
	{
		return ERROR_NO_EQUALS_IN_SET;
	}

	bufferPos++; // skip past the equals
	outputFunction('=');  // write the equals

	skipInputSpaces();

	return processValue();
}

struct stackItem {
	unsigned char constructionType;
	int count;
	unsigned char indentLevel;
};

#define STACK_SIZE 10

struct stackItem operation[STACK_SIZE];

int operationStackPointer;

#define EMPTY_STACK -1
#define IF_CONSTRUCTION_STACK_ITEM 1
#define WHILE_CONSTRUCTION_STACK_ITEM 3
#define FOREVER_CONSTRUCTION_STACK_ITEM 4

int labelCounter;

void dropValue(int value)
{
	while (true)
	{
		char ch = '0' + (value % 10);
		outputFunction(ch);
		value = value / 10;
		if (value == 0)
			break;
	}
	return;
}


// Push an operation onto the operation stack.
// This manages the if, do and while constructions
//
void push_operation(unsigned char type, unsigned char count)
{
	operation[operationStackPointer].constructionType = type;
	operation[operationStackPointer].count = count;
	operation[operationStackPointer].indentLevel = currentIndentLevel;
	operationStackPointer++;
}

// Get the type of the top operation without removing anything from the stack
// We need to use this to check to make sure that the end element of a construction
// matches the start element.

bool inline operation_stack_empty()
{
	return operationStackPointer == 0;
}

unsigned char top_operation_type()
{
	if (operationStackPointer == 0)
		return EMPTY_STACK;

	return operation[operationStackPointer - 1].constructionType;
}

int top_operation_label()
{
	if (operationStackPointer == 0)
		return EMPTY_STACK;

	return operation[operationStackPointer - 1].count;
}


unsigned char top_operation_indent_level()
{
	if (operationStackPointer == 0)
		return EMPTY_STACK;

	return operation[operationStackPointer - 1].indentLevel;
}

// Get the top value on the operation stack
int pop_operation_count()
{
	operationStackPointer--;
	return operation[operationStackPointer].count;
}

const char labelCommand[] = "CLl";

void dropLabel(int labelCounter)
{
	// first character of the label
	sendCommand(labelCommand);
	dropValue(labelCounter);
}

void dropLabelStatement(int labelCounter)
{
	dropLabel(labelCounter);
	endCommand();
}

void pushLabel(unsigned char labelType)
{
	labelCounter++; // move on to the next construction
	push_operation(labelType, labelCounter);
	dropLabel(labelCounter);
}

const char jumpCommand[] = "CJl";

void dropJump(int labelCounter)
{
	sendCommand(jumpCommand);
	dropValue(labelCounter);
}

void dropJumpCommand(int labelCounter)
{
	dropJump(labelCounter);
	endCommand();
}

void resetScriptLine()
{
	scriptInputBufferPos = 0;
}

void beginCompilingStatements()
{
	currentIndentLevel = 0;
	previousStatementStartedBlock = false;
	operationStackPointer = 0;
	labelCounter = 0;
	resetScriptLine();
	scriptLineNumber = 1; // start at the first line
	programError = false; // indicate that no errors were detected
	compilingProgram = true; // indicate that we are compiling a program
}

const char endCommandText[] = "RX";

const char failedCommandText[] = "RA";

void endCompilingStatements()
{
	if (programError)
	{
		sendCommand(failedCommandText);
		Serial.println("Errors");
	}
	else
	{
		sendCommand(endCommandText);
		Serial.println("OK");
	}

	compilingProgram = false;
}

// Drops a comparison statement
int dropComparisonStatement(int labelNo, bool trueTest)
{
	outputFunction('C');

	if (trueTest)
		outputFunction('T');
	else
		outputFunction('F');

	skipInputSpaces();

	// Get the first value in the logical expression
	int result = processSingleValue();

	if (result != ERROR_OK)
		return result;

	// Skip to the logical operator
	skipInputSpaces();

	// Get the logical operator
	struct logicalOp * ifOp = findLogicalOp(bufferPos);

	// Abandon if there is no matching logical operator
	if (ifOp == NULL)
	{
		// no logical operator
		return ERROR_MISSING_OPERATOR_IN_COMPARE;
	}

	// Write out the logical operator
	writeMatchingStringFromBuffer(ifOp->operatorCh);

	// Skip to the second operand
	skipInputSpaces();

	// process the second operand
	result = processSingleValue();

	if (result != ERROR_OK)
		return result;

	// if we get here the condition is valid and we need to drop out the destination label
	// for the branch past the 

	outputFunction(',');  // write the comma

	// Drop out the first character of the label (which is l)
	outputFunction('l');
	// drop the label counter value
	dropValue(labelNo);

	return ERROR_OK;
}

int compileIf()
{

	if (!compilingProgram)
	{
		return ERROR_IF_CANNOT_BE_USED_OUTSIDE_A_PROGRAM;
	}


#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling if: "));
#endif // SCRIPT_DEBUG

	labelCounter++; // move on to the next label

					// Add the start of the if to the operation stack

	push_operation(IF_CONSTRUCTION_STACK_ITEM, labelCounter);

	int result = dropComparisonStatement(labelCounter, false);

	labelCounter++; // reserve a label for use by else - if any

	previousStatementStartedBlock = true;

	return result;
}

int compileElse()
{
#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling else: "));
#endif // SCRIPT_DEBUG
	if (!compilingProgram)
	{
		return ERROR_ELSE_CANNOT_BE_USED_OUTSIDE_A_PROGRAM;
	}

	return ERROR_OK;
}

int compileWhile()
{
#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling while: "));
#endif // SCRIPT_DEBUG

	if (!compilingProgram)
	{
		return ERROR_WHILE_CANNOT_BE_USED_OUTSIDE_A_PROGRAM;
	}

	// First drop out a label so that 
	// we can branch back to the top

	pushLabel(WHILE_CONSTRUCTION_STACK_ITEM);

	// Going to follow this command with another
	endCommand();

	labelCounter++; // move on to the next label

	// Now insert the branch past the loop

	previousStatementStartedBlock = true;

	return dropComparisonStatement(labelCounter, false);
}

int compileForever()
{

#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling forever: "));
#endif // SCRIPT_DEBUG

	if (!compilingProgram)
	{
		return ERROR_FOREVER_CANNOT_BE_USED_OUTSIDE_A_PROGRAM;
	}

	// First drop out a label so that 
	// we can branch back to the top

	pushLabel(FOREVER_CONSTRUCTION_STACK_ITEM);

	labelCounter++; // move on to the next label

					// Now insert the branch past the loop

	previousStatementStartedBlock = true;

	return ERROR_OK;
}


// finds the most recent loop construction on the 
// operation stack. Returns -1 if no suitable construction found
//

#define NO_LABEL_FOR_LOOP_ON_STACK -1

int findTopLoopConstructionLabel()
{
	// Start the search at the top of the stack
	// Rememver that
	int searchStackPointer = operationStackPointer;


	// If the operation stack pointer is zero there is nothing
	// on the stack
	while (searchStackPointer > 0)
	{
		searchStackPointer--; // climb down the stack
							  // pointer aways points to next free location
		unsigned char constructionType = operation[searchStackPointer].constructionType;

		if ((constructionType == WHILE_CONSTRUCTION_STACK_ITEM) || (constructionType == FOREVER_CONSTRUCTION_STACK_ITEM))
		{
			// found a loop construction
			// return the label from that loop
			return operation[searchStackPointer].count;
		}
	}

	// If we get here there is no loop construction available - which is an error

	return NO_LABEL_FOR_LOOP_ON_STACK;

}

int compileBreak()
{

	// Not allowed to indent after a break
	previousStatementStartedBlock = false;

#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling break: "));
#endif // SCRIPT_DEBUG

	if (!compilingProgram)
	{
		return ERROR_BREAK_CANNOT_BE_USED_OUTSIDE_A_PROGRAM;
	}

	int operation_label = findTopLoopConstructionLabel();

	if (operation_label == NO_LABEL_FOR_LOOP_ON_STACK)
		return ERROR_NO_LABEL_FOR_LOOP_ON_STACK_IN_BREAK;

	// first label value is the jump for the loop repeat
	// next label value is the label after the end of the loop

	dropJump(operation_label + 1);
	return ERROR_OK;
}

int compileContinue()
{

#ifdef SCRIPT_DEBUG
	Serial.print(F("Compiling break: "));
#endif // SCRIPT_DEBUG

	// Not allowed to indent after a continue
	previousStatementStartedBlock = false;


	if (!compilingProgram)
	{
		return ERROR_CONTINUE_CANNOT_BE_USED_OUTSIDE_A_PROGRAM;
	}

	int operation_label = findTopLoopConstructionLabel();

	if (operation_label == NO_LABEL_FOR_LOOP_ON_STACK)
		return ERROR_NO_LABEL_FOR_LOOP_ON_STACK_IN_CONTINUE;

	// first label value is the jump for the loop repeat

	dropJump(operation_label);

	return ERROR_OK;
}


/// Program control commands - not part of the script
//

const char clearVariablesCommand[] = "VC";

int clearProgram()
{
#ifdef SCRIPT_DEBUG
	Serial.print(F("Performing clear program: "));
#endif // SCRIPT_DEBUG


	if (compilingProgram)
	{
		return ERROR_CLEAR_WHEN_COMPILING_PROGRAM;
	}

	sendCommand(clearVariablesCommand);

	return ERROR_OK;
}

const char runCommand[] = "RS";

int runProgram()
{
#ifdef SCRIPT_DEBUG
	Serial.print(F("Performing run program: "));
#endif // SCRIPT_DEBUG

	if (compilingProgram)
	{
		return ERROR_RUN_WHEN_COMPILING_PROGRAM;
	}

	sendCommand(runCommand);

	return ERROR_OK;
}

const char waitCommand[] = "CA";

int compileWait()
{
	// Not allowed to indent after a wait
	previousStatementStartedBlock = false;

	sendCommand(waitCommand);

	return ERROR_OK;
}

const char stopCommand[] = "RH";

int compileStop()
{

	// Not allowed to indent after a sound
	previousStatementStartedBlock = false;

	if (compilingProgram)
	{
		return ERROR_STOP_WHEN_COMPILING_PROGRAM;
	}

	sendCommand(stopCommand);
	return ERROR_OK;
}

const char clearCommand[] = "RC";
const char beginCommand[] = "RM";

int compileBegin()
{
	// Not allowed to indent after a begin
	previousStatementStartedBlock = false;

	if (compilingProgram)
	{
		return ERROR_BEGIN_WHEN_COMPILING_PROGRAM;
	}

	beginCompilingStatements();
	sendCommand(clearCommand);
	endCommand();
	sendCommand(beginCommand);
	return ERROR_OK;
}

int compileEnd()
{
	// Not allowed to indent after a end
	previousStatementStartedBlock = false;

	if (!compilingProgram)
	{
		return ERROR_END_WHEN_NOT_COMPILING_PROGRAM;
	}

	endCompilingStatements();

	return ERROR_OK;
}

// compile a print statement
// The command is followed by an expression or a string of text enclosed in " characters
//
int compilePrint()
{
	// Not allowed to indent after a print
	previousStatementStartedBlock = false;

	// first character of the write command
	outputFunction('W');

	skipInputSpaces();

	if (*bufferPos == '"')
	{
		// start of a message - just drop out the string of text
		outputFunction('T');

		bufferPos++; // skip the starting double quote
		while (*bufferPos != 0 && *bufferPos != '"')
		{
			outputFunction(*bufferPos);
			bufferPos++;
		}
		if (*bufferPos == 0)
		{
			return ERROR_MISSING_CLOSE_QUOTE_ON_PRINT;
		}
		else
		{
			return ERROR_OK;
		}
	}
	else 
	{
		// start of a value - just drop out the expression
		outputFunction('V');
		// dropping a value - just process it
		return processValue();
	}
}

const char newlineCommand[] = "WL";

int compilePrintln()
{
	// Not allowed to indent after a println
	previousStatementStartedBlock = false;

	compilePrint();

	// Going to follow this command with another
	endCommand();

	sendCommand(newlineCommand);
	return ERROR_OK;
}

// Decodes a zero terminated script line into a sequence of unsigned char commands
// The unsigned char commnds are sent to the output function specified
// The script line is not buffered, and must not change while this function is running


int compileDirectCommand()
{
	// Not allowed to indent after a sound
	previousStatementStartedBlock = false;

	while (*bufferPos)
	{
		outputFunction(*bufferPos);
		bufferPos++;
	}
	return ERROR_OK;
}

int processCommand(unsigned char commandNo)
{
	switch (commandNo)
	{
	case COMMAND_DELAY:// delay
		return compileDelay();

	case COMMAND_IF:// if
		return compileIf();

	case COMMAND_WHILE:// while
		return compileWhile();

	case COMMAND_CLEAR: // clear	
		return clearProgram();

	case COMMAND_RUN: // run
		return runProgram();

	case COMMAND_ELSE: // else
		return compileElse();

	case COMMAND_FOREVER: // forever
		return compileForever();

	case COMMAND_SET:
		return compileAssignment();

	case COMMAND_WAIT:
		return compileWait();

	case COMMAND_STOP:
		return compileStop();

	case COMMAND_BEGIN:
		return compileBegin();

	case COMMAND_END:
		return compileEnd();

	case COMMAND_PRINT:
		return compilePrint();

	case COMMAND_PRINTLN:
		return compilePrintln();

	case COMMAND_SYSTEM_COMMAND:
		return compileDirectCommand();

	case COMMAND_BREAK:
		return compileBreak();

	case COMMAND_CONTINUE:
		return compileContinue();

	default:
		return compileAssignment();
	}

	return ERROR_INVALID_COMMAND;
}

// Called when the indent level of a new line is less than that of the 
// previous line. This means that we are closing off one or more indent
// levels. Also called at the end of compilation to close off any loops 
// or conditions. Supplied with the new level of indent and the 
// number of the command on the statement being indented

//#define SCRIPT_DEBUG_INDENT_OUT

int indentOutToNewIndentLevel(unsigned char indent, int commandNo)
{
	int result;
	int labelNo;

#ifdef SCRIPT_DEBUG_INDENT_OUT
	Serial.println("Indent out to new indent level");
	Serial.print("Indent: ");
	Serial.print(indent);
	Serial.print(" Command: ");
	Serial.print(commandNo);
	Serial.print(" Current Indent Level: ");
	Serial.println(currentIndentLevel);
#endif

	while (indent < currentIndentLevel)
	{
#ifdef SCRIPT_DEBUG_INDENT_OUT
		Serial.println("Looping");
#endif
		if (operation_stack_empty())
		{
#ifdef SCRIPT_DEBUG_INDENT_OUT
			Serial.println("Operation stack empty");
#endif
			// This should not happen as it is not possible to
			// indent code without creating an enclosing command
			return ERROR_INDENT_OUTWARDS_WITHOUT_ENCLOSING_COMMAND;
		}

		// pull back the indent level to the previous one
		currentIndentLevel = top_operation_indent_level();

		// if this indent level is not the same as the indent
		// level of the item on the top of the stack we just close
		// this element off

#ifdef SCRIPT_DEBUG_INDENT_OUT
		Serial.print("New Current Indent Level: ");
		Serial.println(currentIndentLevel);
#endif
		// Generate the code to match the end of the 
		// enclosing statement

		switch (top_operation_type())
		{
			case IF_CONSTRUCTION_STACK_ITEM:
	#ifdef SCRIPT_DEBUG_INDENT_OUT
				Serial.print("Handling an if..");
	#endif
				// might have an else clause for an if construction

				// need to spin up through the operation stack looking for 
				// the if condition with the same indent as this else, as that is the 
				// one that matches. Any other items that we find (including do) will
				// need to be closed off at this point

				if (currentIndentLevel == indent && 
					commandNo == COMMAND_ELSE)
				{
	#ifdef SCRIPT_DEBUG_INDENT_OUT
					Serial.print("...with an else");
	#endif

					// get the destination label for the end of the code
					// skipped past if the if condition is not obeyed
					// This code will be obeyed as the else clause

					// get the label number for the label reached if we jump 
					// past the code controlled by the if

					labelNo = pop_operation_count();

					// drop a jump to the next label number
					// this number was reserved when the if was created
					// this is the position which will mark the end of the 
					// code performed by the else - when we see the endif

					dropJumpCommand(labelNo + 1);

					// Now drop a label to serve as the destination of the 
					// jump past the if clause code. This is the code obeyed 
					// if else is the case.

					dropLabel(labelNo);  // drop the label that is jumped

												  // Now need to push a label number for the endif to use
												  // to create the destination label for the jump past the 
												  // else code

					push_operation(IF_CONSTRUCTION_STACK_ITEM, labelNo + 1);

					// Allow statements after this one to indent
					previousStatementStartedBlock = true;
				}
				else
				{
	#ifdef SCRIPT_DEBUG_INDENT_OUT
					Serial.print("...on its own");
	#endif
					dropLabelStatement(pop_operation_count());
				}
				break;

			case WHILE_CONSTRUCTION_STACK_ITEM:

				labelNo = pop_operation_count();

				dropJumpCommand(labelNo);

				dropLabelStatement(labelNo + 1);
				break;

			case FOREVER_CONSTRUCTION_STACK_ITEM:

				labelNo = pop_operation_count();

				dropJumpCommand(labelNo);

				dropLabelStatement(labelNo + 1);
				break;

			default:
				result = ERROR_INDENT_OUTWARDS_HAS_INVALID_OPERATION_ON_STACK;
				break;
		}
	}
	// When we get here the indent of this statement should match the 
	// the indent level pushed onto the operation stack when we started
	// this block
	if (indent != currentIndentLevel)
	{
		result = ERROR_INDENT_OUTWARDS_DOES_NOT_MATCH_ENCLOSING_STATEMENT_INDENT;
	}
	else
	{
		result = ERROR_OK;
	}

	return result;

}

int decodeScriptLine(char * input, void(*output) (unsigned char))
{

	// Set the shared buffer pointer to point to the statement being decoded
	bufferPos = input;

	// Set the output function to point to the statement being output
	outputFunction = output;

	int result;

	unsigned char indent = skipInputSpaces();

	// Lines that start with a # are comments
	if (*bufferPos == '#')
	{
		return ERROR_OK;
	}

	int commandNo = decodeCommandName();

	if (commandNo == COMMAND_EMPTY_LINE)
	{
		// Ignore empty lines
		return ERROR_OK;
	}

#ifdef SCRIPT_DEBUG

	Serial.print(previousStatementStartedBlock);
	Serial.print(" Current indent: ");
	Serial.print(currentIndentLevel);
	Serial.print("Indent: ");
	Serial.println(indent);

	Serial.print("Compiling: ");
	Serial.println(input);

#endif // SCRIPT_DEBUG

	// Find the position of the first item 
	// sort out any outward indents


	if (compilingProgram)
	{
		if (indent < currentIndentLevel)
		{
			// new statement is being outdented 
			result = indentOutToNewIndentLevel(indent, commandNo);
			if (result == ERROR_OK)
			{
				result = processCommand(commandNo);
			}
		}
		else
		{
			if (indent > currentIndentLevel)
			{
				// Indenting the text
				// Only valid if we were pre-ceded by a 
				// statement that can cause an indent
				if (previousStatementStartedBlock)
				{
					// It's OK to increase the indent if you're starting a new block
					// Set the new indent level for this block
					currentIndentLevel = indent;
					// Now process the command
					result = processCommand(commandNo);
				}
				else
				{
					// Inconsistent indents are bad
					result = ERROR_INDENT_INWARDS_WITHOUT_A_VALID_ENCLOSING_STATEMENT;
				}
			}
			else
			{
				// At the same level - just process the command
				result = processCommand(commandNo);
			}
		}
	}
	else
	{
		// Immediate mode
		result = processCommand(commandNo);
	}

	if (result != ERROR_OK)
	{
		abandonCompilation();

		if (compilingProgram)
		{
			Serial.print("Line:  ");
			Serial.print(scriptLineNumber);
			Serial.print(" ");
		}

		Serial.print("Error: ");
		Serial.print(result);
		Serial.print(" ");
		Serial.println(input);
	}

	endCommand();

	return result;
}

int decodeScriptChar(char b, void(*output) (unsigned char))
{
	// convert linefeeds into carriage return

	if (b == '\n')
		b = STATEMENT_TERMINATOR;

	// convert upper case characters into lower case
	if ((b >= 'A') && (b <= 'Z'))
		b = b + 32;

	if (scriptInputBufferPos == SCRIPT_INPUT_BUFFER_LENGTH)
		return ERROR_SCRIPT_INPUT_BUFFER_OVERFLOW;

	if (b == STATEMENT_TERMINATOR)
	{
		scriptInputBuffer[scriptInputBufferPos] = 0;
		int result = decodeScriptLine(scriptInputBuffer, output);
		scriptLineNumber++; // move on to the next line
		resetScriptLine();
		return result;
	}

	scriptInputBuffer[scriptInputBufferPos++] = b;
	return ERROR_OK;
}

void testScript()
{
	beginCompilingStatements();
	clearVariables();

#ifdef SCRIPT_DEBUG

	Serial.print("Script test");

#endif // SCRIPT_DEBUG

	//  Serial.println(decodeCommandName("forward"));
	//  Serial.println(decodeCommandName("back"));
	//  Serial.println(decodeCommandName("wallaby"));

	//  decodeScriptLine("angry", dumpByte);
	//  decodeScriptLine("happy", dumpByte);

#ifdef SCRIPT_MOVE_TEST

	decodeScriptLine("move 50", dumpByte);
	decodeScriptLine("move 50 intime 10", dumpByte);
	decodeScriptLine("move", dumpByte);
	decodeScriptLine("move ", dumpByte);
	decodeScriptLine("move zz", dumpByte);
	decodeScriptLine("move 50zz", dumpByte);
	decodeScriptLine("move 50 intime", dumpByte);
	decodeScriptLine("move 50 intime 10", dumpByte);

#endif

	//#define SCRIPT_TURN_TEST

#ifdef SCRIPT_TURN_TEST

	decodeScriptLine("turn 50", dumpByte);
	decodeScriptLine("turn 50 intime 10", dumpByte);
	decodeScriptLine("turn", dumpByte);
	decodeScriptLine("turn ", dumpByte);
	decodeScriptLine("turn zz", dumpByte);
	decodeScriptLine("turn 50zz", dumpByte);
	decodeScriptLine("turn 50 intime", dumpByte);
	decodeScriptLine("turn 50 intime 10", dumpByte);

#endif

	//#define SCRIPT_ARC_TEST

#ifdef SCRIPT_ARC_TEST
	decodeScriptLine("arc 90, 180", dumpByte);
	decodeScriptLine("arc 90, 180 intime 100", dumpByte);
	decodeScriptLine("arc 90 , 80", dumpByte);
	decodeScriptLine("arc 90 ,80", dumpByte);
	decodeScriptLine("arc", dumpByte);
	decodeScriptLine("arc ", dumpByte);
	decodeScriptLine("arc zz", dumpByte);
	decodeScriptLine("arc 90", dumpByte);
	decodeScriptLine("arc 90,", dumpByte);
	decodeScriptLine("arc 90,zz", dumpByte);
	decodeScriptLine("arc 90+ 80", dumpByte);
#endif

	//#define SET_TEST
#ifdef SET_TEST
	decodeScriptLine("move x", dumpByte);
	decodeScriptLine("set x=99", dumpByte);
	decodeScriptLine("move x", dumpByte);
	decodeScriptLine("set x=x+1", dumpByte);
	decodeScriptLine("move x+10", dumpByte);

#endif

	//#define DELAY_TEST

#ifdef DELAY_TEST

	decodeScriptLine("delay 100", dumpByte);
	decodeScriptLine("delay", dumpByte);
	decodeScriptLine("delay ", dumpByte);
	decodeScriptLine("delay zz", dumpByte);
#endif

	//#define COLOUR_TEST

#ifdef COLOUR_TEST
	decodeScriptLine("colour 255,128,0", dumpByte);
	decodeScriptLine("colour 255,128,", dumpByte);
	decodeScriptLine("colour 255,128", dumpByte);
	decodeScriptLine("colour 255,", dumpByte);
	decodeScriptLine("colour 255", dumpByte);
	decodeScriptLine("colour ", dumpByte);
	decodeScriptLine("colour", dumpByte);

	decodeScriptLine("color 255,128,0", dumpByte);
	decodeScriptLine("color 255,128,", dumpByte);
	decodeScriptLine("color 255,128", dumpByte);
	decodeScriptLine("color 255,", dumpByte);
	decodeScriptLine("color 255", dumpByte);
	decodeScriptLine("color ", dumpByte);
	decodeScriptLine("color", dumpByte);

#endif

	//#define IF_TEST

#ifdef IF_TEST
	decodeScriptLine("if 1 > 20", dumpByte);
	decodeScriptLine("colour 255,128,0", dumpByte);
	decodeScriptLine("endif", dumpByte);
	decodeScriptLine("if 1 >= 20", dumpByte);
	decodeScriptLine("colour 255,128,255", dumpByte);
	decodeScriptLine("endif", dumpByte);

#endif

	//#define IF_ELSE_TEST

#ifdef IF_ELSE_TEST

	decodeScriptLine("do", dumpByte);
	decodeScriptLine("if %dist > 20", dumpByte);
	decodeScriptLine("    colour 255,0,0", dumpByte);
	decodeScriptLine("else", dumpByte);
	decodeScriptLine("    colour 0,255,0", dumpByte);
	decodeScriptLine("endif", dumpByte);
	decodeScriptLine("forever", dumpByte);

#endif


	//#define DO_TEST

#ifdef DO_TEST
	decodeScriptLine("set count = 0", dumpByte);
	decodeScriptLine("do", dumpByte);
	decodeScriptLine("colour 255,128,255", dumpByte);
	decodeScriptLine("delay 10", dumpByte);
	decodeScriptLine("colour 0,0,0", dumpByte);
	decodeScriptLine("delay 10", dumpByte);
	decodeScriptLine("set count = count + 1", dumpByte);
	decodeScriptLine("until count > 10", dumpByte);
#endif

	//#define WHILE_TEST

#ifdef WHILE_TEST
	decodeScriptLine("set count = 0", dumpByte);
	decodeScriptLine("while count < 10", dumpByte);
	decodeScriptLine("colour 255,128,255", dumpByte);
	decodeScriptLine("delay 10", dumpByte);
	decodeScriptLine("colour 0,0,0", dumpByte);
	decodeScriptLine("delay 10", dumpByte);
	decodeScriptLine("set count = count + 1", dumpByte);
	decodeScriptLine("endwhile", dumpByte);
#endif



	//  decodeScriptLine("turn 90", dumpByte);
  //  decodeScriptLine("arc 90, 180", dumpByte);
  //  decodeScriptLine("delay 10", dumpByte);
  //  decodeScriptLine("colour 255,0,255", dumpByte);
  //  decodeScriptLine("color 255,0,255", dumpByte);
  //  decodeScriptLine("pixel 255,255,255,0", dumpByte);
}
