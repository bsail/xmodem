#pragma once

enum XMODEM_CONTROL_CHARACTERS {SOH = 0x01, EOT = 0x04, ACK = 0x06, NACK = 0x15, CAN = 0x18, C = 0x43}; 

enum XMODEM_TRANSMIT_STATES {XMODEM_TRANSMIT_INITIAL,               XMODEM_TRANSMIT_WAIT_FOR_C, 
                             XMODEM_TRANSMIT_WAIT_FOR_TRANSFER_ACK, XMODEM_TRANSMIT_WRITE_BLOCK_FAILED,
                             XMODEM_TRANSMIT_ABORT,                 XMODEM_TRANSMIT_WRITE_BLOCK, 
                             XMODEM_TRANSMIT_TRANSFER_ACK_RECEIVED, XMODEM_TRANSMIT_COMPLETE,
                             XMODEM_TRANSMIT_WAIT_WRITE_BLOCK,      XMODEM_TRANSMIT_UNKNOWN } typedef xmodem_transmit_state_t;

enum XMODEM_RECEIVE_STATES {XMODEM_RECEIVE_INITIAL, XMODEM_RECEIVE_WAIT_FOR_NACK, XMODEM_RECEIVE_SEND_REQUEST_FOR_TRANSFER, XMODEM_RECEIVE_WAIT_FOR_TRANSFER_ACK, XMODEM_RECEIVE_TIMEOUT_TRANSFER_ACK,
                    XMODEM_RECEIVE_TIMEOUT_WAIT_READ_BLOCK, XMODEM_RECEIVE_ABORT_TRANSFER, XMODEM_RECEIVE_READ_BLOCK, XMODEM_RECEIVE_TRANSFER_ACK_RECEIVED,
                    XMODEM_RECEIVE_TRANSFER_COMPLETE, XMODEM_RECEIVE_BLOCK_RECEIVED, XMODEM_RECEIVE_INVALID_BLOCK, XMODEM_RECEIVE_ACK_BLOCK, XMODEM_RECEIVE_VALID_BLOCK,
                    XMODEM_RECEIVE_UNKNOWN } typedef xmodem_receive_state_t;

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

xmodem_transmit_state_t xmodem_transmit_state();
xmodem_receive_state_t xmodem_receive_state();

bool xmodem_transmit_init(uint8_t *buffer, uint32_t size);
bool xmodem_receive_init();
bool xmodem_transmit_process(const uint32_t current_time);
bool xmodem_receive_process(const uint32_t current_time);
bool xmodem_cleanup();
bool xmodem_verify_packet(const xmodem_packet_t packet, uint8_t expected_packet_id);
bool xmodem_calculate_crc(const uint8_t *data, const uint32_t size, uint16_t *result);

void xmodem_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_set_callback_is_outbound_full(bool (*callback)());
void xmodem_set_callback_is_inbound_empty(bool (*callback)());
