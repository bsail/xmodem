/* This is a personal academic project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "xmodem.h"
#include "xmodem_transmitter.h"

// private functions
static uint8_t(*callback_is_inbound_empty) ();
static uint8_t(*callback_is_outbound_full) ();
static uint8_t(*callback_read_data) (const uint32_t requested_size,
                                     uint8_t * buffer,
                                     uint32_t * returned_size);
static uint8_t(*callback_write_data) (const uint32_t requested_size,
                                      uint8_t * buffer,
                                      uint8_t * write_success);
static uint8_t(*callback_get_buffer) (const uint32_t position,
                                      uint8_t * buffer);

// private variables
static xmodem_transmit_state_t transmit_state;
static const uint32_t TRANSFER_ACK_TIMEOUT = 60000; // 60 seconds
static const uint32_t TRANSFER_EOT_TIMEOUT = 10000; // 10 seconds
static const uint32_t TRANSFER_ETB_TIMEOUT = 10000; // 10 seconds
static const uint32_t TRANSFER_WRITE_BLOCK_TIMEOUT = 60000; // 60 seconds
static const uint8_t WRITE_BLOCK_MAX_RETRIES = 10; // max 10 retries per block
static const uint8_t WRITE_ETB_MAX_RETRIES = 5; // max 5 retries for ETB ACK
static uint8_t control_character = 0;
static uint8_t write_success = false;
static uint32_t returned_size = 0;
static uint8_t inbound = 0;
static uint32_t payload_buffer_position = 0;
static uint32_t payload_size = 0;
static uint8_t current_packet_id = 0;
static uint8_t write_block_retries = 0;
static uint32_t write_block_timer = 0;
static uint8_t write_etb_retries = 0;
static xmodem_packet_t current_packet;

static uint8_t internal_buffer[XMODEM_BLOCK_SIZE];

xmodem_transmit_state_t xmodem_transmit_state()
{
  return transmit_state;
}

uint8_t xmodem_transmit_init(uint32_t size)
{

  uint8_t result = false;
  transmit_state = XMODEM_TRANSMIT_UNKNOWN;

  if (0 != callback_is_inbound_empty &&
      0 != callback_is_outbound_full &&
      0 != callback_read_data &&
      0 != callback_write_data && 0 != callback_get_buffer && 0 == size % 128) {
    transmit_state = XMODEM_TRANSMIT_INITIAL;
    result = true;
    payload_size = size;
    payload_buffer_position = 0;
    write_block_retries = 0;
    write_block_timer = 0;
    write_etb_retries = 0;
    memset(&current_packet, 0, sizeof(xmodem_packet_t));

    current_packet_id = 1;
    payload_buffer_position = 0;
    // write_block_timer       = current_time;
  }

  return result;
}

uint8_t xmodem_transmitter_cleanup()
{
  callback_is_inbound_empty = 0;
  callback_is_outbound_full = 0;
  callback_read_data = 0;
  callback_write_data = 0;
  callback_get_buffer = 0;
  transmit_state = XMODEM_TRANSMIT_UNKNOWN;
  payload_buffer_position = 0;
  inbound = 0;
  control_character = 0;
  write_block_retries = 0;
  write_block_timer = 0;
  write_success = false;
  returned_size = 0;
  return true;
}

uint8_t xmodem_transmit_process(const uint32_t current_time)
{

  static uint32_t stopwatch = 0;
  static uint32_t stopwatch_eot = 0;
  static uint32_t stopwatch_etb = 0;

  switch (transmit_state) {

  case XMODEM_TRANSMIT_INITIAL:
    {
      transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK;//XMODEM_TRANSMIT_WAIT_FOR_C;
      write_block_timer = current_time;
      break;
    }

  case XMODEM_TRANSMIT_WAIT_FOR_C:
    {
      if (!callback_is_inbound_empty()) {
        callback_read_data(1, &inbound, &returned_size);

        if (returned_size > 0 && C == inbound) {
          transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK;
          current_packet_id = 1;
          payload_buffer_position = 0;
          write_block_timer = current_time;
        }
      }
      break;
    }

  case XMODEM_TRANSMIT_WRITE_BLOCK:
    {
      if (current_time > (write_block_timer + TRANSFER_WRITE_BLOCK_TIMEOUT)) {
        transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT;
      } else                    //if ((payload_size / XMODEM_BLOCK_SIZE) >= current_packet_id)
      {
        /* setup current packet */
        current_packet.preamble = SOH;
        current_packet.id = current_packet_id;
        current_packet.id_complement = 0xFF - current_packet_id;
        callback_get_buffer(payload_buffer_position, internal_buffer);
        xmodem_calculate_crc(internal_buffer, XMODEM_BLOCK_SIZE,
                             &current_packet.crc);
        callback_write_data(1, (uint8_t *) & (current_packet.preamble),
                            &write_success);
        callback_write_data(1, (uint8_t *) & (current_packet.id),
                            &write_success);
        callback_write_data(1, (uint8_t *) & (current_packet.id_complement),
                            &write_success);
        callback_write_data(XMODEM_BLOCK_SIZE, internal_buffer, &write_success);
        uint8_t crc_high = (current_packet.crc) >> 8;
        uint8_t crc_low = (current_packet.crc) & (0x00FF);
        callback_write_data(1, (uint8_t *) & (crc_high), &write_success);
        callback_write_data(1, (uint8_t *) & (crc_low), &write_success);

        if (write_success)      // check if the output buffer had room
        {
          /* increment for next packet */
          ++current_packet_id;
          payload_buffer_position = payload_buffer_position + XMODEM_BLOCK_SIZE;
          transmit_state = XMODEM_TRANSMIT_WAIT_FOR_C_ACK; // end of document
        }

      }

      break;
    }

  case XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT:
    {
      transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK_FAILED;
      break;
    }

