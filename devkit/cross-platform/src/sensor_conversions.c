#include "sensor_conversions.h"

float TMP102_To_Float(uint16_t input)
{
	//return ((input & 0x8000) ? (-1.0f) * (((~input) + 1) * (float)TMP102_LSB_INC) : input * (float)TMP102_LSB_INC);
    return input * (float)TMP102_LSB_INC;
}

float HTU21D_Humid_To_Float(uint16_t input)
{
	return -6.0f + (125.0f * ((float)input / (float)(1 << 16)));
}

uint16_t Float_To_HTU21D_Humid(float input)
{
	return ((input + 6.0f) / 125.0f) * ((float)(1 << 16));
}

float HTU21D_Temp_To_Float(uint16_t input)
{
	return -46.85f + (175.72f * ((float)input / (float)(1 << 16)));
}

uint16_t Float_To_HTU21D_Temp(float input)
{
	return (((input + 46.85f) / 175.72f) * ((float)(1 << 16)));
}

float MPL311_Alt_To_Float(uint32_t input)
{
    return (float)input / 65536.0f;
}

float SMS_To_Float(uint16_t input)
{
    return input * 1.0;
}

uint16_t Float_To_SMS(float input)
{
    return input / 1.0;
}

float Moisture_To_Float(float moist)
{
    float res = (moist - 3669.52f) / 89.1429f;
    
    if(res < 0.0f)
    {
        res = 0.0f;
    }
    
    return res;
}
