
#include <memory>
#include <Arduino.h>

#include <U8g2lib.h>
#include <MUIU8g2.h>

#include <Adafruit_ADS1X15.h>
#include <ESP32RotaryEncoder.h>
#include <N2kMessages.h>
#include <NMEA2000_esp32.h>
#include <Wire.h>
#include <eh_analog.h>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_app_builder.h"
#include "sensesp_onewire/onewire_temperature.h"


#include "jwDebug.h"
#include "jwConfig.h"
#include "jwLiveDispays.h"
#include "jwMenuAndUI.h"
#include "jwStructsAndGlobals.h"


#define MAIN_FILENAME __FILE__  //later try __FILE_NAME__
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

  DEBUG_PRINTF("Scanning...\n");

  for (address = 1; address < 127; address++) {
    i2c->beginTransmission(address);
    error = i2c->endTransmission();

    if (error == 0)
      DEBUG_PRINTF("I2C device found at Address:0x%X\n", address);
    else if (error == 4)
      DEBUG_PRINTF("Unknown error at Address:0x%X\n", address);
  }
  DEBUG_PRINTF("done");
}

void IRAM_ATTR tachIsr() {
  static long testRawRpm;
  static long lastMicroSecond = 0;

  long thisMicroSecond = 0;
  static long thisPeriodMicroSeconds = 0;

  thisMicroSecond = micros();
  thisPeriodMicroSeconds = thisMicroSecond - lastMicroSecond;

  //if (thisPeriodMicroSeconds < 4000) return;  // high pass filter (noise?) potential lockout??
  if (thisPeriodMicroSeconds <
          ENGINE_STOPPED_PERIOD_US && /*check for stopped */
      thisPeriodMicroSeconds > 0)     /* dont divide by zero!*/
  {
    testRawRpm = ((60 * 1000000) / thisPeriodMicroSeconds) / NUMBER_OF_MAGNETS;
    if (testRawRpm < ENGINE_MAX_RPM) tachParam[0].tachReadingRpm = testRawRpm;
  } else { /*incase stopped */
    tachParam[0].tachReadingRpm = 0;
  }
  tachParam[0].lastMicroSeconds = thisMicroSecond;
  tachParam[0].periodMicroSeconds = thisPeriodMicroSeconds;
  lastMicroSecond = thisMicroSecond;

  static bool testLedState = false;
  testLedState = !testLedState;
  digitalWrite(TEST_LED_PIN, testLedState);
}

