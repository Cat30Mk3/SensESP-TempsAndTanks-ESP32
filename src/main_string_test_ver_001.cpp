#include <Arduino.h>
#include <string>
using namespace std;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("*** STARTING SETUP - PRINT ALWAYS ***");

  String  testString=__FILENAME__;
  String  subString= testString.substring(4);
  String  shortString;
  //String version_str;
  shortString = testString;
  shortString.replace(".cpp","");
  //version_str =shortString;

  //int verPosn = shortString.lastIndexOf("ver_");
  //version_str=shortString.substring(verPosn);
  //shortString.remove(5);
  //String  shortString= testString.replace(".cpp","");

  Serial.println(testString);
  Serial.println(subString);
  Serial.println(shortString);
  //Serial.println(version_str);
  Serial.println("==========");
}


void loop(){
}
