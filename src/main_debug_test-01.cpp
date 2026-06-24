#include <Arduino.h>
//#define DEBUG_DISABLE_JW  //must be before #include "jwDebug.h", or early line in jwDebug.h, or in platformio.ini
#include "jwDebug.h"




// #ifndef DEBUG_DISABLE_JW
// #define DEBUG_PRINTLN(x)  Serial.println(x)
// #define DEBUG_PRINT(x)    Serial.print(x)
// #define DEBUG_PRINTF(fmt,...) Serial.printf(fmt,##__VA_ARGS__)  
// /* note the ##__VA_ARGS__ substitutes a variable number of arguments (inculding none) and deals with hanging comma that "none" otherwise creates */
// #else
// #define DEBUG_PRINTLN(x) 
// #define DEBUG_PRINT(x)  
// #define DEBUG_PRINTF(fmt,...)
// /* note the ... refers to the variable number of trailing arguments that are substituted with the ##__VA_ARGS__ predefined macro */
// #endif



// #ifndef DEBUG_DISABLE_JW
// #define DEBUG_PRINTLN(x)  Serial.println(x)
// #define DEBUG_PRINT(x)    Serial.print(x)
// #define DEBUG_PRINTF(fmt,...) Serial.printf(fmt,##__VA_ARGS__)  
// /* note the ##__VA_ARGS__ substitutes a variable number of arguments (inculding none) and deals with hanging comma that "none" otherwise creates */
// #else
// #define DEBUG_PRINTLN(x) 
// #define DEBUG_PRINT(x)  
// #define DEBUG_PRINTF(fmt,...)
// /* note the ... refers to the variable number of trailing arguments that are substituted with the ##__VA_ARGS__ predefined macro */
// #endif


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("*** STARTING SETUP - PRINT ALWAYS ***");
  DEBUG_PRINTLN("Println Only in debug");
  DEBUG_PRINT("Print Only in debug with 2 CR\n\n");

  float pi=3.14159;

  //Serial.printf("Always printf statement with 4 variables: %s - %s and %s: %6.4f\n","__BASE_FILE__","_LINE_","PI",pi);
  //Serial.printf("Always printf statement with 0 variables.\n");

  DEBUG_PRINTF("Debug printf statement __BASE_FILE__: %s  __FILE__: %s\n",__BASE_FILE__ , __FILE__);
  DEBUG_PRINTF("Debug printf statement __DATE__: %s __FILE_NAME__: %s __TIME__: %s \n",__DATE__, __TIME__, __TIMESTAMP__);

  DEBUG_PRINTF("Debug printf statement with 4 variables: %s - %s and %s: %6.4f\n","ONE","TWO","PI",pi);
  DEBUG_PRINTF("Debug printf statement with 0 variables.\n");


//confirmed GCC does not support __func__ and __LINE_  .. no biggy!
}

void loop(){

}

