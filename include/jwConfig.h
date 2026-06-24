#pragma once
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "jwStructsAndGlobals.h"
#include "jwDebug.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED false

// define filename to store config file
const String config_filename = "/jwConfig.json";

// initialize the variables of the program
int value1 = 0;
int value2 = 0;
String user_string = "";

void updateVolts2OhmsParameters(void) {
  for (int tankIndex = 0; tankIndex < NUMBER_OF_TANK_SENSORS; tankIndex++) {
    double Xupper = tankParam[tankIndex].calUpperVolts;
    double Yupper = tankParam[tankIndex].calUpperResistance;
    double Xlower = tankParam[tankIndex].calLowerVolts;
    double Ylower = tankParam[tankIndex].calLowerResistance;

    double M = (Yupper - Ylower) / (Xupper - Xlower);  // M = (rise/run)
    double B = Yupper - (M * Xupper);                  // b=y-Mx

    tankParam[tankIndex].volts2ohmsM = M;
    tankParam[tankIndex].volts2ohmsB = B;
  }
}

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
//
void loadDefaultTempParameters(void) {
  tempParam[0].enabled = true;
  tempParam[0].n2kEnabled = true;
  tempParam[0].displayEnabled = 0b00000001;  // bit 0: degrees C
  strcpy(tempParam[0].tempName, "Oil");

  tempParam[1].enabled = true;
  tempParam[1].n2kEnabled = true;
  tempParam[1].displayEnabled = 0b00000001;  // bit 0: degrees C
  strcpy(tempParam[1].tempName, "Coolant");

  tempParam[2].enabled = true;
  tempParam[2].n2kEnabled = true;
  tempParam[2].displayEnabled = 0b00000001;  // bit 0: degrees C
  strcpy(tempParam[2].tempName, "Exhaust");
}

void loadDefaultTachParameters(void) {
  tachParam[0].enabled = true;
  tachParam[0].n2kEnabled = true;
  tachParam[0].displayEnabled = 0b00000011;  // bit 0: rpm   bit 1: ms
  strcpy(tachParam[0].tachName, "Tachometer");
  // tachParam[0].tachReadingRpm = 3600;  //just for display testing
}

void loadDefaultCommonParameters(void) {
  remoteNotLocal = false;
  startupDisplayAddress = DISPLAY_LOCAL_I2C_ADDRESS;
  displaySleepDelay = 10;  // 10minutes
}

