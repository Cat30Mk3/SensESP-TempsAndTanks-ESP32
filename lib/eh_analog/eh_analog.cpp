#include "eh_analog.h"

#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/curveinterpolator.h"
#include "sensesp/transforms/linear.h"

// ADS1115 input hardware scale factor (input voltage vs voltage at ADS1115)
// const float kAnalogInputScale = 29. / 2.048;
const float kAnalogInputScale = 1.0;

// Engine Hat constant measurement current (A)
// const float kMeasurementCurrent = 0.01;
const float kMeasurementCurrent = 1.0;

// Default fuel tank size, in m3
const float kTankDefaultSize = 60. / 1000;

uint8_t calculateWemaStep(float Rmeas) {
  bool found = false;
  int lastStepOhms =
      2 * WEMAresistanceSteps[0];  // something much bigger than first step
                                   // resistance for initial test
  int thisStepOhms = 0;
  uint8_t thisStep = 0;

  // detect high end case
  if (Rmeas <= WEMAresistanceSteps[NUMBER_OF_WEMA_STEPS - 2] -
                   ((WEMAresistanceSteps[NUMBER_OF_WEMA_STEPS - 2] -
                     WEMAresistanceSteps[NUMBER_OF_WEMA_STEPS - 1]) /
                    2)) {
    thisStep = NUMBER_OF_WEMA_STEPS - 1;
    found = true;
  }
  if (!found) {
    // find index for mid cases - using halfway criteria
    for (thisStep = 0; thisStep <= NUMBER_OF_WEMA_STEPS - 2; thisStep++) {
      thisStepOhms =
          WEMAresistanceSteps[thisStep] -
          ((WEMAresistanceSteps[thisStep] - WEMAresistanceSteps[thisStep + 1]) /
           2);

      if ((Rmeas <= lastStepOhms) && (Rmeas >= thisStepOhms)) {
        found = true;
        break;
      }
      lastStepOhms = thisStepOhms;
    };
  }
  if (found) {
    // Serial.print("\nSTEP found at index [");
    // Serial.print(thisStep);
    // Serial.print("] ");
    // Serial.print(lastStepOhms);
    // Serial.print("--> ");
    // Serial.print(Rmeas);
    // Serial.print("<--");
    // Serial.println(thisStepOhms);
    // tankParam[3].WemaStep = thisStep;
    // Serial.printf("***** GLOBAL STEP: %d\n\n", tankParam[3].WemaStep);
    return thisStep;
  } else
    return -1;  // if no match found
}

