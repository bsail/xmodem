#include <stdint.h>
#include <stdbool.h>
#include "xmodem.h"


enum XMODEM_CONTROL_CHARACTERS {SOH = 0x01, EOT = 0x04, ACK = 0x06, NAK = 0x21, NAK_CRC = 0x67, CAN = 0x24}; 

static xmodem_state_t state;
static const uint32_t TRANSFER_ACK_TIMEOUT = 60000; //60 seconds

xmodem_state_t xmodem_state()
{
   return state;
}

bool xmodem_init()
{
   state = XMODEM_INITIAL;
   return true;
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