void loadDefaultTankParameters(void) {
  // ** SIMULATOR VALUES **
  // WEMAresistanceSteps[0] = 238.7;
  // WEMAresistanceSteps[1] = 189.9;
  // WEMAresistanceSteps[2] = 140.0;
  // WEMAresistanceSteps[3] = 131.6;
  // WEMAresistanceSteps[4] = 122.7;
  // WEMAresistanceSteps[5] = 113.9;
  // WEMAresistanceSteps[6] = 104.1;
  // WEMAresistanceSteps[7] = 91.2;
  // WEMAresistanceSteps[8] = 78.5;
  // WEMAresistanceSteps[9] = 65.6;
  // WEMAresistanceSteps[10] = 52.8;
  // WEMAresistanceSteps[11] = 42.9;
  // WEMAresistanceSteps[12] = 33.2;

  // ** GUAGE VALUES **
  WEMAresistanceSteps[0] = 239.7;
  WEMAresistanceSteps[1] = 189.1;
  WEMAresistanceSteps[2] = 142.1;
  WEMAresistanceSteps[3] = 133.1;
  WEMAresistanceSteps[4] = 124.0;
  WEMAresistanceSteps[5] = 115.0;
  WEMAresistanceSteps[6] = 105.1;
  WEMAresistanceSteps[7] = 92.1;
  WEMAresistanceSteps[8] = 79.0;
  WEMAresistanceSteps[9] = 66.1;
  WEMAresistanceSteps[10] = 53.1;
  WEMAresistanceSteps[11] = 43.1;
  WEMAresistanceSteps[12] = 33.1;



  tankParam[0].enabled = true;
  strcpy(tankParam[0].tankName, "BlackWema");
  tankParam[0].wemaTank = true;
  tankParam[0].n2kEnabled = true;
  tankParam[0].displayEnabled =
      0b00011111;  // bit 0: percent / bit 1: litres / bit 2: step / bit 3: ohms
                   // / bit 4: volts /
  tankParam[0].fluidType = N2kft_BlackWater;
  tankParam[0].calUpperResistance = 33.0;
  tankParam[0].calUpperVolts = 0.152;
  tankParam[0].calLowerResistance = 239.5;
  tankParam[0].calLowerVolts = 1.057;
  tankParam[0].R0ohms = 99.6;
  tankParam[0].R1ohms = 20000;
  tankParam[0].R2ohms = 51000;
  tankParam[0].TankInstance = 0;
  tankParam[0].adsChannel = 0;
  tankParam[0].TankCapacity = 64;
  tankParam[0].WemaStep = 99;
  tankParam[0].TankLevelPercent = N2kFloatNA;

  tankParam[1].enabled = true;
  strcpy(tankParam[1].tankName, "BlackAcu");
  tankParam[1].wemaTank = false;
  tankParam[1].n2kEnabled = true;
  tankParam[1].displayEnabled =
      0b00011011;  // bit 0: percent / bit 1: litres / bit 2: step / bit 3: ohms
                   // / bit 4: volts /
  tankParam[1].fluidType = N2kft_BlackWater;
  tankParam[1].calUpperResistance = 33.0;
  tankParam[1].calUpperVolts = 0.302;
  tankParam[1].calLowerResistance = 230.0;
  tankParam[1].calLowerVolts = 0.017;
  tankParam[1].R0ohms = 99.6;
  tankParam[1].R1ohms = 51000;
  tankParam[1].R2ohms = 20000;
  tankParam[1].TankInstance = 1;
  tankParam[1].adsChannel = 1;
  tankParam[1].TankCapacity = 60;
  tankParam[1].WemaStep = 99;
  tankParam[1].TankLevelPercent = N2kFloatNA;

  tankParam[2].enabled = true;
  strcpy(tankParam[2].tankName, "FreshFwd");
  tankParam[2].wemaTank = false;
  tankParam[2].n2kEnabled = true;
  tankParam[2].displayEnabled =
      0b00011011;  // bit 0: percent / bit 1: litres / bit 2: step / bit 3: ohms
                   // / bit 4: volts /
  tankParam[2].fluidType = N2kft_Water;
  tankParam[2].calUpperResistance = 33.0;
  tankParam[2].calUpperVolts = 0.420;
  tankParam[2].calLowerResistance = 230.0;
  tankParam[2].calLowerVolts = 0.001;
  tankParam[2].R0ohms = 99.6;
  tankParam[2].R1ohms = 51000;
  tankParam[2].R2ohms = 20000;
  tankParam[2].TankInstance = 0;
  tankParam[2].adsChannel = 2;
  tankParam[2].TankCapacity = 90;
  tankParam[2].WemaStep = 99;
  tankParam[2].TankLevelPercent = N2kFloatNA;

  tankParam[3].enabled = true;
  strcpy(tankParam[3].tankName, "FreshStbd");
  tankParam[3].wemaTank = false;
  tankParam[3].n2kEnabled = true;
  tankParam[3].displayEnabled =
      0b00011011;  // bit 0: percent / bit 1: litres / bit 2: step / bit 3: ohms
                   // / bit 4: volts /
  tankParam[3].fluidType = N2kft_Water;
  tankParam[3].calUpperResistance = 33.0;
  tankParam[3].calUpperVolts = 0.245;
  tankParam[3].calLowerResistance = 230.0;
  tankParam[3].calLowerVolts = 0.009;
  tankParam[3].R0ohms = 99.6;
  tankParam[3].R1ohms = 51000;
  tankParam[3].R2ohms = 20000;
  tankParam[3].TankInstance = 1;
  tankParam[3].adsChannel = 3;
  tankParam[3].TankCapacity = 60;
  tankParam[3].WemaStep = 99;
  tankParam[3].TankLevelPercent = N2kFloatNA;
}

