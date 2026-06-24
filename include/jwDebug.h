#pragma once


// define  DEBUG_DISABLE_JW in platformio.ini build_flag
// ;-D DEBUG_DISABLE_JW

#ifndef DEBUG_DISABLE_JW
#define DEBUG_PRINTLN(x)  Serial.println(x)
#define DEBUG_PRINT(x)    Serial.print(x)
#define DEBUG_PRINTF(fmt,...) Serial.printf(fmt,##__VA_ARGS__)  
/* note the ##__VA_ARGS__ substitutes a variable number of arguments (inculding none) and deals with hanging comma that "none" otherwise creates */
#else
#define DEBUG_PRINTLN(x) 
#define DEBUG_PRINT(x)  
#define DEBUG_PRINTF(fmt,...)
/* note the ... refers to the variable number of trailing arguments that are substituted with the ##__VA_ARGS__ predefined macro */
#endif