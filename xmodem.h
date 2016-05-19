#pragma once

enum XMODEM_CONTROL_CHARACTERS {SOH = 0x01, EOT = 0x04, ACK = 0x06, NAK = 0x21, NAK_CRC = 0x67, CAN = 0x24}; 

#if 0
// Communication control characters
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15
#define ETB 0x17
#define CAN 0x18
#define EOF 0x1A
#endif

enum XMODEM_STATES {XMODEM_INITIAL, XMODEM_SEND_REQUEST_FOR_TRANSFER, XMODEM_WAIT_FOR_TRANSFER_ACK, XMODEM_TIMEOUT_TRANSFER_ACK,
                    XMODEM_TIMEOUT_WAIT_READ_BLOCK, XMODEM_ABORT_TRANSFER, XMODEM_READ_BLOCK, XMODEM_TRANSFER_ACK_RECEIVED,
                    XMODEM_TRANSFER_COMPLETE, XMODEM_BLOCK_RECEIVED, XMODEM_INVALID_BLOCK, XMODEM_ACK_BLOCK, XMODEM_VALID_BLOCK,
                    XMODEM_UNKNOWN } typedef xmodem_state_t;

xmodem_state_t xmodem_state();
bool xmodem_init();
bool xmodem_process(const uint32_t current_time);
bool xmodem_cleanup();

void xmodem_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_set_callback_is_outbound_full(bool (*callback)());
void xmodem_set_callback_is_inbound_empty(bool (*callback)());