void dumpTankParams(void) {
  DEBUG_PRINTF("\n[dumpTankParams]:\n");
  for (int tankIndex = 0; tankIndex < NUMBER_OF_TANK_SENSORS; tankIndex++) {
    DEBUG_PRINTF(
        "tank[%d] %s %s %s \n", tankIndex,
        tankParam[tankIndex].enabled ? "enabled " : "disabled",
        tankParam[tankIndex].wemaTank ? "WEMA " : "ACCU",
        tankParam[tankIndex].n2kEnabled ? "n2k_enabled " : "n2k_disabled");
    DEBUG_PRINTF("tank[%d] tankName: %s\n", tankIndex,
                 tankParam[tankIndex].tankName);
    DEBUG_PRINTF("tank[%d] displayEnabled 0x%0X\n", tankIndex,
                 tankParam[tankIndex].displayEnabled);                                         
    DEBUG_PRINTF("tank[%d] fluidType: %d", tankIndex,
                 tankParam[tankIndex].fluidType);
    DEBUG_PRINTF("calUpperResistance: %f\n", tankIndex,
                 tankParam[tankIndex].calUpperResistance);
    DEBUG_PRINTF("tank[%d] calUpperVolts: %f\n", tankIndex,
                 tankParam[tankIndex].calUpperVolts);
    DEBUG_PRINTF("tank[%d] calLowerResistance: %f\n", tankIndex,
                 tankParam[tankIndex].calLowerResistance);
    DEBUG_PRINTF("tank[%d] calLowerVolts: %f\n", tankIndex,
                 tankParam[tankIndex].calLowerVolts);
    DEBUG_PRINTF("tank[%d] volts2ohmsM: %d\n", tankIndex,
                 tankParam[tankIndex].volts2ohmsM);
    DEBUG_PRINTF("tank[%d] volts2ohmsB: %d\n", tankIndex,
                 tankParam[tankIndex].volts2ohmsB);
    DEBUG_PRINTF("tank[%d] TankInstance: %d\n", tankIndex,
                 tankParam[tankIndex].TankInstance);
    DEBUG_PRINTF("tank[%d] adsChannel: %d\n", tankIndex,
                 tankParam[tankIndex].adsChannel);
    DEBUG_PRINTF("tank[%d] TankCapacity: %d\n", tankIndex,
                 tankParam[tankIndex].TankCapacity);
    DEBUG_PRINTF("tank[%d] TankLevelPercent: %d\n", tankIndex,
                 tankParam[tankIndex].TankLevelPercent);
    DEBUG_PRINTF("\n");
  }
}

void dumpTempParams(void) {
  DEBUG_PRINTF("\n[dumpTempParams]:\n");
  for (int tempIndex = 0; tempIndex < NUMBER_OF_TEMP_SENSORS; tempIndex++) {
    DEBUG_PRINTF("temp[%d] %s\n", tempIndex,
                 tempParam[tempIndex].enabled ? "enabled " : "disabled");
    DEBUG_PRINTF("temp[%d] tempName: %s\n", tempIndex,
                 tempParam[tempIndex].tempName);
    DEBUG_PRINTF("tank[%d] displayEnabled 0x%0X\n", tempIndex,
                 tempParam[tempIndex].displayEnabled);
  }
  DEBUG_PRINTF("\n");
}

