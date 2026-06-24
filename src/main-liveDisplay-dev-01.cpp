#include <Arduino.h>

// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1X15.h>
#include <ESP32RotaryEncoder.h>
#include <N2kMessages.h>
#include <NMEA2000_esp32.h>

#include <Wire.h>
#include <eh_analog.h>

#include <U8g2lib.h>
#include <MUIU8g2.h>


#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"
#include "sensesp_onewire/onewire_temperature.h"

#include "jwConfig.h"
#include "jwLiveDispays.h"
#include "jwMenuAndUI.h"
#include "jwStructsAndGlobals.h"
// 1-Wire data pin on SH-ESP32
#define ONEWIRE_PIN 4

// CAN bus (NMEA 2000) pins on SH-ESP32
#define CAN_RX_PIN GPIO_NUM_34
#define CAN_TX_PIN GPIO_NUM_32

// define temperature display units

// #define TEMP_DISPLAY_FUNC KelvinToFahrenheit

using namespace sensesp;
#define SERIAL_DEBUG_DISABLED

// ADS1115 I2C address
const int kADS1115Address = 0x48;

reactesp::ReactESP app;
// ReactESP app;

// TwoWire* i2c;
// Adafruit_SSD1306* display;
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, /*
// reset=*/U8X8_PIN_NONE);
tNMEA2000 *nmea2000;

TwoWire *i2c2;
TwoWire *i2c;

// enum tN2kFluidType {
//                             N2kft_Fuel=0,             ///< fluid type is fuel
//                             N2kft_Water=1,            ///< fluid type is
//                             water N2kft_GrayWater=2,        ///< fluid type
//                             is gray water N2kft_LiveWell=3,         ///<
//                             fluid type is live well N2kft_Oil=4, ///< fluid
//                             type is oil N2kft_BlackWater=5,       ///< fluid
//                             type is black water N2kft_FuelGasoline=6, ///<
//                             fluid type is gasoline fuel N2kft_Error=14, ///<
//                             error occurred N2kft_Unavailable=15      ///<
//                             unavailable
//                           };

// float KelvinToCelsius(float temp) { return temp - 273.15; }
// #define TEMP_DISPLAY_FUNC KelvinToCelsius

// float KelvinToFahrenheit(float temp) { return (temp - 273.15) * 9. / 5.
// + 32.; }

double oil_temperature = N2kDoubleNA;
double coolant_temperature = N2kDoubleNA;

bool CheckI2C(TwoWire *i2c, uint8_t address) {
  uint8_t error;
  i2c->beginTransmission(address);
  error = i2c->endTransmission();
  if (error != 0)
    return false;
  else
    return true;
}

// Convenience function to print the addresses found on the I2C bus
void ScanI2C(TwoWire *i2c) {
  uint8_t error, address;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    i2c->beginTransmission(address);
    error = i2c->endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("");
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  Serial.println("done");
}
/**
 * @brief Send Engine Dynamic Parameter data
 *
 * Send engine temperature data using the Engine Dynamic Parameter PGN.
 * All unused fields that are sent with undefined value except the status
 * bit fields are sent as zero. Hopefully we're not resetting anybody's engine
 * warnings...
 */
void SendEngineTemperatures() {
  tN2kMsg N2kMsg;
  SetN2kEngineDynamicParam(N2kMsg,
                           0,  // instance of a single engine is always 0
                           N2kDoubleNA,  // oil pressure
                           oil_temperature, coolant_temperature,
                           N2kDoubleNA,  // alternator voltage
                           N2kDoubleNA,  // fuel rate
                           N2kDoubleNA,  // engine hours
                           N2kDoubleNA,  // engine coolant pressure
                           N2kDoubleNA,  // engine fuel pressure
                           N2kInt8NA,    // engine load
                           N2kInt8NA,    // engine torque
                           (tN2kEngineDiscreteStatus1)0,
                           (tN2kEngineDiscreteStatus2)0);
  nmea2000->SendMsg(N2kMsg);
}

// tank parameters
// double tank_0_level_percent = N2kDoubleNA;
// double tank_0_capacity_litres = N2kDoubleNA;

