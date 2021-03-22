#pragma once

#define DEBUG debug

#ifdef DEBUG

#define TRACE(s) Serial.print(s)
#define TRACE_HEX(s) Serial.print(s, HEX)
#define TRACELN(s) Serial.println(s)
#define TRACE_HEXLN(s) Serial.println(s, HEX)

#else

#define TRACE(s) 
#define TRACE_HEX(s) 
#define TRACELN(s) 
#define TRACE_HEXLN(s) 

#endif