void dumpTachParams(void) {
  DEBUG_PRINTF("\n[dumpTachParams]:\n");
  for (int tachIndex = 0; tachIndex < NUMBER_OF_TACH_SENSORS; tachIndex++) {
    DEBUG_PRINTF("tach[%d] %s\n", tachIndex,
                 tachParam[tachIndex].enabled ? "enabled " : "disabled");
    DEBUG_PRINTF("tach[%d] tachName: %s\n", tachIndex,
                 tachParam[tachIndex].tachName);
    DEBUG_PRINTF("tach[%d] displayEnabled 0x%0X\n", tachIndex,
                 tachParam[tachIndex].displayEnabled);
  }
  DEBUG_PRINTF("\n");
}

void dumpCommonParams(void) {
  DEBUG_PRINTF("\n[dumpCommon]:\n");
  DEBUG_PRINTF("common[remoteNotLocal]:%s\n",
               remoteNotLocal ? "true" : "false");
  DEBUG_PRINTF("common[startupDisplayAddress]:0X%X\n", startupDisplayAddress);
  DEBUG_PRINTF("common[displaySleepDelay]:%d minutes\n", displaySleepDelay);
  DEBUG_PRINTF("\n");
}

const char *filename = "/jwConfig.json";  // <- SD library uses 8.3 filenames