void checkEngineStopped(void) {
  long thisMicroSecond = micros();
  long periodMicroSeconds = thisMicroSecond - tachParam[0].lastMicroSeconds;
  if (periodMicroSeconds > ENGINE_STOPPED_PERIOD_US) {
    tachParam[0].engineRunning = false;
    tachParam[0].periodMicroSeconds = 0;
    tachParam[0].tachReadingRpm = 0;
  }
  //static bool testLedState = false;
  //testLedState = !testLedState;
  //digitalWrite(TEST_LED_PIN, testLedState);
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

void SendTankLevel(int index) {
  tN2kMsg N2kMsg;

  // DEBUG_PRINTF(">>from SendTankLevel [%d] %s Pcnt:%f%%%\n", index,
  // tankParam[index].tankName, tankParam[index].TankLevelPercent);

  SetN2kPGN127505(N2kMsg, tankParam[index].TankInstance,
                  tankParam[index].fluidType,  // tN2kFluidType FluidType,
                  tankParam[index].TankLevelPercent * 100.0,  // double Level,
                  tankParam[index].TankCapacity               // double Capacity
  );
  nmea2000->SendMsg(N2kMsg);
}

void SendEngineSpeed(int index) {
  tN2kMsg N2kMsg;

  DEBUG_PRINTF(">>from SendEngineSpeed [%d] %s RPM:%d\n", index,
                tachParam[index].tachName, tachParam[index].tachReadingRpm);

  SetN2kPGN127488(N2kMsg,
                  /*EngineInstance*/ 0,
                  /*EngineSpeed*/ tachParam[index].tachReadingRpm,
                  /* EngineBoostPressure */ N2kDoubleNA,
                  /* EngineTiltTrim */ N2kInt8NA);

  nmea2000->SendMsg(N2kMsg);
}

//**** S E T U P ******
void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

#ifndef DEBUG_DISABLE_JW
  Serial.begin(115200);
#endif
  DEBUG_PRINTF("*** STARTING SETUP ***\n");

  runListDir(/* delWMconfig */ false, /* deleteAllButJson */ false,
             /* delJson */ false);

  // while(true);

  pinMode(TEST_LED_PIN, OUTPUT);
  digitalWrite(TEST_LED_PIN, HIGH);

  pinMode(HALL_EFFECT_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(HALL_EFFECT_SENSOR_PIN, tachIsr, RISING);

  loadDefaultCommonParameters();
  loadDefaultTankParameters();
  loadDefaultTempParameters();
  loadDefaultTachParameters();
  updateVolts2OhmsParameters();

  // writeConfigFile();  //uncomment for config file change
  //  wipeConfigDB();     //uncomment for config file change

  loadConfigFile();
  updateVolts2OhmsParameters();
  dumpCommonParams();
  dumpTankParams();
  dumpTempParams();
  dumpTachParams();

  // delay(15000);
  // while(true);

  // initialize the I2C2 bus (discovered u2g2 and adafruiy_ads1115 libraries
  // conflict on i2c ???)
  i2c2 = new TwoWire(0);
  i2c2->begin(SDA_2_PIN, SCL_2_PIN);
  DEBUG_PRINTF("\nScanning %s on pins SDA:%d & SCL:%d ...\n", "i2c2",
                SDA_2_PIN, SCL_2_PIN);
  ScanI2C(i2c2);

  i2c = new TwoWire(1);
  i2c->begin(SDA_PIN, SCL_PIN);
  DEBUG_PRINTF("\nScanning %s on pins SDA:%d & SCL:%d ...\n", "i2c", SDA_PIN,
                SCL_PIN);
  ScanI2C(i2c);

  DEBUG_PRINTF("\nStartup Display Address:0x%X \n\n", startupDisplayAddress);

  if (remoteNotLocal) {
    if (CheckI2C(i2c, DISPLAY_REMOTE_I2C_ADDRESS)) {
      startupDisplayAddress = DISPLAY_REMOTE_I2C_ADDRESS;
      rotaryEncoder.setPins(REMOTE_ENC_A_VIOLET, REMOTE_ENC_B_GREY,
                            REMOTE_PB_ENCODER);
    } else {
      startupDisplayAddress = DISPLAY_LOCAL_I2C_ADDRESS;
      rotaryEncoder.setPins(LOCAL_ENC_A_VIOLET, LOCAL_ENC_B_GREY,
                            LOCAL_PB_ENCODER);
    };
  } else {
    if (CheckI2C(i2c, DISPLAY_LOCAL_I2C_ADDRESS)) {
      startupDisplayAddress = DISPLAY_LOCAL_I2C_ADDRESS;
      rotaryEncoder.setPins(LOCAL_ENC_A_VIOLET, LOCAL_ENC_B_GREY,
                            LOCAL_PB_ENCODER);
    } else {
      startupDisplayAddress = DISPLAY_REMOTE_I2C_ADDRESS;
      rotaryEncoder.setPins(REMOTE_ENC_A_VIOLET, REMOTE_ENC_B_GREY,
                            REMOTE_PB_ENCODER);
    };
  }

  rotaryEncoder.setEncoderType(EncoderType::FLOATING);  //FLOATING causes internl pull-ups to be used

  rotaryEncoder.setBoundaries(-1, 1, false);
  rotaryEncoder.onTurned(&knobCallback);
  rotaryEncoder.onPressed(&buttonCallback);
  rotaryEncoder.begin();

  // while(true);

  // Initialize ADS1115
  auto ads1115 = new Adafruit_ADS1115();
  ads1115->setGain(GAIN_ONE);
  bool ads_initialized = ads1115->begin(kADS1115Address, i2c2);
  debugD("ADS1115 initialized: %s", ads_initialized ? "Yes" : "No");

  // while(true)yield();  //diagnostic hold here

  // JW U8G2 Block
  u8g2.setI2CAddress(/*uint8_t adr*/ startupDisplayAddress *
                     2);  // 0x3C or 0x3D adr: I2C address multiplied with 2
                          // (the lowest bit must be zero)
  // u8g2.setI2CAddress(/*uint8_t adr*/ 0x3D * 2);  //0x7B or 0x7A adr: I2C
  // address multiplied with 2 (the lowest bit must be zero)

  u8g2.begin();
  char lineBuff[40];
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  sprintf(lineBuff, "U8G2 Initialized");
  DEBUG_PRINTF("%s\n",lineBuff);
  u8g2.drawStr(0, CHARACTER_HEIGHT, lineBuff);
  u8g2.sendBuffer();

  initializeMenu();

  displaySleepTimer = 0;
  lastMsTime = millis();

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("TempsAndTanks")->get_app();
  // builder.set_wifi("JohnAndLinda", "7054563991")->get_app();
  //  FloatProducer* ConnectTankSender(Adafruit_ADS1115* ads1115, int channel,
  //  String name)

  // Construct the global SensESPApp() object
  //   SensESPAppBuilder builder;
  //   sensesp_app = (&builder)
  //                     // Set a custom hostname for the app.
  //                     ->set_hostname("TempsAndTanks")
  //                     // Optionally, hard-code the WiFi and Signal K server
  //                     // settings. This is normally not needed.
  //                     ->set_wifi("JohnAndLinda", "7054563991")
  //                     //->set_sk_server("192.168.10.3", 80)
  //                     ->get_app();

  DallasTemperatureSensors *dts = new DallasTemperatureSensors(ONEWIRE_PIN);

  // define three 1-Wire temperature sensors that update every 1000 ms
  // and have specific web UI configuration paths

  auto main_engine_oil_temperature =
      new OneWireTemperature(dts, 1000, "/mainEngineOilTemp/oneWire");
  auto main_engine_coolant_temperature =
      new OneWireTemperature(dts, 1000, "/mainEngineCoolantTemp/oneWire");
  auto main_engine_exhaust_temperature =
      new OneWireTemperature(dts, 1000, "/mainEngineWetExhaustTemp/oneWire");

  // Connect the tank senders
  auto tank_0_volume =
      ConnectTankSender(ads1115, tankParam[0].adsChannel, tankParam[0].tankName,
                        tankParam[0].volts2ohmsM, tankParam[0].volts2ohmsB);
  auto tank_1_volume =
      ConnectTankSender(ads1115, tankParam[1].adsChannel, tankParam[1].tankName,
                        tankParam[1].volts2ohmsM, tankParam[1].volts2ohmsB);
  auto tank_2_volume =
      ConnectTankSender(ads1115, tankParam[2].adsChannel, tankParam[2].tankName,
                        tankParam[2].volts2ohmsM, tankParam[2].volts2ohmsB);
  auto tank_3_volume =
      ConnectTankSender(ads1115, tankParam[3].adsChannel, tankParam[3].tankName,
                        tankParam[3].volts2ohmsM, tankParam[3].volts2ohmsB);


  // auto tank_b_volume = ConnectTankSender(ads1115, 1, "B");
  // auto tank_c_volume = ConnectTankSender(ads1115, 2, "C");
  // auto tank_d_volume = ConnectTankSender(ads1115, 3, "D");

  // define metadata for sensors

  // tank_A_volume_metadata  defined in eh_analog ==> ConnectTankSender()

  auto main_engine_oil_temperature_metadata =
      new SKMetadata("K",                       // units
                     "Engine Oil Temperature",  // display name
                     "Engine Oil Temperature",  // description
                     "Oil Temperature",         // short name
                     10.                        // timeout, in seconds
      );
  auto main_engine_coolant_temperature_metadata =
      new SKMetadata("K",                           // units
                     "Engine Coolant Temperature",  // display name
                     "Engine Coolant Temperature",  // description
                     "Coolant Temperature",         // short name
                     10.                            // timeout, in seconds
      );
  auto main_engine_temperature_metadata =
      new SKMetadata("K",                   // units
                     "Engine Temperature",  // display name
                     "Engine Temperature",  // description
                     "Temperature",         // short name
                     10.                    // timeout, in seconds
      );
  auto main_engine_exhaust_temperature_metadata =
      new SKMetadata("K",                        // units
                     "Wet Exhaust Temperature",  // display name
                     "Wet Exhaust Temperature",  // description
                     "Exhaust Temperature",      // short name
                     10.                         // timeout, in seconds
      );

  // connect the sensors to Signal K output paths

  main_engine_oil_temperature->connect_to(new SKOutput<float>(
      "propulsion.main.oilTemperature", "/mainEngineOilTemp/skPath",
      main_engine_oil_temperature_metadata));
  main_engine_coolant_temperature->connect_to(new SKOutput<float>(
      "propulsion.main.coolantTemperature", "/mainEngineCoolantTemp/skPath",
      main_engine_coolant_temperature_metadata));
  // transmit coolant temperature as overall engine temperature as well
  main_engine_coolant_temperature->connect_to(new SKOutput<float>(
      "propulsion.main.temperature", "/mainEngineTemp/skPath",
      main_engine_temperature_metadata));
  // propulsion.*.wetExhaustTemperature is a non-standard path
  main_engine_exhaust_temperature->connect_to(
      new SKOutput<float>("propulsion.main.wetExhaustTemperature",
                          "/mainEngineWetExhaustTemp/skPath",
                          main_engine_exhaust_temperature_metadata));

  // JW U8G2 Block
  // char lineBuff[40];
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  sprintf(lineBuff, "Host: %s", sensesp_app->get_hostname().c_str());
  u8g2.drawStr(0, CHARACTER_HEIGHT, lineBuff);
  u8g2.sendBuffer();

  // Add display updaters for temperature values
  main_engine_oil_temperature->connect_to(
      new LambdaConsumer<float>([](float temperature) {
        tempParam[0].tempReadingCelsius = temperature - 273.15;
        oil_temperature = temperature;
        if (tempParam[0].n2kEnabled) SendEngineTemperatures();
      }));
  main_engine_coolant_temperature->connect_to(
      new LambdaConsumer<float>([](float temperature) {
        tempParam[1].tempReadingCelsius = temperature - 273.15;
        coolant_temperature = temperature;
        if (tempParam[1].n2kEnabled) SendEngineTemperatures();
      }));
  main_engine_exhaust_temperature->connect_to(
      new LambdaConsumer<float>([](float temperature) {
        tempParam[2].tempReadingCelsius = temperature - 273.15;
        if (tempParam[2].n2kEnabled) {
          tN2kMsg N2kMsg;
          SetN2kTemperature(N2kMsg,
                            1,                            // SID
                            2,                            // TempInstance
                            N2kts_ExhaustGasTemperature,  // TempSource
                            temperature                   // actual temperature
          );
          nmea2000->SendMsg(N2kMsg);
        };
      }));
  // Add display updaters for tank values
  tank_0_volume->connect_to(new LambdaConsumer<float>([](float value) {
    tankParam[0].TankLevelPercent = value;
    if (tankParam[0].n2kEnabled) SendTankLevel(0);
  }));
  tank_1_volume->connect_to(new LambdaConsumer<float>([](float value) {
    tankParam[1].TankLevelPercent = value;
    if (tankParam[1].n2kEnabled) SendTankLevel(1);
  }));
  tank_2_volume->connect_to(new LambdaConsumer<float>([](float value) {
    tankParam[2].TankLevelPercent = value;
    if (tankParam[2].n2kEnabled) SendTankLevel(2);
  }));
  tank_3_volume->connect_to(new LambdaConsumer<float>([](float value) {
    tankParam[3].TankLevelPercent = value;
    if (tankParam[3].n2kEnabled) SendTankLevel(3);
  }));


// **** Tach connection to NMEA 200 is missing !! ****
  // tach_0_rpm->connect_to(new LambdaConsumer<float>([](float value) {
  //    tachParam[0].tachReadingRpm = value;
  //    if (tachParam[0].n2kEnabled) SendEngineSpeed(0);
  //  }));

  // initialize the NMEA 2000 subsystem

  // instantiate the NMEA2000 object
  nmea2000 = new tNMEA2000_esp32(CAN_TX_PIN, CAN_RX_PIN);

  // Reserve enough buffer for sending all messages. This does not work on
  // small memory devices like Uno or Mega
  nmea2000->SetN2kCANSendFrameBufSize(250);
  nmea2000->SetN2kCANReceiveFrameBufSize(250);

  // Set Product information
  nmea2000->SetProductInformation(
      "20240617",  // Manufacturer's Model serial code (max 32 chars)
      103,         // Manufacturer's product code
      "SH-ESP32 Sensor Interface",  // Manufacturer's Model ID (max 33 chars)
      "0.8.0.0 (2024-06-17)",  // Manufacturer's Software version code (max 40
                               // chars)
      "1.0.0.0 (2024-06-17)"   // Manufacturer's Model version (max 24 chars)
  );
  // Set device information
  nmea2000->SetDeviceInformation(
      1,    // Unique number. Use e.g. Serial number.
      130,  // Device function=Analog to NMEA 2000 Gateway. See codes on
            // http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
      75,   // Device class=Inter/Intranetwork Device. See codes on
           // http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
      2046  // Just choosen free from code list on
            // http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
  );

  nmea2000->SetMode(tNMEA2000::N2km_NodeOnly, 22);
  // Disable all msg forwarding to USB (=Serial)
  nmea2000->EnableForward(false);
  nmea2000->Open();

  // No need to parse the messages at every single loop iteration; 1 ms will
  // do

  app.onRepeat(1, []() { nmea2000->ParseMessages(); });
  app.onRepeatMicros(10000, []() { handle_events(); });
  app.onRepeatMicros(1000000, []() {  //changed from 500,000
    checkEngineStopped();
    newLiveDisplayBuilder();
  });

  // app.onRepeat(1000, []() {
  //   checkEngineStopped();
  //   // if (tankParam[0].n2kEnabled) {
  //   //   tN2kMsg N2kMsg;
  //   //   SendEngineSpeed(0);
  //   //   nmea2000->SendMsg(N2kMsg);
  //   // };  // once every second
  // });

  sensesp_app->start();
}
// main program loop

void loop(void) { app.tick(); }