// unsigned char TankInstance = N2kUInt8NA;
// double TankLevelPercent = N2kDoubleNA;
// double TankCapacity = N2kDoubleNA;

/* void SendTankLevel()
{
    tN2kMsg N2kMsg;

Serial.printf(">>from SendTankLevel T:Pcnt:%f\n",TankLevelPercent);

    SetN2kPGN127505(N2kMsg,
                    TankInstance,
                    N2kft_BlackWater,  // tN2kFluidType FluidType,
                    TankLevelPercent*100.0, // double Level,
                    TankCapacity      // double Capacity
    );
    nmea2000->SendMsg(N2kMsg);
} */

void SendTankLevel(int index) {
  tN2kMsg N2kMsg;

  // Serial.printf(">>from SendTankLevel [%d] %s Pcnt:%f%%%\n", index,
  // tankParam[index].tankName, tankParam[index].TankLevelPercent);

  SetN2kPGN127505(N2kMsg, tankParam[index].TankInstance,
                  tankParam[index].fluidType,  // tN2kFluidType FluidType,
                  tankParam[index].TankLevelPercent * 100.0,  // double Level,
                  tankParam[index].TankCapacity               // double Capacity
  );
  nmea2000->SendMsg(N2kMsg);
}

// dev todos:
// (1)
//

int isNthBitSet(unsigned char c, int n) {
  // static unsigned char mask[] = {128, 64, 32, 16, 8, 4, 2, 1};
  static unsigned char mask[] = {1, 2, 4, 8, 16, 32, 64, 128};
  return ((c & mask[n]) != 0);
}

void testFindingBits(void) {
  Serial.println("\n\n** FINDING BITS IN tankParam[0].displayEnabled byte **");
  for (int bitPosn = 0; bitPosn < 8; bitPosn++) {
    int checkBit = isNthBitSet(tankParam[0].displayEnabled, bitPosn);
    Serial.printf("[bit=%d,value=%d]\n", bitPosn, checkBit);
  }
  Serial.println();
}

void dumpAllDisplayList(void) {
  int checkBit;
  int displayTypeSensorCount[NUMBER_OF_DISPLAY_TYPES],
      displayCount[NUMBER_OF_DISPLAY_TYPES];
  displayTypeSensorCount[TANK_ALL_TYPE] = 1;
  displayTypeSensorCount[TEMP_ALL_TYPE] = 1;
  displayTypeSensorCount[TANK_INDV_TYPE] = NUMBER_OF_TANK_SENSORS;
  displayTypeSensorCount[TEMP_INDV_TYPE] = NUMBER_OF_TEMP_SENSORS;
  displayTypeSensorCount[TACH_INDV_TYPE] = 1;
  displayCount[TANK_ALL_TYPE] = 1;
  displayCount[TEMP_ALL_TYPE] = 1;
  displayCount[TANK_INDV_TYPE] = NUMBER_OF_TANK_DISPLAYS;
  displayCount[TEMP_INDV_TYPE] = NUMBER_OF_TEMP_DISPLAYS;
  displayCount[TACH_INDV_TYPE] = NUMBER_OF_TACH_DISPLAYS;

  Serial.println("\n\n** DUMPING ALL DISPLAY LIST **");

  for (int displayTypeIndex = 0; displayTypeIndex < NUMBER_OF_DISPLAY_TYPES;
       displayTypeIndex++) {
    for (int sensorIndex = 0;
         sensorIndex < displayTypeSensorCount[displayTypeIndex];
         sensorIndex++) {
      for (int displayIndex = 0; displayIndex < displayCount[displayTypeIndex];
           displayIndex++) {
        switch (displayTypeIndex) {
          case TANK_INDV_TYPE:
            checkBit = isNthBitSet(tankParam[sensorIndex].displayEnabled,
                                   displayIndex);
            break;
          case TEMP_INDV_TYPE:
            checkBit = isNthBitSet(tempParam[sensorIndex].displayEnabled,
                                   displayIndex);
            break;
          default:
            checkBit = 1;
        }
        Serial.printf("[type=%d,sensor=%d,display=%d,flag=%d]\n",
                      displayTypeIndex, sensorIndex, displayIndex, checkBit);
      }
    }
  }
  Serial.println();
}