void writeConfigFile(void) {
  DEBUG_PRINTF("[writingConfigFile]\n");
  // const size_t capacity = JSON_ARRAY_SIZE(6) + 6 * JSON_OBJECT_SIZE(3) +
  // JSON_OBJECT_SIZE(6);
  //StaticJsonDocument<2048> doc;
  JsonDocument doc;


  // create an object
  JsonObject root = doc.to<JsonObject>();

  //JsonObject common = root.createNestedObject("common");  //deprecated
  JsonObject common = doc["common"].to<JsonObject>();  //v7

  common["startupDisplayAddress"] = startupDisplayAddress;
  common["remoteNotLocal"] = remoteNotLocal;
  common["displaySleepDelay"] = displaySleepDelay;

  //JsonObject tank_0 = root.createNestedObject("tank_0");
  JsonObject tank_0 = doc["tank_0"].to<JsonObject>();  //v7
  tank_0["enabled"] = tankParam[0].enabled;
  tank_0["tankName"] = tankParam[0].tankName;
  tank_0["n2kEnabled"] = tankParam[0].n2kEnabled;
  tank_0["displayEnable"] = tankParam[0].displayEnabled;
  tank_0["fluidType"] = tankParam[0].fluidType;
  tank_0["calUpperResistance"] = tankParam[0].calUpperResistance;
  tank_0["calUpperVolts"] = tankParam[0].calUpperVolts;
  tank_0["calLowerResistance"] = tankParam[0].calLowerResistance;
  tank_0["calLowerVolts"] = tankParam[0].calLowerVolts;
  tank_0["TankInstance"] = tankParam[0].TankInstance;
  tank_0["adsChannel"] = tankParam[0].adsChannel;
  tank_0["TankCapacity"] = tankParam[0].TankCapacity;
  tank_0["TankLevelPercent"] = tankParam[0].TankLevelPercent;

  //JsonObject tank_1 = root.createNestedObject("tank_1");
  JsonObject tank_1 = doc["tank_1"].to<JsonObject>();  //v7
  tank_1["enabled"] = tankParam[1].enabled;
  tank_1["tankName"] = tankParam[1].tankName;
  tank_1["n2kEnabled"] = tankParam[1].n2kEnabled;
  tank_1["displayEnable"] = tankParam[1].displayEnabled;
  tank_1["fluidType"] = tankParam[1].fluidType;
  tank_1["calUpperResistance"] = tankParam[1].calUpperResistance;
  tank_1["calUpperVolts"] = tankParam[1].calUpperVolts;
  tank_1["calLowerResistance"] = tankParam[1].calLowerResistance;
  tank_1["calLowerVolts"] = tankParam[1].calLowerVolts;
  tank_1["TankInstance"] = tankParam[1].TankInstance;
  tank_1["adsChannel"] = tankParam[1].adsChannel;
  tank_1["TankCapacity"] = tankParam[1].TankCapacity;
  tank_1["TankLevelPercent"] = tankParam[1].TankLevelPercent;

  //JsonObject tank_2 = root.createNestedObject("tank_2");
  JsonObject tank_2 = doc["tank_2"].to<JsonObject>();  //v7
  tank_2["enabled"] = tankParam[2].enabled;
  tank_2["tankName"] = tankParam[2].tankName;
  tank_2["n2kEnabled"] = tankParam[2].n2kEnabled;
  tank_2["displayEnable"] = tankParam[2].displayEnabled;
  tank_2["fluidType"] = tankParam[2].fluidType;
  tank_2["calUpperResistance"] = tankParam[2].calUpperResistance;
  tank_2["calUpperVolts"] = tankParam[2].calUpperVolts;
  tank_2["calLowerResistance"] = tankParam[2].calLowerResistance;
  tank_2["calLowerVolts"] = tankParam[2].calLowerVolts;
  tank_2["TankInstance"] = tankParam[2].TankInstance;
  tank_2["adsChannel"] = tankParam[2].adsChannel;
  tank_2["TankCapacity"] = tankParam[2].TankCapacity;
  tank_2["TankLevelPercent"] = tankParam[2].TankLevelPercent;

  //JsonObject tank_3 = root.createNestedObject("tank_3");
  JsonObject tank_3 = doc["tank_3"].to<JsonObject>();  //v7
  tank_3["enabled"] = tankParam[3].enabled;
  tank_3["tankName"] = tankParam[3].tankName;
  tank_3["n2kEnabled"] = tankParam[3].n2kEnabled;
  tank_3["displayEnable"] = tankParam[3].displayEnabled;
  tank_3["fluidType"] = tankParam[3].fluidType;
  tank_3["calUpperResistance"] = tankParam[3].calUpperResistance;
  tank_3["calUpperVolts"] = tankParam[3].calUpperVolts;
  tank_3["calLowerResistance"] = tankParam[3].calLowerResistance;
  tank_3["calLowerVolts"] = tankParam[3].calLowerVolts;
  tank_3["TankInstance"] = tankParam[3].TankInstance;
  tank_3["adsChannel"] = tankParam[3].adsChannel;
  tank_3["TankCapacity"] = tankParam[3].TankCapacity;
  tank_3["TankLevelPercent"] = tankParam[3].TankLevelPercent;

  //...
  //JsonObject temp_0 = root.createNestedObject("temp_0");
  JsonObject temp_0 = doc["temp_0"].to<JsonObject>();  //v7
  temp_0["enabled"] = tempParam[0].enabled;
  temp_0["tempName"] = tempParam[0].tempName;
  temp_0["n2kEnabled"] = tempParam[0].n2kEnabled;
  temp_0["displayEnable"] = tempParam[0].displayEnabled;

  //JsonObject temp_1 = root.createNestedObject("temp_1");
  JsonObject temp_1 = doc["temp_1"].to<JsonObject>();  //v7
  temp_1["enabled"] = tempParam[1].enabled;
  temp_1["tempName"] = tempParam[1].tempName;
  temp_1["n2kEnabled"] = tempParam[1].n2kEnabled;
  temp_1["displayEnable"] = tempParam[1].displayEnabled;

  //JsonObject temp_2 = root.createNestedObject("temp_2");
  JsonObject temp_2 = doc["temp_2"].to<JsonObject>();  //v7
  temp_2["enabled"] = tempParam[2].enabled;
  temp_2["tempName"] = tempParam[2].tempName;
  temp_2["n2kEnabled"] = tempParam[2].n2kEnabled;
  temp_2["displayEnable"] = tempParam[2].displayEnabled;

  //JsonObject tach_0 = root.createNestedObject("tach_0");
  JsonObject tach_0 = doc["tach_0"].to<JsonObject>();  //v7
  tach_0["enabled"] = tachParam[0].enabled;
  tach_0["tachName"] = tachParam[0].tachName;
  tach_0["n2kEnabled"] = tachParam[0].n2kEnabled;
  tach_0["displayEnable"] = tachParam[0].displayEnabled;
  //..

  DEBUG_PRINTF("*** printing pretty before write to file****\n");
  serializeJsonPretty(doc, Serial);
  DEBUG_PRINTF("\n");

  if (SPIFFS.begin()) {
    DEBUG_PRINTF("File system mounted.\n");
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile) {
      DEBUG_PRINTF("failed to open config file for writing\n");
      return;
    }
    // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0) {
      DEBUG_PRINTF("Failed to write to file\n");
    }

    configFile.close();
    DEBUG_PRINTF("Success! config file writing complete\n");
  } else {
    DEBUG_PRINTF("Failed to mount file system.\n");
  }
}

