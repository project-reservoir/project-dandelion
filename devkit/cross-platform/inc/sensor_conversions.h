#ifndef _SENSOR_CONVERSIONS_H
#define _SENSOR_CONVERSIONS_H

//Include standard compiler definitions through this file
#include "FreeRTOS.h"

#define TMP102_LSB_INC   0.0625f

typedef struct SensorData_t {
    float temp0;
    float temp1;
    float temp2;
    float tempChip;
    float tempRadio;
    float tempAir;
    float moist0;
    float moist1;
    float moist2;
    float humid;
} SensorData;

// TMP102 sensors
float    TMP102_To_Float(uint16_t input);
uint16_t Float_To_TMP102(float input);

// HTU21D Air temperature/humidity sensors
float    HTU21D_Humid_To_Float(uint16_t input);
uint16_t Float_To_HTU21D_Humid(float input);
float    HTU21D_Temp_To_Float(uint16_t input);
uint16_t Float_To_HTU21D_Temp(float input);

#endif
