#include "sensor_conversions.h"

float TMP102_To_Float(uint16_t input)
{
	return input * (float)TMP102_LSB_INC;
}

uint16_t Float_To_TMP102(float input)
{
	return input / (float)TMP102_LSB_INC;
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