void loadConfigFile(void) {
  int sensorIndex;
  boolean success = false;
  //StaticJsonDocument<2048> doc;  //deprecated
  JsonDocument doc;   //v7

  // clean FS, for testing
  // SPIFFS.format();

  // read configuration from FS json
  DEBUG_PRINTF("[loadConfigFile] mounting FS...\n");

  if (SPIFFS.begin()) {
    DEBUG_PRINTF("mounted file system\n");
    if (SPIFFS.exists(filename)) {
      // file exists, reading and loading
      DEBUG_PRINTF("reading config file\n");
      File configFile = SPIFFS.open(filename, "r");

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, configFile);
      if (error)
        DEBUG_PRINTF("\n[loadConfigFile] deserialization error: %s\n\n",
                     error.c_str());
      //    Serial.println(F("[loadConfigFile]Failed to read file, using default
      //    configuration"));
      else {
        DEBUG_PRINTF("\n[loadConfigFile] deserialization success!: %s\n\n",
                     error.c_str());

        JsonObject common = doc["common"];
        startupDisplayAddress = common["startupDisplayAddress"];
        remoteNotLocal = common["remoteNotLocal"];
        displaySleepDelay = common["displaySleepDelay"];

        JsonObject tank_0 = doc["tank_0"];
        tankParam[0].enabled = tank_0["enabled"];
        strcpy(tankParam[0].tankName, tank_0["tankName"]);
        tankParam[0].n2kEnabled = tank_0["n2kEnabled"];
        tankParam[0].displayEnabled = tank_0["displayEnable"];
        tankParam[0].fluidType = tank_0["fluidType"];
        tankParam[0].calUpperResistance = tank_0["calUpperResistance"];
        tankParam[0].calUpperVolts = tank_0["calUpperVolts"];
        tankParam[0].calLowerResistance = tank_0["calLowerResistance"];
        tankParam[0].calLowerVolts = tank_0["calLowerVolts"];
        tankParam[0].TankInstance = tank_0["TankInstance"];
        tankParam[0].adsChannel = tank_0["adsChannel"];
        tankParam[0].TankCapacity = tank_0["TankCapacity"];
        tankParam[0].TankLevelPercent = tank_0["TankLevelPercent"];

        JsonObject tank_1 = doc["tank_1"];
        tankParam[1].enabled = tank_1["enabled"];
        strcpy(tankParam[1].tankName, tank_1["tankName"]);
        tankParam[1].n2kEnabled = tank_1["n2kEnabled"];
        tankParam[1].displayEnabled = tank_1["displayEnable"];
        tankParam[1].fluidType = tank_1["fluidType"];
        tankParam[1].calUpperResistance = tank_1["calUpperResistance"];
        tankParam[1].calUpperVolts = tank_1["calUpperVolts"];
        tankParam[1].calLowerResistance = tank_1["calLowerResistance"];
        tankParam[1].calLowerVolts = tank_1["calLowerVolts"];
        tankParam[1].TankInstance = tank_1["TankInstance"];
        tankParam[1].adsChannel = tank_1["adsChannel"];
        tankParam[1].TankCapacity = tank_1["TankCapacity"];
        tankParam[1].TankLevelPercent = tank_1["TankLevelPercent"];

        JsonObject tank_2 = doc["tank_2"];
        tankParam[2].enabled = tank_2["enabled"];
        strcpy(tankParam[2].tankName, tank_2["tankName"]);
        tankParam[2].n2kEnabled = tank_2["n2kEnabled"];
        tankParam[2].displayEnabled = tank_2["displayEnable"];
        tankParam[2].fluidType = tank_2["fluidType"];
        tankParam[2].calUpperResistance = tank_2["calUpperResistance"];
        tankParam[2].calUpperVolts = tank_2["calUpperVolts"];
        tankParam[2].calLowerResistance = tank_2["calLowerResistance"];
        tankParam[2].calLowerVolts = tank_2["calLowerVolts"];
        tankParam[2].TankInstance = tank_2["TankInstance"];
        tankParam[2].adsChannel = tank_2["adsChannel"];
        tankParam[2].TankCapacity = tank_2["TankCapacity"];
        tankParam[2].TankLevelPercent = tank_2["TankLevelPercent"];

        JsonObject tank_3 = doc["tank_3"];
        tankParam[3].enabled = tank_3["enabled"];
        strcpy(tankParam[3].tankName, tank_3["tankName"]);
        tankParam[3].n2kEnabled = tank_3["n2kEnabled"];
        tankParam[3].displayEnabled = tank_3["displayEnable"];
        tankParam[3].fluidType = tank_3["fluidType"];
        tankParam[3].calUpperResistance = tank_3["calUpperResistance"];
        tankParam[3].calUpperVolts = tank_3["calUpperVolts"];
        tankParam[3].calLowerResistance = tank_3["calLowerResistance"];
        tankParam[3].calLowerVolts = tank_3["calLowerVolts"];
        tankParam[3].TankInstance = tank_3["TankInstance"];
        tankParam[3].adsChannel = tank_3["adsChannel"];
        tankParam[3].TankCapacity = tank_3["TankCapacity"];
        tankParam[3].TankLevelPercent = tank_3["TankLevelPercent"];

        JsonObject temp_0 = doc["temp_0"];
        tempParam[0].enabled = temp_0["enabled"];
        strcpy(tempParam[0].tempName, temp_0["tempName"]);
        tempParam[0].n2kEnabled = temp_0["n2kEnabled"];
        tempParam[0].displayEnabled = temp_0["displayEnable"];

        JsonObject temp_1 = doc["temp_1"];
        tempParam[1].enabled = temp_1["enabled"];
        strcpy(tempParam[1].tempName, temp_1["tempName"]);
        tempParam[1].n2kEnabled = temp_1["n2kEnabled"];
        tempParam[1].displayEnabled = temp_1["displayEnable"];

        JsonObject temp_2 = doc["temp_2"];
        tempParam[2].enabled = temp_2["enabled"];
        strcpy(tempParam[2].tempName, temp_2["tempName"]);
        tempParam[2].n2kEnabled = temp_2["n2kEnabled"];
        tempParam[2].displayEnabled = temp_2["displayEnable"];

        JsonObject tach_0 = doc["tach_0"];
        tachParam[0].enabled = tach_0["enabled"];
        strcpy(tachParam[0].tachName, tach_0["tachName"]);
        tachParam[0].n2kEnabled = tach_0["n2kEnabled"];
        tachParam[0].displayEnabled = tach_0["displayEnable"];

        success = true;
        DEBUG_PRINTF("[loadConfigFile]successfully loaded config from file\n");
      }
    }
  }

  if (!success) {
    DEBUG_PRINTF(
        "[loadConfigFile]Failed to load config from file. Loading default "
        "config.\n");
    loadDefaultTankParameters();
    writeConfigFile();
  }
  return;
}

