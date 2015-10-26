#ifndef _RADIO_PACKETS_
#define _RADIO_PACKETS_

#include "radio.h"

#define RADIO_MSG_BYTES             RADIO_MAX_PACKET_LENGTH
#define NUM_FW_UPDATE_PAYLOAD_WORDS 15

typedef enum {
    FW_UPD_START    = 1,
    FW_UPD_PAYLOAD  = 2,
    FW_UPD_END      = 3,
    SENSOR_MSG      = 4,
    DEVICE_INFO     = 5,
    ACK             = 6,
    PING            = 7,
    PONG            = 8
} radio_cmd_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint16_t moisture0;
    uint16_t moisture1;
    uint16_t moisture2;
    uint16_t humid;
    uint16_t temp0;
    uint16_t temp1;
    uint16_t temp2;
    uint16_t air_temp;
    uint16_t battery_level;
    uint16_t solar_level;
    uint8_t  ignd[RADIO_MSG_BYTES - 21];
} sensor_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint32_t crc32;
    uint8_t  ignd[RADIO_MSG_BYTES - 5];
} fw_update_start_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint32_t payload[NUM_FW_UPDATE_PAYLOAD_WORDS];
    uint8_t  ignd[RADIO_MSG_BYTES - 1 - (4 * NUM_FW_UPDATE_PAYLOAD_WORDS)];
} fw_update_payload_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  ignd[RADIO_MSG_BYTES - 1];
} fw_update_end_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  ignd[RADIO_MSG_BYTES - 1];
} device_info_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  ignd[RADIO_MSG_BYTES - 1];
} generic_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  ignd[RADIO_MSG_BYTES - 1];
} ack_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  ignd[RADIO_MSG_BYTES - 1];
} ping_message_t;

#pragma pack(1)
typedef struct {
    uint8_t  cmd;
    uint8_t  ignd[RADIO_MSG_BYTES - 1];
} pong_message_t;

typedef union {
    generic_message_t           generic;
    device_info_message_t       device_info;
    sensor_message_t            sensor;
    ping_message_t              ping;
    fw_update_end_message_t     fw_update_end;
    fw_update_payload_message_t fw_update_payload;
    fw_update_start_message_t   fw_update_start;
} radio_message_t;

#endif // _RADIO_PACKETS_
