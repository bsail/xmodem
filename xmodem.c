#include <stdint.h>
#include <stdbool.h>
#include "xmodem.h"

static bool (*callback_is_inbound_empty)();
static bool (*callback_is_outbound_full)();
static bool (*callback_read_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);
static bool (*callback_write_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);



static xmodem_state_t state;
static const uint32_t TRANSFER_ACK_TIMEOUT = 60000; //60 seconds

xmodem_state_t xmodem_state()
{
   return state;
}

bool xmodem_init()
{
  
   bool result = false; 
   state       = XMODEM_UNKNOWN;

   if (0 != callback_is_inbound_empty &&
       0 != callback_is_outbound_full  &&
       0 != callback_read_data &&
       0 != callback_write_data)
   {
      state  = XMODEM_INITIAL;
      result = true;
   }

   return result;
}

bool xmodem_cleanup()
{
   return true;
}

bool xmodem_process(const uint32_t current_time)
{

   static uint32_t stopwatch = 0;

   switch(state)
   {

      case XMODEM_INITIAL:
      {
        state = XMODEM_SEND_REQUEST_FOR_TRANSFER;
        break;
      }

      case XMODEM_SEND_REQUEST_FOR_TRANSFER:
      {
        state = XMODEM_WAIT_FOR_TRANSFER_ACK;
        //TODO: send request for transfer
        
        stopwatch = current_time;  // start the stopwatch to watch for a TRANSFER_ACK TIMEOUT
        break;
      }

      case XMODEM_WAIT_FOR_TRANSFER_ACK:
      {
          if (current_time > (stopwatch + TRANSFER_ACK_TIMEOUT))
          {
             state = XMODEM_TIMEOUT_TRANSFER_ACK;
          }
          else
          {
             uint8_t   inbound       = 0;
             uint32_t  returned_size = 0;
 
             if (!callback_is_inbound_empty())
             {
                callback_read_data(1, &inbound, &returned_size);

                if (returned_size > 0)
                {
                   if (ACK == inbound)
                   {
                       state = XMODEM_TRANSFER_ACK_RECEIVED;
                   }
                   else if (EOT == inbound)
                   {
                       state = XMODEM_TRANSFER_COMPLETE;
                   }
                } 
             } 
             //TODO: check for ACK or EOT in inbound buffer
          }

          break;
      }

      case XMODEM_TIMEOUT_TRANSFER_ACK:
      {
          state = XMODEM_ABORT_TRANSFER; 
          break;
      }

      case XMODEM_ABORT_TRANSFER:
      {
          //TODO: send abort 
          break;
      }

      case XMODEM_READ_BLOCK:
      {
          break;
      }

      case XMODEM_TRANSFER_ACK_RECEIVED:
      {
          break;
      }

      case XMODEM_TRANSFER_COMPLETE:
      {
          break;
      }
 
      case XMODEM_BLOCK_RECEIVED:
      {
          break;
      }

      case XMODEM_INVALID_BLOCK:
      {
          break;
      }

      case XMODEM_VALID_BLOCK:
      {
          state = XMODEM_ACK_BLOCK;
          break;
      }

      case XMODEM_ACK_BLOCK:
      {
          //TODO: send ACK
          stopwatch = current_time;  // start the stopwatch to watch for a TRANSFER_ACK TIMEOUT
          state = XMODEM_WAIT_FOR_TRANSFER_ACK;
          break;
      }

      case XMODEM_UNKNOWN:
      {
          state = XMODEM_ABORT_TRANSFER;
          break;
      }

      default:
      {
          state = XMODEM_UNKNOWN; 
      }

#if 0
enum XMODEM_STATES {XMODEM_INITIAL, XMODEM_SEND_REQUEST_FOR_TRANSFER, XMODEM_WAIT_FOR_TRANSFER_ACK, XMODEM_TIMEOUT_TRANSFER_ACK,
                    XMODEM_TIMEOUT_WAIT_READ_BLOCK, XMODEM_ABORT_TRANSFER, XMODEM_READ_BLOCK, XMODEM_TRANSFER_ACK_RECEIVED,
                    XMODEM_TRANSFER_COMPLETE, XMODEM_BLOCK_RECEIVED, XMODEM_INVALID_BLOCK, XMODEM_ACK_BLOCK, XMODEM_VALID_BLOCK} typedef xmodem_state_t;
#endif




   };

   return true;
}



void xmodem_set_callback_write(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size))
{
   callback_write_data = callback;
}

void xmodem_set_callback_read(bool (*callback)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size))
{
   callback_read_data = callback;
}

void xmodem_set_callback_is_outbound_full(bool (*callback)())
{
   callback_is_outbound_full = callback;
}

void xmodem_set_callback_is_inbound_empty(bool (*callback)())
{
   callback_is_inbound_empty = callback;
}