void wipeConfigDB(void) {
  DEBUG_PRINTF("wiping tank config...\n");
  for (int index = 0; index < NUMBER_OF_TANK_SENSORS; index++) {
    tankParam[index].enabled = false;
    strcpy(tankParam[index].tankName, "");
    tankParam[index].n2kEnabled = false;
    tankParam[index].fluidType = N2kft_Unavailable;
    tankParam[index].calUpperResistance = 0;
    tankParam[index].calUpperVolts = 0;
    tankParam[index].calLowerResistance = 0;
    tankParam[index].calLowerVolts = 0;
    tankParam[index].TankInstance = 0;
    tankParam[index].adsChannel = 0;
    tankParam[index].TankCapacity = 0;
    tankParam[index].TankLevelPercent = N2kFloatNA;
  }

  DEBUG_PRINTF("wiping temp config...\n");
  for (int index = 0; index < NUMBER_OF_TEMP_SENSORS; index++) {
    tempParam[index].enabled = false;
    tempParam[index].n2kEnabled = false;
    tempParam[index].displayEnabled = 0b00000000;  // bit 0: degrees C
    strcpy(tempParam[index].tempName, "");
  }
}

void readFile(fs::FS &fs, const char *path) {
  DEBUG_PRINTF("Reading file: %s ", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    DEBUG_PRINTF("- failed to open file for reading\n");
    return;
  }

  DEBUG_PRINTF("- read from file:\n");
#ifndef DEBUG_DISABLE_JW
  while (file.available()) {
    Serial.write(file.read());
  }
#endif
  DEBUG_PRINTF("\n");
  file.close();
}

