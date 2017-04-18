#pragma once


enum XMODEM_TRANSMIT_STATES {XMODEM_TRANSMIT_INITIAL,               XMODEM_TRANSMIT_WAIT_FOR_C, 
                             XMODEM_TRANSMIT_WAIT_FOR_C_ACK,        XMODEM_TRANSMIT_WRITE_BLOCK_FAILED,
                             XMODEM_TRANSMIT_ABORT_TRANSFER,        XMODEM_TRANSMIT_WRITE_BLOCK, 
                             XMODEM_TRANSMIT_C_ACK_RECEIVED,        XMODEM_TRANSMIT_COMPLETE,
                             XMODEM_TRANSMIT_WRITE_EOT,             XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK,
                             XMODEM_TRANSMIT_TIMEOUT_EOT,           XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT,
                             XMODEM_TRANSMIT_WRITE_ETB,             XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK,
                             XMODEM_TRANSMIT_TIMEOUT_ETB,
                             XMODEM_TRANSMIT_WAIT_WRITE_BLOCK,      XMODEM_TRANSMIT_UNKNOWN } typedef xmodem_transmit_state_t;

xmodem_transmit_state_t xmodem_transmit_state();

bool xmodem_transmit_init(uint8_t *buffer, uint32_t size);
bool xmodem_transmit_process(const uint32_t current_time);
bool xmodem_transmitter_cleanup();
void xmodem_transmitter_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, bool *write_status));
void xmodem_transmitter_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size));
void xmodem_transmitter_set_callback_is_outbound_full(bool (*callback)());
void xmodem_transmitter_set_callback_is_inbound_empty(bool (*callback)());

