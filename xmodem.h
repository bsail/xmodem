#pragma once

enum XMODEM_CONTROL_CHARACTERS {SOH = 0x01, EOT = 0x04, ACK = 0x06, NACK = 0x21, NACK_CRC = 0x67, CAN = 0x24}; 

enum XMODEM_TRANSMIT_STATES {XMODEM_INITIAL, XMODEM_WAIT_FOR_NACK, XMODEM_SEND_REQUEST_FOR_TRANSFER, XMODEM_WAIT_FOR_TRANSFER_ACK, XMODEM_TIMEOUT_TRANSFER_ACK,
                    XMODEM_TIMEOUT_WAIT_READ_BLOCK, XMODEM_ABORT_TRANSFER, XMODEM_READ_BLOCK, XMODEM_TRANSFER_ACK_RECEIVED,
                    XMODEM_TRANSFER_COMPLETE, XMODEM_BLOCK_RECEIVED, XMODEM_INVALID_BLOCK, XMODEM_ACK_BLOCK, XMODEM_VALID_BLOCK,
                    XMODEM_UNKNOWN } typedef xmodem_transmit_state_t;

enum XMODEM_RECEIVE_STATES {XMODEM_INITIAL, XMODEM_WAIT_FOR_NACK, XMODEM_SEND_REQUEST_FOR_TRANSFER, XMODEM_WAIT_FOR_TRANSFER_ACK, XMODEM_TIMEOUT_TRANSFER_ACK,
                    XMODEM_TIMEOUT_WAIT_READ_BLOCK, XMODEM_ABORT_TRANSFER, XMODEM_READ_BLOCK, XMODEM_TRANSFER_ACK_RECEIVED,
                    XMODEM_TRANSFER_COMPLETE, XMODEM_BLOCK_RECEIVED, XMODEM_INVALID_BLOCK, XMODEM_ACK_BLOCK, XMODEM_VALID_BLOCK,
                    XMODEM_UNKNOWN } typedef xmodem_receive_state_t;

xmodem_state_t xmodem_state();
bool xmodem_init();
bool xmodem_process(const uint32_t current_time);
bool xmodem_cleanup();

void xmodem_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_set_callback_is_outbound_full(bool (*callback)());
void xmodem_set_callback_is_inbound_empty(bool (*callback)());
