
#pragma once
#include <Arduino.h>
#include <ESP32RotaryEncoder.h>

#include <N2kMsg.h>
#include <N2kTypes.h>
#include <U8g2lib.h>
#include <MUIU8g2.h>


#define PROGRAM_VERSION "Dev 14.1"


// Sensor globals
#define NUMBER_OF_TEMP_SENSORS 3
#define NUMBER_OF_TANK_SENSORS 4
#define NUMBER_OF_TACH_SENSORS 1
#define NUMBER_OF_TANK_DISPLAYS 5
#define NUMBER_OF_TEMP_DISPLAYS 1
#define NUMBER_OF_TACH_DISPLAYS 2
#define NUMBER_OF_DISPLAY_TYPES 5

#define NUMBER_OF_WEMA_STEPS 13

#define TANK_ALL_TYPE 0
#define TEMP_ALL_TYPE 1
#define TANK_INDV_TYPE 2
#define TEMP_INDV_TYPE 3
#define TACH_INDV_TYPE 4

#define TANK_QTY_PCNT 0
#define TANK_QTY_LTRS 1
#define TANK_QTY_STEP 2
#define TANK_QTY_OHMS 3
#define TANK_QTY_VOLT 4



// OLED display width and height, in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CHARACTER_HEIGHT 10

#define WIFI_SCROLLER_WINDOW 3
#define WIFI_SCROLLER_LIST_COUNT 10
#define WIFI_SCROLLER_INDEX_MAX \
  (WIFI_SCROLLER_LIST_COUNT - WIFI_SCROLLER_WINDOW)

#define ABOUT_LIST_COUNT 3


// SDA and SCL pins on SH-ESP32
#define SDA_PIN 16
#define SCL_PIN 17

#define SDA_2_PIN 26
#define SCL_2_PIN 25

#define DISPLAY_LOCAL_I2C_ADDRESS 0x3C
#define DISPLAY_REMOTE_I2C_ADDRESS 0x3D

// Encoder pins on SH-ESP32
#define ENC_A_VIOLET 21  // violet
#define ENC_B_GREY 18    // grey
#define PB_ENCODER 23    // yellow

// Tach Pin
#define NUMBER_OF_MAGNETS 1       //testbed has 2 - Avalon Engine has 1
#define HALL_EFFECT_SENSOR_PIN 35  // GPIO 35 via optical isolation input
                                   // OPT_IN1
#define TACH_BUFFER_LEN 1                                   
#define ENGINE_STOPPED_PERIOD_US 1000000  // (1000000 / 1)  uS per 1 revs == stopped at 60 rpm
#define ENGINE_MAX_RPM 4000

#define TACH_QTY_RPM 0
#define TACH_QTY_MS 1
#define TACH_AVG_MAX_COUNT 5

#define REFRESH 0
#define FORWARD 1
#define BACKWARD 2


#define LOCAL_ENCODER 0
#define REMOTE_ENCODER 1

#define LOCAL_ENC_A_VIOLET 18  // violet
#define LOCAL_ENC_B_GREY 21    // grey
#define LOCAL_PB_ENCODER 23    // yellow

#define REMOTE_ENC_A_VIOLET 5  // violet
#define REMOTE_ENC_B_GREY 19     // grey
#define REMOTE_PB_ENCODER 22    // yellow

#define TEST_LED_PIN 27

#define ENC_MAX_VALUE 1
#define ENC_MIN_VALUE -1

// #define HLD_TANK 3  //?? what is this for >> wrong value ????

struct tankParamStruct {
  bool enabled;
  char tankName[15];
  bool wemaTank = false;
  bool n2kEnabled;
  uint8_t displayEnabled;  // bit 0: percent / bit 1: litres / bit 2: step / bit
                           // 3: ohms / bit 4: volts /
  tN2kFluidType fluidType;
  float calUpperResistance;
  float calUpperVolts;
  float calLowerResistance;
  float calLowerVolts;
  unsigned char TankInstance;
  int adsChannel;
  float R0ohms;
  float R1ohms;
  float R2ohms;
  double volts2ohmsM = 0;
  double volts2ohmsB = 0;
  uint8_t step = 0;
  double voltsMeasured;
  double OhmsCalc;
  uint8_t WemaStep;
  double TankLevelPercent;
  double TankCapacity;
};

struct tempParamStruct {
  bool enabled = true;
  bool n2kEnabled = true;
  uint8_t displayEnabled;  // bit 0: degrees C
  char tempName[15];
  float tempReadingCelsius;
};

struct tachParamStruct {
  bool enabled = true;
  bool n2kEnabled = true;
  uint8_t displayEnabled;  // bit 0: degrees C
  char tachName[15];
  volatile bool engineRunning = false;
  volatile long periodMicroSeconds;
  volatile long lastMicroSeconds;
  volatile uint32_t tachReadingRpm;
  volatile uint32_t tachAverageRpm;

};

// **** G L O B A L S ****
// these are the declarations that the globals are external (ie #include
// <jwStructsAndGlobals.h> within file that needs access to the global)
// -- the actual global declarations are in  jwStructsAndGlobals.cpp  -- a neat
// trick to keep things tidy!


extern char main_filename[30];

// encoder and display switching globals
extern uint8_t startupDisplayAddress;
extern bool remoteNotLocal;

// menu globals
extern uint8_t exit_code;
extern bool testLedState;
// menu globals - calibration Form(15)
extern uint8_t selectedTank;
extern uint8_t selectedTemp;
extern uint8_t selectedTach;
extern uint8_t selectedPair;  // 0 == Lower, 1 == Upper
extern char curMvOhms_str[30];
extern char newMv_str[30];
extern uint8_t newOhms;
extern uint8_t newOhmsdec;
// menu globals - delete config files Form(32)
extern uint8_t selectFile2Delete;

//averaging globals
extern double averagingArray[TACH_AVG_MAX_COUNT];
extern bool averagingIntialized;
extern double averagingOutput;
extern int averagingIndex;

// ads globals
extern double VCCvolts;
extern double ICCma;
extern double WEMAresistanceSteps[NUMBER_OF_WEMA_STEPS];

// enum LiveDisplays { temp_3, tank_4};
extern int maxLiveDisplays;
extern int currentLiveDisplay;
extern bool menuIsActive;
extern int8_t WiFiScrollerIndex;

// new Live Display globals
extern int currentLiveDisplayType;
extern int currentLiveDisplaySensor;
extern int currentLiveDisplayQty;

extern boolean semaphore_encoder;

// display sleep globals
extern boolean displayIsAsleep;
extern uint8_t displaySleepDelay;  // ms
extern uint32_t displaySleepTimer;
extern uint32_t thisMsTime, lastMsTime;

extern uint8_t is_redraw;

extern volatile int selectedEncoder;

extern volatile bool turnedRightFlag;
extern volatile bool turnedLeftFlag;
extern volatile bool push_event;

// **** E X T E R N A L   G L O B A L   V A R I A B L E S  ****
extern struct tankParamStruct tankParam[NUMBER_OF_TANK_SENSORS];
extern struct tempParamStruct tempParam[NUMBER_OF_TEMP_SENSORS];
extern struct tachParamStruct tachParam[NUMBER_OF_TACH_SENSORS];

// **** G L O B A L    I N S T A N C E S  ****
extern U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2;
extern MUIU8G2 mui;
extern RotaryEncoder rotaryEncoder;
extern RotaryEncoder rotaryEncoderSelected[];