void moveLiveDisplay(bool forward, int *displayTypeIndex, int *sensorIndex,
          int *displayIndex) {
  // addresses are being passed - values are being processed
  int checkBit;
  int displayTypeSensorCount[NUMBER_OF_DISPLAY_TYPES],
      displayCount[NUMBER_OF_DISPLAY_TYPES];
  displayTypeSensorCount[TANK_ALL_TYPE] = 1;
  displayTypeSensorCount[TEMP_ALL_TYPE] = 1;
  displayTypeSensorCount[TANK_INDV_TYPE] = NUMBER_OF_TANK_SENSORS;
  displayTypeSensorCount[TEMP_INDV_TYPE] = NUMBER_OF_TEMP_SENSORS;
  displayTypeSensorCount[TACH_INDV_TYPE] = 1;
  displayCount[TANK_ALL_TYPE] = 1;
  displayCount[TEMP_ALL_TYPE] = 1;
  displayCount[TANK_INDV_TYPE] = NUMBER_OF_TANK_DISPLAYS;
  displayCount[TEMP_INDV_TYPE] = NUMBER_OF_TEMP_DISPLAYS;
  displayCount[TACH_INDV_TYPE] = NUMBER_OF_TACH_DISPLAYS;

  checkBit = 0;
  if (forward) {
    while (checkBit == 0) {
      (*displayIndex)++;
      if ((*displayIndex) >= displayCount[(*displayTypeIndex)]) {
        (*displayIndex) = 0;
        (*sensorIndex)++;
        if ((*sensorIndex) >= displayTypeSensorCount[*displayTypeIndex]) {
          (*sensorIndex) = 0;
          (*displayTypeIndex)++;
          if ((*displayTypeIndex) >= NUMBER_OF_DISPLAY_TYPES) {
            (*displayTypeIndex) = 0;
          }
        }
      }
      if ((*displayTypeIndex) == TEMP_INDV_TYPE)
        checkBit =
            isNthBitSet(tempParam[*sensorIndex].displayEnabled, *displayIndex);
      else if ((*displayTypeIndex) == TANK_INDV_TYPE)
        checkBit =
            isNthBitSet(tankParam[*sensorIndex].displayEnabled, *displayIndex);
      else
        checkBit = 1;
    }
  } else {  // backward
    while (checkBit == 0) {
      (*displayIndex)--;          // next lower sensor
      if ((*displayIndex) < 0) {  // leaving lowest display
        (*sensorIndex)--;         // next lower sensor
        (*displayIndex) =
            displayCount[(*displayTypeIndex)] - 1;  // highest disp of same type
        if ((*sensorIndex) < 0) {                   // leaving lowest sensor
          (*displayTypeIndex)--;                    // next lower type
          (*sensorIndex) = displayTypeSensorCount[*displayTypeIndex] - 1;  // highest sensor of next lower type
          (*displayIndex) = displayCount[(*displayTypeIndex)] - 1;         // highest display of next lower type
          if ((*displayTypeIndex) < 0) {  // leaving lowest type
            (*displayTypeIndex) = NUMBER_OF_DISPLAY_TYPES - 1;  // highest type
            (*sensorIndex) = displayTypeSensorCount[*displayTypeIndex] - 1;  // highest sensor of highest type
            (*displayIndex) = displayCount[(*displayTypeIndex)] -  1;  // highest display of highest type
          }
        }
      }
      if ((*displayTypeIndex) == TEMP_INDV_TYPE)
        checkBit =
            isNthBitSet(tempParam[*sensorIndex].displayEnabled, *displayIndex);
      else if ((*displayTypeIndex) == TANK_INDV_TYPE)
        checkBit =
            isNthBitSet(tankParam[*sensorIndex].displayEnabled, *displayIndex);
      else
        checkBit = 1;
    }
  }

  Serial.printf("[type=%d,sensor=%d,display=%d,flag=%d]\n", *displayTypeIndex,
                *sensorIndex, *displayIndex, checkBit);
}

