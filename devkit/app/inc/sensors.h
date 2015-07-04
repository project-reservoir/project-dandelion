#ifndef _SENSORS_H
#define _SENSORS_H

// TMP102 (Temperature Sensor) definitions
#define TMP102_0_ADDR               0x90
#define TMP102_1_ADDR               0x91
#define TMP102_2_ADDR               0x92

#define TMP102_CONFIG_1_ONESHOT_VAL 0xE1
#define TMP102_CONFIG_1_VAL         0x41
#define TMP102_CONFIG_2_VAL         0xA0

#define TMP102_TEMP_ADDR            0x00
#define TMP102_CONFIG_ADDR          0x01
#define TMP102_TLOW_ADDR            0x02
#define TMP102_THIGH_ADDR           0x03

/*typedef struct SensorData_t {
    float temp0;
    float temp2;
    float temp3;
    float tempChip;
    float tempRadio;
    float moist0;
    float moist1;
    float moist2;
    float humid;
} SensorData;*/

void SensorsTaskOSInit(void);
void SensorsTaskHwInit(void);
void SensorsTask(void);







#endif // _SENSORS_H
