#include "jwStructsAndGlobals.h"

struct tankParamStruct tankParam[NUMBER_OF_TANK_SENSORS];
struct tempParamStruct tempParam[NUMBER_OF_TEMP_SENSORS];
struct tachParamStruct tachParam[NUMBER_OF_TACH_SENSORS];


// **** G L O B A L S ****
// these are the actual global declarations (in some cases they have optional initializations too) 
// -- the declarations that these globals are external are in jwStructsAndGlobals.h
//   (ie #include <jwStructsAndGlobals.h> within file that needs access to the global)
// -- a neat trick to keep things tidy!

char main_filename[30];


// encoder and display switching globals
//uint8_t startupDisplayAddress = DISPLAY_REMOTE_I2C_ADDRESS; 
uint8_t startupDisplayAddress = DISPLAY_LOCAL_I2C_ADDRESS;  
bool remoteNotLocal = false;

// display sleep  globals
boolean displayIsAsleep = false;
uint8_t displaySleepDelay = 10; // 10minutes 
uint32_t displaySleepTimer = 0;
uint32_t thisMsTime, lastMsTime;

// menu globals
uint8_t exit_code = 0;    // return value from the menu system
bool testLedState = false;
//menu globals - calibration Form(15)
uint8_t selectedTank = 0;
uint8_t selectedTemp = 0;
uint8_t selectedTach = 0;
uint8_t selectedPair = 0;  // 0 == Lower, 1 == Upper
char curMvOhms_str[30];
char newMv_str[30];
uint8_t newOhms;
uint8_t newOhmsdec;
//menu globals - delete config files Form(32)
uint8_t selectFile2Delete = 0;


//averaging globals
double averagingArray[TACH_AVG_MAX_COUNT];
double averagingOutput = 0;
bool averagingIntialized = false;
int averagingIndex = 0;


//ads globals
double VCCvolts = 3.3;
double ICCma = 6.30; 
double WEMAresistanceSteps[NUMBER_OF_WEMA_STEPS];

 
uint8_t is_redraw = 1;

int maxLiveDisplays = 19;
int currentLiveDisplay = 0;
bool menuIsActive = false;
int8_t WiFiScrollerIndex = 0;

//new Live Display globals
int currentLiveDisplayType = 0;
int currentLiveDisplaySensor = 0;
int currentLiveDisplayQty = 0;
boolean semaphore_encoder = false;


volatile bool turnedRightFlag = false;
volatile bool turnedLeftFlag = false;
volatile bool push_event = false;// global tach variables


#define LOCAL_ENCODER 0
#define REMOTE_ENCODER 1
volatile int selectedEncoder = LOCAL_ENCODER;

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, /* reset=*/U8X8_PIN_NONE);
MUIU8G2 mui;
//RotaryEncoder rotaryEncoder(LOCAL_ENC_A_VIOLET, LOCAL_ENC_B_GREY, LOCAL_PB_ENCODER, /*DO_ENCODER_VCC*/ -1);
//RotaryEncoder rotaryEncoderLocal(LOCAL_ENC_A_VIOLET, LOCAL_ENC_B_GREY, LOCAL_PB_ENCODER, /*DO_ENCODER_VCC*/ -1);
//RotaryEncoder rotaryEncoderRemote(REMOTE_ENC_A_VIOLET, REMOTE_ENC_B_GREY, REMOTE_PB_ENCODER, /*DO_ENCODER_VCC*/ -1);
RotaryEncoder rotaryEncoderSelected[]{{LOCAL_ENC_A_VIOLET, LOCAL_ENC_B_GREY, LOCAL_PB_ENCODER, /*DO_ENCODER_VCC*/ -1},
                                 {REMOTE_ENC_A_VIOLET, REMOTE_ENC_B_GREY, REMOTE_PB_ENCODER, /*DO_ENCODER_VCC*/ -1}};
  