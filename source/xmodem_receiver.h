#pragma once

enum XMODEM_RECEIVE_STATES {XMODEM_RECEIVE_INITIAL, XMODEM_RECEIVE_WAIT_FOR_NACK, XMODEM_RECEIVE_SEND_REQUEST_FOR_TRANSFER, XMODEM_RECEIVE_WAIT_FOR_TRANSFER_ACK, XMODEM_RECEIVE_TIMEOUT_TRANSFER_ACK,
                    XMODEM_RECEIVE_TIMEOUT_WAIT_READ_BLOCK, XMODEM_RECEIVE_ABORT_TRANSFER, XMODEM_RECEIVE_READ_BLOCK, XMODEM_RECEIVE_TRANSFER_ACK_RECEIVED,
                    XMODEM_RECEIVE_TRANSFER_COMPLETE, XMODEM_RECEIVE_BLOCK_RECEIVED, XMODEM_RECEIVE_INVALID_BLOCK, XMODEM_RECEIVE_ACK_BLOCK, XMODEM_RECEIVE_VALID_BLOCK,
                    XMODEM_RECEIVE_UNKNOWN } typedef xmodem_receive_state_t;


xmodem_receive_state_t xmodem_receive_state();

bool xmodem_receive_init();
bool xmodem_receive_process(const uint32_t current_time);
bool xmodem_receiver_cleanup();

void xmodem_receiver_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_receiver_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_receiver_set_callback_is_outbound_full(bool (*callback)());
void xmodem_receiver_set_callback_is_inbound_empty(bool (*callback)());


