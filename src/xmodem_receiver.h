#ifndef xmodem_receiver_h
#define xmodem_receiver_h
// #pragma once

#include <inttypes.h>

enum XMODEM_RECEIVE_STATES { XMODEM_RECEIVE_INITIAL,
  XMODEM_RECEIVE_SEND_C, XMODEM_RECEIVE_WAIT_FOR_ACK,
  XMODEM_RECEIVE_TIMEOUT_ACK, XMODEM_RECEIVE_READ_BLOCK_TIMEOUT,
  XMODEM_RECEIVE_ABORT_TRANSFER, XMODEM_RECEIVE_READ_BLOCK,
  XMODEM_RECEIVE_ACK_SUCCESS, XMODEM_RECEIVE_TRANSFER_COMPLETE,
  XMODEM_RECEIVE_READ_BLOCK_SUCCESS, XMODEM_RECEIVE_BLOCK_INVALID,
  XMODEM_RECEIVE_BLOCK_ACK, XMODEM_RECEIVE_BLOCK_VALID,
  XMODEM_RECEIVE_UNKNOWN
};
typedef enum XMODEM_RECEIVE_STATES xmodem_receive_state_t;

xmodem_receive_state_t xmodem_receive_state();

uint8_t xmodem_receive_init();
uint8_t xmodem_receive_process(const uint32_t current_time);
uint8_t xmodem_receive_cleanup();

void
xmodem_receive_set_callback_write(uint8_t(*callback)
                                  (const uint32_t requested_size,
                                   uint8_t * buffer, uint8_t * write_status));
void
xmodem_receive_set_callback_read(uint8_t(*callback)
                                 (const uint32_t requested_size,
                                  uint8_t * buffer, uint32_t * returned_size));
void xmodem_receive_set_callback_is_outbound_full(uint8_t(*callback) ());
void xmodem_receive_set_callback_is_inbound_empty(uint8_t(*callback) ());
void xmodem_receive_set_callback_set_buffer(uint8_t(*callback)
                                                (const uint32_t position,
                                                 uint8_t * buff));

#endif