void forward(int *displayTypeIndex, int *sensorIndex, int *displayIndex) {
  // addresses are being passed - values are being processed
  int checkBit;
  int displayTypeSensorCount[NUMBER_OF_DISPLAY_TYPES],
      displayCount[NUMBER_OF_DISPLAY_TYPES];
  displayTypeSensorCount[TANK_ALL_TYPE] = 1;
  displayTypeSensorCount[TEMP_ALL_TYPE] = 1;
  displayTypeSensorCount[TANK_INDV_TYPE] = NUMBER_OF_TANK_SENSORS;
  displayTypeSensorCount[TEMP_INDV_TYPE] = NUMBER_OF_TEMP_SENSORS;
  displayTypeSensorCount[TACH_INDV_TYPE] = 1;
  displayCount[TANK_ALL_TYPE] = 1;
  displayCount[TEMP_ALL_TYPE] = 1;
  displayCount[TANK_INDV_TYPE] = NUMBER_OF_TANK_DISPLAYS;
  displayCount[TEMP_INDV_TYPE] = NUMBER_OF_TEMP_DISPLAYS;
  displayCount[TACH_INDV_TYPE] = NUMBER_OF_TACH_DISPLAYS;

  checkBit = 0;
  while (checkBit == 0) {
    (*displayIndex)++;
    if ((*displayIndex) >= displayCount[(*displayTypeIndex)]) {
      (*displayIndex) = 0;
      (*sensorIndex)++;
      if ((*sensorIndex) >= displayTypeSensorCount[*displayTypeIndex]) {
        (*sensorIndex) = 0;
        (*displayTypeIndex)++;
        if ((*displayTypeIndex) >= NUMBER_OF_DISPLAY_TYPES) {
          (*displayTypeIndex) = 0;
        }
      }
    }
    if ((*displayTypeIndex) == TEMP_INDV_TYPE)
      checkBit =
          isNthBitSet(tempParam[*sensorIndex].displayEnabled, *displayIndex);
    else if ((*displayTypeIndex) == TANK_INDV_TYPE)
      checkBit =
          isNthBitSet(tankParam[*sensorIndex].displayEnabled, *displayIndex);
    else
      checkBit = 1;
  }
  Serial.printf("[type=%d,sensor=%d,display=%d,flag=%d]\n", *displayTypeIndex,
                *sensorIndex, *displayIndex, checkBit);
}

void walkLiveDisplay(void) {
  int type = 4;
  int sensor = 0;
  int display = 0;

  Serial.println("\n\n** WALKING FORWARD TWICE **");

  for (int counter = 0; counter < 50; counter++) {
    // forward(&type, &sensor, &display);  // addresses are being passed
    moveLiveDisplay(false, &type, &sensor, &display);  // addresses are being passed
  }

  Serial.println();
}

//**** S E T U P ******
void setup() {
  Serial.begin(115200);
  Serial.println("*** STARTING SETUP ***");

  // list files stored in ESP32 SPIFFS
  // runListDir(/* delWMconfig */ false, /* deleteAllButJson */ false,
  //           /* delJson */ false);

  // pinMode(TEST_LED_PIN, OUTPUT);
  // digitalWrite(TEST_LED_PIN, HIGH);

  // pinMode(PB_PREV_BLUE, INPUT);
  // pinMode(PB_NEXT_RED, INPUT);
  // pinMode(PB_SEL_BLACK, INPUT);
  // pinMode(PB_ENCODER, INPUT);
  // rotary.setTrigger(LOW);

  loadDefaultTankParameters();   //"factory settings " defines in jwconfig.h
  loadDefaultTempParameters();   //"factory settings " defines in jwconfig.h
  updateVolts2OhmsParameters();  // calculate and update tankParam[].M & B
                                 // values for each tank from calUpper* and
                                 // calLower*
  // dumpTankParams();
  // writeConfigFile();
  // wipeConfigDB();
  // loadDefaultTempParameters();
  // loadConfigFile();
  // updateVolts2OhmsParameters();  // calculate and update tankParam[].M & B
  // values for each tank from calUpper* and
  // calLower*
  dumpCommonParams();
  dumpTankParams();
  dumpTempParams();

  // dumpAllDisplayList();

  // testFindingBits();

  walkLiveDisplay();
}

void loop(void) {}
