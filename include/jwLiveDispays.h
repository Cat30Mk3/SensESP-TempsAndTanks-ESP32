#pragma once
#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include <U8g2lib.h>

#include "jwMenuAndUI.h"
#include "jwStructsAndGlobals.h"

#define LCDWidth u8g2.getDisplayWidth()
#define ALIGN_CENTER(t) ((LCDWidth - (u8g2.getUTF8Width(t))) / 2)

int startCentre(char *dispStr) {
  int startPosn = (u8g2.getDisplayWidth() - u8g2.getStrWidth(dispStr)) / 2;
  return startPosn;
}



void newLiveDisplayBuilder(void) {
  if (!mui.isFormActive() && !semaphore_encoder) {
    if (!displayIsAsleep) {
      char lineBuff[40];
      char title[25];
      char name[20];
      int row = 0;
      static int polIndex = 0;  // proofOfLife index into polChars 4 character
                                // array to produce animated character
      char polChars[] = {"|/-\\"};  //  note // is a single backslash - this
                                    //  array contains 4 characters [0] ..[3]
      u8g2.setFont(
          u8g2_font_9x15B_mf);  // use monolithic so animation doesnt
                                // resize length and cause title to jump
      u8g2.clearBuffer();
      if (++polIndex > 3)
        polIndex = 0;  // increments and rolls over all in one line -- slick!

      sprintf(lineBuff, "%c", polChars[polIndex]);
      u8g2.drawStr(LCDWidth - 10, CHARACTER_HEIGHT, lineBuff);

      switch (currentLiveDisplayType) {
        case TEMP_ALL_TYPE:
          sprintf(title, "%s", "Temperatures");
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(ALIGN_CENTER(title), CHARACTER_HEIGHT, title);
          row++;  // one line space below title
          for (int tempIndex = 0; tempIndex < NUMBER_OF_TEMP_SENSORS;
               tempIndex++) {
            if (tempParam[tempIndex].enabled) {
              row++;
              strcpy(name, tempParam[tempIndex].tempName);
              strcat(name, ":");
              sprintf(lineBuff, "%-15s", name);
              u8g2.drawStr(0, CHARACTER_HEIGHT + (CHARACTER_HEIGHT * row),
                           lineBuff);
              sprintf(lineBuff, "%4.1f",
                      tempParam[tempIndex].tempReadingCelsius);
              u8g2.drawStr(u8g2.getDisplayWidth() * 2 / 3,
                           CHARACTER_HEIGHT + (CHARACTER_HEIGHT * row),
                           lineBuff);
              
            }
            
          }
          u8g2.sendBuffer();
          break;

        case TANK_ALL_TYPE:
          sprintf(title, "%s", "Tanks");
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(ALIGN_CENTER(title), CHARACTER_HEIGHT, title);
          row++;  // one line space below title
          for (int tankIndex = 0; tankIndex < NUMBER_OF_TANK_SENSORS;
               tankIndex++) {
            if (tankParam[tankIndex].enabled) {
              row++;
              strcpy(name, tankParam[tankIndex].tankName);
              strcat(name, ":");
              sprintf(lineBuff, "%-15s", name);
              u8g2.drawStr(0, CHARACTER_HEIGHT + (CHARACTER_HEIGHT * row),
                           lineBuff);
              sprintf(lineBuff, "%4.1f",
                      100 * tankParam[tankIndex].TankLevelPercent);
              u8g2.drawStr(u8g2.getDisplayWidth() * 2 / 3,
                           CHARACTER_HEIGHT + (CHARACTER_HEIGHT * row),
                           lineBuff);
              

            }
          }
          u8g2.sendBuffer();
          break;

        case TANK_INDV_TYPE:
          if(tankParam[currentLiveDisplaySensor].enabled){
          sprintf(title, "%s", tankParam[currentLiveDisplaySensor].tankName);
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(ALIGN_CENTER(title), CHARACTER_HEIGHT, title);
          u8g2.setFont(u8g2_font_mystery_quest_32_tr);
          switch (currentLiveDisplayQty) {
            case TANK_QTY_PCNT:
              sprintf(
                  lineBuff, "%5.1f %%%",
                  tankParam[currentLiveDisplaySensor].TankLevelPercent * 100);
              break;
            case TANK_QTY_LTRS:
              sprintf(lineBuff, "%5.1f L",
                      tankParam[currentLiveDisplaySensor].TankLevelPercent *
                          tankParam[currentLiveDisplaySensor].TankCapacity);
              break;
            case TANK_QTY_STEP:
              sprintf(lineBuff, "Step %-2d",
                      tankParam[currentLiveDisplaySensor].WemaStep);
              break;
            case TANK_QTY_OHMS:
              sprintf(lineBuff, "%3.0f Ohms",
                      tankParam[currentLiveDisplaySensor].OhmsCalc);
              break;
            case TANK_QTY_VOLT:
              sprintf(lineBuff, "%5.3f V",
                      tankParam[currentLiveDisplaySensor].voltsMeasured);
              break;
            default:
              Serial.printf(
                  "\nDISPLAY INDEX ERROR: currentLiveDisplayQty %3d\n\n",
                  currentLiveDisplayQty);
              break;
          };  // switch (displayPanelIndex)
          u8g2.setCursor(startCentre(lineBuff), 50);
          u8g2.print(lineBuff);
          u8g2.sendBuffer();
          }
          break;

        case TEMP_INDV_TYPE:
         if(tempParam[currentLiveDisplaySensor].enabled){
          sprintf(title, "%s", tempParam[currentLiveDisplaySensor].tempName);
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(ALIGN_CENTER(title), CHARACTER_HEIGHT, title);
          u8g2.setFont(u8g2_font_mystery_quest_32_tr);
          sprintf(lineBuff, "%5.1f C",
                  tempParam[currentLiveDisplaySensor].tempReadingCelsius);
          u8g2.setCursor(startCentre(lineBuff), 50);
          u8g2.print(lineBuff);
          u8g2.sendBuffer();
         }
          break;

        case TACH_INDV_TYPE:
        if(tachParam[currentLiveDisplaySensor].enabled){
          sprintf(title, "%s", tachParam[currentLiveDisplaySensor].tachName);
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(ALIGN_CENTER(title), CHARACTER_HEIGHT, title);
          u8g2.setFont(u8g2_font_mystery_quest_28_tr);
          switch (currentLiveDisplayQty) {
            case TACH_QTY_RPM:
               sprintf(lineBuff, "%d rpm", tachParam[currentLiveDisplaySensor].tachReadingRpm); 
              break;
            case TACH_QTY_MS:
               sprintf(lineBuff, "%5.1f ms", (float) tachParam[currentLiveDisplaySensor].periodMicroSeconds/1000.0); 
              break;
          }
          u8g2.setCursor(startCentre(lineBuff), 50);
          u8g2.print(lineBuff);
          u8g2.sendBuffer();
        }
          break;

        default:
          DEBUG_PRINTF("\nDISPLAY INDEX ERROR: currentLiveDisplayType %3d\n\n",
                        currentLiveDisplayType);
      };  //  switch (currentLiveDisplayType)

    }  // if (!displayIsAsleep)
  }    // if (!mui.isFormActive())
}