// FloatProducer* ConnectTankSender(Adafruit_ADS1115* ads1115, int channel,
// String name) {
FloatProducer *ConnectTankSender(Adafruit_ADS1115 *ads1115, int channel,
                                 String name, double M, double B) {
  const uint ads_read_delay = 500;  // ms ..was 500

  char config_path[80];
  char sk_path[80];
  char meta_display_name[80];
  char meta_description[80];

  snprintf(config_path, sizeof(config_path), "/Tank %s/Resistance",
           name.c_str());
  auto sender_resistance =
      new RepeatSensor<float>(ads_read_delay, [ads1115, channel, M, B]() {
        float adc_output_ohms, input_volts;

        int16_t adc_output = ads1115->readADC_SingleEnded(channel);
        float adc_output_volts = ads1115->computeVolts(adc_output);

        // if(channel < 3)adc_output_volts = adc_output_volts * 10.0;
        // Serial.printf("ADC volts returned: %f\n",adc_output_volts );

        // adc_output_ohms = (M* adc_output_volts) + B;

        if (tankParam[channel].wemaTank) {
          //  adc_output_ohms = (adc_output_volts * tankParam[channel].R0ohms) /
          //  //reg Vcc sensor circuit
          //                    (VCCvolts - adc_output_volts); //reg Vcc sensor
          //                    circuit

          if (adc_output_volts > tankParam[channel].calLowerVolts)
            adc_output_volts = tankParam[channel].calLowerVolts;
          input_volts =
              adc_output_volts *
              (tankParam[channel].R1ohms + tankParam[channel].R2ohms) /
              tankParam[channel].R2ohms;

          adc_output_ohms = input_volts / (ICCma / 1000.0);
          tankParam[channel].WemaStep = calculateWemaStep(adc_output_ohms);

        } else {
          if (adc_output_volts < tankParam[channel].calLowerVolts)
            adc_output_volts = tankParam[channel].calLowerVolts;
          // input_volts =
          //     adc_output_volts *
          //     (tankParam[channel].R1ohms + tankParam[channel].R2ohms) /
          //     tankParam[channel].R2ohms;
          adc_output_ohms = (M * adc_output_volts) + B;
        }

        tankParam[channel].voltsMeasured = adc_output_volts;
        tankParam[channel].OhmsCalc = adc_output_ohms;

        // Serial.printf("ADC Chn:%d cnt:%d volts: %5.3f ohms: %5.1f step: %d\n",
        //               channel, adc_output, tankParam[channel].voltsMeasured,
        //               tankParam[channel].OhmsCalc, tankParam[channel].WemaStep);

        // adc_output_ohms = (133.647 * adc_output_volts) - 77.5; //based on
        // imperical readings of test jig
        // Serial.printf("Sender Resistance returned: %f\n",adc_output_ohms );

        // return kAnalogInputScale * adc_output_volts / kMeasurementCurrent;
        return adc_output_ohms;
      });

  snprintf(config_path, sizeof(config_path), "/Tank %s/Resistance SK Path",
           name.c_str());
  snprintf(sk_path, sizeof(sk_path), "tanks.fuel.%s.senderResistance",
           name.c_str());
  snprintf(meta_display_name, sizeof(meta_display_name), "Resistance %s",
           name.c_str());
  snprintf(meta_description, sizeof(meta_description),
           "Measured tank %s sender resistance", name.c_str());
  auto sender_resistance_sk_output = new SKOutputFloat(
      sk_path, config_path,
      new SKMetadata("ohm", meta_display_name, meta_description));

  snprintf(config_path, sizeof(config_path), "/Tank %s/Level Curve",
           name.c_str());
  auto tank_level = (new CurveInterpolator(nullptr, config_path))
                        ->set_input_title("Sender Resistance (ohms)")
                        ->set_output_title("Fuel Level (ratio)");

  ConfigItem(tank_level)
                        ->set_title(config_path)
                        ->set_description("Tank Level Curve.")
                        ->set_sort_order(1100);

  if (tank_level->get_samples().empty())
  //    if (true)  //JW force to use these values for testing
  {
    // If there's no prior configuration, provide a default curve
    tank_level->clear_samples();
    tank_level->add_sample(CurveInterpolator::Sample(33.6, 1));
    tank_level->add_sample(
        CurveInterpolator::Sample((234.8 - 33.6) / 2.0, 0.5));
    tank_level->add_sample(CurveInterpolator::Sample(234.8, 0));
  }

  snprintf(config_path, sizeof(config_path), "/Tank %s/Current Level SK Path",
           name.c_str());
  snprintf(sk_path, sizeof(sk_path), "tanks.fuel.%s.currentLevel",
           name.c_str());
  snprintf(meta_display_name, sizeof(meta_display_name), "Tank %s level",
           name.c_str());
  snprintf(meta_description, sizeof(meta_description), "Tank %s level",
           name.c_str());
  auto tank_level_sk_output = new SKOutputFloat(
      sk_path, config_path,
      new SKMetadata("ratio", meta_display_name, meta_description));

  snprintf(config_path, sizeof(config_path), "/Tank %s/Total Volume",
           name.c_str());
  auto tank_volume = new Linear(kTankDefaultSize, 0, config_path);

  snprintf(config_path, sizeof(config_path), "/Tank %s/Current Volume SK Path",
           name.c_str());
  snprintf(sk_path, sizeof(sk_path), "tanks.fuel.%s.currentVolume",
           name.c_str());
  snprintf(meta_display_name, sizeof(meta_display_name), "Tank %s volume",
           name.c_str());
  snprintf(meta_description, sizeof(meta_description),
           "Calculated tank %s remaining volume", name.c_str());
  auto tank_volume_sk_output = new SKOutputFloat(
      sk_path, config_path,
      new SKMetadata("m3", meta_display_name, meta_description));

  sender_resistance->connect_to(sender_resistance_sk_output);

  sender_resistance->connect_to(tank_level)->connect_to(tank_level_sk_output);

  tank_level->connect_to(tank_volume)->connect_to(tank_volume_sk_output);

  return tank_level;
}