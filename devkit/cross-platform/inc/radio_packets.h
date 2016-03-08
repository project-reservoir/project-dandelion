#ifndef _RADIO_PACKETS_
#define _RADIO_PACKETS_

#include "radio.h"
#include "debug.h"

#define RADIO_MSG_BYTES             RADIO_MAX_PACKET_LENGTH
#define RADIO_MAX_PAYLOAD_BYTES     (RADIO_MAX_PACKET_LENGTH - 12)
#define NUM_FW_UPDATE_PAYLOAD_WORDS 12

typedef enum {
    FW_UPD_START    = 1,
    FW_UPD_PAYLOAD  = 2,
    FW_UPD_END      = 3,
    SENSOR_MSG      = 4,
    DEVICE_INFO     = 5,
    ACK             = 6,
    PING            = 7,
    PONG            = 8,
    ANNOUNCE        = 9,
    JOIN            = 10,
    TIME_UPDATE     = 11,
    RSSI            = 12,
    SENSOR_CMD      = 13
} radio_cmd_t;

#pragma pack(1)
typedef struct {
    uint16_t moisture0;
    uint16_t moisture1;
    uint16_t moisture2;
    uint16_t humid;
    uint16_t temp0;
    uint16_t temp1;
    uint16_t temp2;
    uint16_t air_temp;
    uint16_t battery_level;
    int16_t  chip_temp;
    uint32_t timestamp;
} sensor_payload_t;

#pragma pack(1)
typedef struct {
    uint32_t crc32;
} fw_update_start_payload_t;

#pragma pack(1)
typedef struct {
    uint32_t offset;
    uint32_t payload[NUM_FW_UPDATE_PAYLOAD_WORDS];
} fw_update_payload_t;

#pragma pack(1)
typedef struct {
    uint32_t mac;
    uint32_t version;
    uint32_t battery_level;
} device_info_payload_t;

#pragma pack(1)
typedef struct {
    uint8_t curr_rssi;
    uint8_t latch_rssi;
    uint8_t ant1_rssi;
    uint8_t ant2_rssi;
} rssi_payload_t;

#pragma pack(1)
typedef struct {
    uint32_t sensor_polling_period; // Field 0
    uint32_t valid_fields;
} sensor_cmd_payload_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  flags;
    uint16_t control;
    uint32_t src;
    uint32_t dst;
    union {
        uint8_t                     buffer[52];
        device_info_payload_t       device_info;
        fw_update_payload_t         fw_update_data;
        fw_update_start_payload_t   fw_update_start;
        sensor_payload_t            sensor_message;
        rssi_payload_t              rssi_message;
        sensor_cmd_payload_t        sensor_cmd;
    } payload;
} generic_message_t;

STATIC_ASSERT(sizeof(generic_message_t) == RADIO_MSG_BYTES);

#endif // _RADIO_PACKETS_
