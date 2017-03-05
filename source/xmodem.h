#pragma once

enum XMODEM_CONTROL_CHARACTERS {SOH = 0x01, EOT = 0x04, ACK = 0x06, NACK = 0x15, ETB = 0x17, CAN = 0x18, C = 0x43}; 

#define XMODEM_BLOCK_SIZE 128

//static const uint8_t  XMODEM_BLOCK_SIZE  = 128;   // fixed block size 

typedef struct
{
  uint8_t  preamble;
  uint8_t  id;
  uint8_t  id_complement;
  uint8_t  data[XMODEM_BLOCK_SIZE];
  uint16_t crc;
} xmodem_packet_t;

bool xmodem_verify_packet(const xmodem_packet_t packet, uint8_t expected_packet_id);
bool xmodem_calculate_crc(const uint8_t *data, const uint32_t size, uint16_t *result);


