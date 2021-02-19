#pragma once


// Performs the variable management 
// Variables can be given names, stored and evaluated
// Simple two operand expressions only

#define NUMBER_OF_VARIABLES 20
#define MAX_VARIABLE_NAME_LENGTH 10

enum parseOperandResult {
	INVALID_VARIABLE_NAME=1,
	NO_ROOM_FOR_VARIABLE=2,
	VARIABLE_NOT_FOUND=3,
	VARIABLE_NAME_TOO_LONG=4,
	VARIABLE_NAME_OK=5,
	OPERAND_OK=6,
	USING_UNASSIGNED_VARIABLE=7,
	INVALID_OPERAND=8,
	INVALID_HARDWARE_READING_NAME=9,
	NO_DIGITS_IN_VALUE=10,
	FIRST_VARIABLE_NOT_FOUND=11,
	FIRST_VARIABLE_USED_BEFORE_DEFINITION=12,
	INVALID_OPERATOR=13,
	SECOND_VARIABLE_NOT_FOUND=14,
	SECOND_VARIABLE_USED_BEFORE_DEFINITION=15,
};

//#define VAR_DEBUG

struct op
{
	char operatorCh;
	int(*evaluator) (int, int);
};

int evaluatePlus(int op1, int op2);
extern struct op addOp;

int evaluateMinus(int op1, int op2);
extern struct op minusOp;

int evaluateTimes(int op1, int op2);
extern struct op timesOp ;

int evaluateDivide(int op1, int op2);
extern struct op divideOp;

int evaluateModulus(int op1, int op2);
extern struct op modulusOp;

#define NUMBER_OF_ARITHMETIC_OPERATORS 5

extern struct op * operators[];

bool validOperator(char ch);

op * findOperator(char ch);

struct logicalOp
{
	char * operatorCh;
	bool(*evaluator) (int, int);
};

bool equalsOp(int op1, int op2);
extern struct logicalOp logicEquals;

bool notEqualsOp(int op1, int op2);
extern struct logicalOp logicNotEquals;

bool lessThanOp(int op1, int op2);
extern struct logicalOp logicLessThan;

bool greaterThanOp(int op1, int op2);
extern struct logicalOp logicGreaterThan;

bool lessThanEqualsOp(int op1, int op2);
extern struct logicalOp logicLessThanEquals;

bool greaterThanEqualsOp(int op1, int op2);
extern struct logicalOp logicGreaterThanEquals;

bool greaterThanEqualsOp(int op1, int op2);
extern struct logicalOp logicGreaterThanEquals;

#define NUMBER_OF_LOGICAL_OPERATORS 6

extern struct logicalOp * logicalOps[] ;

struct logicalOp * findLogicalOp(char * text);

struct reading {
	char * name;
	int(*reader)(void);
};

inline bool isReadingNameStart(char * ch);
inline bool isReadingNameChar(char * ch);

#define READING_START_CHAR '@'

int readTest();
extern struct reading test;

int readRandom();
extern struct reading randomReading;

#define NO_OF_HARDWARE_READERS 2

extern struct reading * readers[];

bool validReading(char * text);
struct reading * getReading(char * text);

struct variable
{
	bool empty;
	bool unassigned;
	// add one to the end for the terminating zero
	char name[MAX_VARIABLE_NAME_LENGTH + 1];
	int value;
};

extern variable variables[];
void clearVariableSlot(int position);
void clearVariables();
void setupVariables();
void setVariable(int position, int value);
int getVariable(int position);
bool isAssigned(int position);
bool isVariableNameStart(char * ch);
bool isVariableNameChar(char * ch);
bool variableSlotEmpty(int position);
int checkIdentifier(char * var);
bool matchVariable(int position, char * text);

// returns the length of the variable name at the given position in the variable store
// used for calculating pointer updates
int getVariableNameLength(int position);

// A variable name must start with a letter and then contain letters and digits only
// This method searches the variable store for a variable of the given name and then 
// sets position to the variable store offset for that variable. 
// Returns OPERAND_OK if all is well
// The parameter points to the area of memory holding the variable name. The variable name is judged to 
// have ended when a non-text/digit character is found
//
parseOperandResult findVariable(char * name, int *position);

// finds an empty location in the variable table and returns the offset into that table
// returns NO_ROOM_FOR_VARIABLE if the table is full
parseOperandResult findVariableSlot(int * result);

// returns INVALID_VARIABLE_NAME if the name is invalid 
// returns NO_ROOM_FOR_VARIABLE if the variable cannot be stored
// returns VARIABLE_NAME_TOO_LONG if the name of the variable is longer than the store length
parseOperandResult createVariable(char * namePos, int * varPos);

// Variable management
// Uses the decode buffer pointers
//
bool readInteger(int * result);

// Gets an operand from the current data feed
// This will either be a literal value, the contents of a variable or the contents of a system variable
// it returns an error code
void skipCodeSpaces(void);

parseOperandResult parseOperand(int * result);
bool getOperand(int * result);

// decodepos points to the first character of a value sequence
// It is either a literal, variable or two operand expression
bool getValue(int * result);

// called from the command processor
// the global variable decodePos holds the position in the decode array (first character
// of the variable name) and the global variable decodeLimit the end of the array

bool testCondition(bool * result);
void setVariable();
void viewVariable();