void deleteFile(fs::FS &fs, const char *path) {
  DEBUG_PRINTF("\nDeleting file: %s ", path);
  if (fs.remove(path)) {
    DEBUG_PRINTF("- file deleted\n\n");
  } else {
    DEBUG_PRINTF("- delete failed\n\n");
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels, bool delWMconfig,
             bool deleteAllButJson, bool delJson) {
  char fileName_str[50];
  DEBUG_PRINTF("\nListing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    DEBUG_PRINTF("- failed to open directory\n");
    return;
  }
  if (!root.isDirectory()) {
    DEBUG_PRINTF(" - not a directory\n");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      DEBUG_PRINTF("  DIR : ");
      DEBUG_PRINTF("%s\n", file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1, delWMconfig, deleteAllButJson,
                delJson);
      }
    } else {
      DEBUG_PRINTF("\r\nFILE: ");
      sprintf(fileName_str, "/%s", file.name());  // adding leading slash
      DEBUG_PRINTF(fileName_str);
      // DEBUG_PRINTF("\tSIZE: ");
      // DEBUG_PRINTF("%s\n",file.size());
      readFile(SPIFFS, fileName_str);

      if (delWMconfig && (strncmp(fileName_str, "/1y6oPD6lHz", 11) == 0))
        deleteFile(SPIFFS, fileName_str);
      if (delJson && (strcmp(fileName_str, "/jwConfig.json") == 0))
        deleteFile(SPIFFS, fileName_str);
      if (deleteAllButJson && (strcmp(fileName_str, "/jwConfig.json") != 0))
        deleteFile(SPIFFS, fileName_str);
    }
    file = root.openNextFile();
  }
}

// void listDir(fs::FS &fs, const char * dirname, uint8_t levels,  bool
// delWMconfig, bool deleteAllButJson, bool delJson){

void runListDir(bool delWMconfig, bool deleteAllButJson, bool delJson) {
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    DEBUG_PRINTF("SPIFFS Mount Failed\n");
    return;
  }
  listDir(SPIFFS, "/", 0, delWMconfig, deleteAllButJson, delJson);
}