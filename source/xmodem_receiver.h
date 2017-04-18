#pragma once

enum XMODEM_RECEIVE_STATES {XMODEM_RECEIVE_INITIAL,                   
                            XMODEM_RECEIVE_SEND_C,                    XMODEM_RECEIVE_WAIT_FOR_ACK,
                            XMODEM_RECEIVE_TIMEOUT_ACK,               XMODEM_RECEIVE_READ_BLOCK_TIMEOUT,
                            XMODEM_RECEIVE_ABORT_TRANSFER,            XMODEM_RECEIVE_READ_BLOCK,
                            XMODEM_RECEIVE_ACK_SUCCESS,               XMODEM_RECEIVE_TRANSFER_COMPLETE,
                            XMODEM_RECEIVE_READ_BLOCK_SUCCESS,        XMODEM_RECEIVE_BLOCK_INVALID,
                            XMODEM_RECEIVE_BLOCK_ACK,                 XMODEM_RECEIVE_BLOCK_VALID,
                            XMODEM_RECEIVE_UNKNOWN } typedef xmodem_receive_state_t;


xmodem_receive_state_t xmodem_receive_state();

bool xmodem_receive_init();
bool xmodem_receive_process(const uint32_t current_time);
bool xmodem_receive_cleanup();

void xmodem_receive_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, bool *write_status));
void xmodem_receive_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_receive_set_callback_is_outbound_full(bool (*callback)());
void xmodem_receive_set_callback_is_inbound_empty(bool (*callback)());