#if 0
  case XMODEM_TRANSMIT_SEND_REQUEST_FOR_TRANSFER:
    {
      static uint8_t outbound = SOH;
      static uint32_t delivered_size = 0;

      if (!callback_is_outbound_full()) //END_OF_TRANSFER_RECE
      {
        callback_write_data(1, &outbound, &delivered_size);

        if (0 < delivered_size) {
          transmit_state = XMODEM_TRANSMIT_WAIT_FOR_TRANSFER_ACK;
          stopwatch = current_time; // start the stopwatch to watch for a TRANSFER_ACK TIMEOUT
        }
      }
      break;
    }

#endif

  case XMODEM_TRANSMIT_WAIT_FOR_C_ACK:
    {
      if (current_time > (stopwatch + TRANSFER_ACK_TIMEOUT)) {
        transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK_FAILED;
      } else {

        if (!callback_is_inbound_empty()) {
          callback_read_data(1, &inbound, &returned_size);

          if (returned_size > 0) {
            if (ACK == inbound) {
              transmit_state = XMODEM_TRANSMIT_C_ACK_RECEIVED;
            } else if (NACK == inbound) {
              transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK_FAILED;
            } else if (EOT == inbound) {
              transmit_state = XMODEM_TRANSMIT_COMPLETE;
            }
          }
        }
      }

      break;
    }

  case XMODEM_TRANSMIT_WRITE_BLOCK_FAILED:
    {
      if (WRITE_BLOCK_MAX_RETRIES < write_block_retries) {
        transmit_state = XMODEM_TRANSMIT_ABORT_TRANSFER;
      } else {
        transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK;
        write_block_timer = current_time;
        ++write_block_retries;
        current_packet_id--;
        // payload_buffer_position -= XMODEM_BLOCK_SIZE;
      }
      break;
    }

  case XMODEM_TRANSMIT_ABORT_TRANSFER:
    {
      control_character = CAN;
      uint8_t result = false;
      callback_write_data(1, &control_character, &result);
      //final state
      break;
    }

  case XMODEM_TRANSMIT_C_ACK_RECEIVED:
    {
      if (payload_buffer_position >= payload_size) {
        transmit_state = XMODEM_TRANSMIT_WRITE_EOT;
      } else {
        transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK;
        write_block_retries = 0;
      }
      break;
    }

  case XMODEM_TRANSMIT_WRITE_EOT:
    {
      control_character = EOT;
      uint8_t result = false;
      callback_write_data(1, &control_character, &result);

      if (result) {
        transmit_state = XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK;
        stopwatch_eot = current_time;
      }
      break;
    }

  case XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK:
    {
      if (current_time > (stopwatch_eot + TRANSFER_EOT_TIMEOUT)) {
        transmit_state = XMODEM_TRANSMIT_TIMEOUT_EOT;
      } else {

        if (!callback_is_inbound_empty()) {
          callback_read_data(1, &inbound, &returned_size);

          if (returned_size > 0) {
            if (ACK == inbound) {
              transmit_state = XMODEM_TRANSMIT_COMPLETE; //XMODEM_TRANSMIT_WRITE_ETB;
            } else if (NACK == inbound) {
              transmit_state = XMODEM_TRANSMIT_ABORT_TRANSFER;
            }
          }
        }
      }
      break;
    }

  case XMODEM_TRANSMIT_TIMEOUT_EOT:
    {
      transmit_state = XMODEM_TRANSMIT_ABORT_TRANSFER;
      break;
    }

  case XMODEM_TRANSMIT_WRITE_ETB:
    {
      control_character = ETB;
      uint8_t result = false;
      callback_write_data(1, &control_character, &result);

      if (result) {
        transmit_state = XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK;
        stopwatch_etb = current_time;
      }
      break;
    }

  case XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK:
    {
      if (current_time > (stopwatch_etb + TRANSFER_ETB_TIMEOUT)) {
        transmit_state = XMODEM_TRANSMIT_TIMEOUT_ETB;
      } else {

        if (!callback_is_inbound_empty()) {
          callback_read_data(1, &inbound, &returned_size);

          if (returned_size > 0) {
            if (ACK == inbound) {
              transmit_state = XMODEM_TRANSMIT_COMPLETE;
            } else if (NACK == inbound) {
              transmit_state = XMODEM_TRANSMIT_ABORT_TRANSFER;
            }
          }
        }
      }

      break;
    }

  case XMODEM_TRANSMIT_TIMEOUT_ETB:
    {
      if (WRITE_ETB_MAX_RETRIES > write_etb_retries) {
        ++write_etb_retries;
        transmit_state = XMODEM_TRANSMIT_WRITE_ETB;
      } else {
        transmit_state = XMODEM_TRANSMIT_COMPLETE;
      }
    }//FALL THROUGH

  case XMODEM_TRANSMIT_COMPLETE:
    {
      control_character = EOT;
      callback_write_data(1, &control_character, &write_success);
      //final state
      break;
    }

  case XMODEM_TRANSMIT_UNKNOWN:
    {
      transmit_state = XMODEM_TRANSMIT_ABORT_TRANSFER;
      break;
    }

  default:
    {
      transmit_state = XMODEM_TRANSMIT_UNKNOWN;
    }

  };

  return true;
}

void xmodem_transmitter_set_callback_write(uint8_t(*callback)
                                           (const uint32_t requested_size,
                                            uint8_t * buffer,
                                            uint8_t * write_success))
{
  callback_write_data = callback;
}

void xmodem_transmitter_set_callback_read(uint8_t(*callback)
                                          (const uint32_t requested_size,
                                           uint8_t * buffer,
                                           uint32_t * returned_size))
{
  callback_read_data = callback;
}

void xmodem_transmitter_set_callback_is_outbound_full(uint8_t(*callback) ())
{
  callback_is_outbound_full = callback;
}

void xmodem_transmitter_set_callback_is_inbound_empty(uint8_t(*callback) ())
{
  callback_is_inbound_empty = callback;
}

void xmodem_transmitter_set_callback_get_buffer(uint8_t(*callback)
                                                (const uint32_t position,
                                                 uint8_t * buffer))
{
  callback_get_buffer = callback;
}
