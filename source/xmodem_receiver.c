#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "xmodem.h"
#include "xmodem_receiver.h"

static bool (*callback_is_inbound_empty)();
static bool (*callback_is_outbound_full)();
static bool (*callback_read_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);
static bool (*callback_write_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);

static xmodem_receive_state_t receive_state;

static const uint32_t  TRANSFER_ACK_TIMEOUT    = 60000; // 60 seconds
static const uint32_t  READ_BLOCK_TIMEOUT      = 60000; // 60 seconds
static uint8_t         control_character       = 0;
static uint32_t        returned_size           = 0;
static uint8_t         inbound                 = 0;
static uint8_t         *payload_buffer         = 0;
static uint32_t        payload_buffer_position = 0;
static uint32_t        payload_size            = 0;
static uint8_t         current_packet_id       = 0;
static xmodem_packet_t current_packet;


xmodem_receive_state_t xmodem_receive_state()
{
   return receive_state;
}


bool xmodem_receive_init()
{
  
   bool result          = false; 
   receive_state        = XMODEM_RECEIVE_UNKNOWN;

   if (0 != callback_is_inbound_empty &&
       0 != callback_is_outbound_full  &&
       0 != callback_read_data &&
       0 != callback_write_data)
   {
      receive_state   = XMODEM_RECEIVE_INITIAL;
      result = true;
   }

   return result;
}

bool xmodem_receiver_cleanup()
{
   callback_is_inbound_empty = 0;
   callback_is_outbound_full = 0;
   callback_read_data        = 0;
   callback_write_data       = 0;
   receive_state             = XMODEM_RECEIVE_UNKNOWN;
   payload_buffer_position   = 0;
   payload_buffer            = 0;
   inbound                   = 0;
   returned_size             = 0;
   control_character         = 0;

   return true;
}


bool xmodem_receive_process(const uint32_t current_time)
{
   static uint32_t stopwatch = 0;

   switch(receive_state)
   {

      case XMODEM_RECEIVE_INITIAL:
      {
         receive_state = XMODEM_RECEIVE_WAIT_FOR_NACK;
         break;
      }

      case XMODEM_RECEIVE_WAIT_FOR_NACK:
      {
         receive_state = XMODEM_RECEIVE_SEND_REQUEST_FOR_TRANSFER;
         break;
      }

      case XMODEM_RECEIVE_UNKNOWN:
      {
          receive_state = XMODEM_RECEIVE_ABORT_TRANSFER;
          break;
      }

      default:
      {
          receive_state = XMODEM_RECEIVE_UNKNOWN; 
      }

   };

   return false;
    
}



void xmodem_receiver_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size))
{
   callback_write_data = callback;
}

void xmodem_receiver_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size))
{
   callback_read_data = callback;
}

void xmodem_receiver_set_callback_is_outbound_full(bool (*callback)())
{
   callback_is_outbound_full = callback;
}

void xmodem_receiver_set_callback_is_inbound_empty(bool (*callback)())
{
   callback_is_inbound_empty = callback;
}








