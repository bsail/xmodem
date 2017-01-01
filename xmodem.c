#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "xmodem.h"


static bool (*callback_is_inbound_empty)();
static bool (*callback_is_outbound_full)();
static bool (*callback_read_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);
static bool (*callback_write_data)(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size);


static xmodem_transmit_state_t transmit_state;
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


bool xmodem_calculate_crc(const uint8_t *data, const uint32_t size, uint16_t *result)
{

   uint16_t crc    = 0x0;
   uint32_t count  = size;
   bool     status = false;
   uint8_t  i      = 0;

   if (0 != data && 0 != result)
   {
           status = true;

	   while (0 < count--)
	   {
	      crc = crc ^ (uint16_t) *data << 8;
              data++;
	      i = 8;

	      do
	      {
		  if (0x8000 & crc)
		  {
		     crc = crc << 1 ^ 0x1021;
		  }
		  else
		  {
		     crc = crc << 1;
		  }

	      } 
	      while (0 < --i);

	   }
           
           *result = crc;
   }

   return status;
}

bool xmodem_verify_packet(const xmodem_packet_t packet, uint8_t expected_packet_id)
{
    bool     status         = false;
    bool     crc_status     = false;
    uint16_t calculated_crc = 0;

    crc_status = xmodem_calculate_crc(packet.data, packet.data_size, &calculated_crc);

    if (packet.preamble == SOH &&
        packet.id == expected_packet_id &&
        packet.id_complement == 0xFF - packet.id &&
        crc_status &&
        calculated_crc == packet.crc)
    {
       status = true;
    }

    return status;
}


xmodem_transmit_state_t xmodem_transmit_state()
{
   return transmit_state;
}

xmodem_receive_state_t xmodem_receive_state()
{
   return receive_state;
}

bool xmodem_transmit_init(uint8_t *buffer, uint32_t size)
{
  
   bool result          = false; 
   transmit_state       = XMODEM_TRANSMIT_UNKNOWN;

   if (0 != callback_is_inbound_empty &&
       0 != callback_is_outbound_full  &&
       0 != callback_read_data &&
       0 != callback_write_data &&
       0 != buffer &&
       0 == size % 128)
   {
      transmit_state          = XMODEM_TRANSMIT_INITIAL;
      result                  = true;
      payload_size            = size;
      payload_buffer          = buffer;
      payload_buffer_position = 0;
      memset(&current_packet, 0, sizeof(xmodem_packet_t));
   }

   return result;
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

bool xmodem_cleanup()
{
   callback_is_inbound_empty = 0;
   callback_is_outbound_full = 0;
   callback_read_data        = 0;
   callback_write_data       = 0;
   receive_state             = XMODEM_RECEIVE_UNKNOWN;
   transmit_state            = XMODEM_TRANSMIT_UNKNOWN; 
   payload_buffer_position   = 0;
   payload_buffer            = 0;
   inbound                   = 0;
   returned_size             = 0;
   control_character         = 0;

   return true;
}

bool xmodem_transmit_process(const uint32_t current_time)
{

   static uint32_t stopwatch = 0;

   switch(transmit_state)
   {

      case XMODEM_TRANSMIT_INITIAL:
      {
        transmit_state = XMODEM_TRANSMIT_WAIT_FOR_C;
        break;
      }

      case XMODEM_TRANSMIT_WAIT_FOR_C:
      {
        if (!callback_is_inbound_empty())
        {
          callback_read_data(1, &inbound, &returned_size);

          if (returned_size > 0 && C == inbound)
          {
            transmit_state          = XMODEM_TRANSMIT_WRITE_BLOCK;
            current_packet_id       = 1;
            payload_buffer_position = 0;
          }
        }
        break;      
      }

      case XMODEM_TRANSMIT_WRITE_BLOCK:
      {
         if ((payload_size / XMODEM_BLOCK_SIZE) >= current_packet_id)
         {
            /* setup current packet */ 
	    current_packet.preamble      = SOH;
	    current_packet.id            = current_packet_id;
	    current_packet.id_complement = 0xFF - current_packet_id;
	    memcpy(current_packet.data, payload_buffer+payload_buffer_position, XMODEM_BLOCK_SIZE);
            xmodem_calculate_crc(current_packet.data, XMODEM_BLOCK_SIZE, &current_packet.crc);      

            /* write to output buffer */ 
            callback_write_data(sizeof(current_packet), &current_packet, &returned_size);  

            if (sizeof(current_packet) == returned_size) // check if the output buffer had room
            {
	       /* increment for next packet */
	       ++current_packet_id;        
	       payload_buffer_position = payload_buffer_position + XMODEM_BLOCK_SIZE;
	    }

         }
         else
         {
           transmit_state = XMODEM_TRANSMIT_COMPLETE; 
         }

         break;
      }

#if 0
      case XMODEM_TRANSMIT_SEND_REQUEST_FOR_TRANSFER:
      {
         static uint8_t   outbound       = SOH;
         static uint32_t  delivered_size = 0;

        if (!callback_is_outbound_full())
        {
            callback_write_data(1, &outbound, &delivered_size);

            if (0 < delivered_size)
            {
              transmit_state     = XMODEM_TRANSMIT_WAIT_FOR_TRANSFER_ACK;
              stopwatch = current_time;  // start the stopwatch to watch for a TRANSFER_ACK TIMEOUT
            }
        } 
        break;
      }

#endif

      case XMODEM_TRANSMIT_WAIT_FOR_TRANSFER_ACK:
      {
          if (current_time > (stopwatch + TRANSFER_ACK_TIMEOUT))
          {
             transmit_state = XMODEM_TRANSMIT_WRITE_BLOCK_FAILED;
          }
          else
          {
 
             if (!callback_is_inbound_empty())
             {
                callback_read_data(1, &inbound, &returned_size);

                if (returned_size > 0)
                {
                   if (ACK == inbound)
                   {
                       transmit_state = XMODEM_TRANSMIT_TRANSFER_ACK_RECEIVED;
                   }
                   else if (NACK == inbound)
                   {
                       //retry or abort
                   }
                   else if (EOT == inbound)
                   {
                       transmit_state = XMODEM_TRANSMIT_COMPLETE;
                   }
                } 
             } 
          }

          break;
      }

#if 0
      case XMODEM_TRANSMIT_TIMEOUT_WAIT_READ_BLOCK:
      {
          transmit_state = XMODEM_TRANSMIT_ABORT_TRANSFER;
          stopwatch = current_time;
          break;
      }

#endif

      case XMODEM_TRANSMIT_ABORT:
      {
          control_character = CAN; 
          callback_write_data(1, &control_character, &returned_size);  
          //final state
          break;
      }


#if 0
      case XMODEM_TRANSMIT_READ_BLOCK:
      {

          if (current_time > (stopwatch + READ_BLOCK_TIMEOUT))
          {
             transmit_state = XMODEM_TRANSMIT_TIMEOUT_WAIT_READ_BLOCK;
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
                       transmit_state = XMODEM_TRANSFER_ACK_RECEIVED;
                   }
                   else if (EOT == inbound)
                   {
                       transmit_state = XMODEM_TRANSFER_COMPLETE;
                   }
                } 
             } 
             //TODO: check for ACK or EOT in inbound buffer

          }
          break;
      }
#endif

      case XMODEM_TRANSMIT_TRANSFER_ACK_RECEIVED:
      {
            //decision, WRITE_BLOCK, END_OF_FILE
//          transmit_state = XMODEM_TRANSMIT_READ_BLOCK;
          break;
      }

      case XMODEM_TRANSMIT_COMPLETE:
      {
          control_character = EOT; 
          callback_write_data(1, &control_character, &returned_size);  
          //final state
          break;
      }

#if 0 
      case XMODEM_TRANSMIT_BLOCK_RECEIVED:
      {
          break;
      }

      case XMODEM_TRANSMIT_INVALID_BLOCK:
      {
          break;
      }

      case XMODEM_TRANSMIT_VALID_BLOCK:
      {
          transmit_state = XMODEM_TRANSMIT_ACK_BLOCK;
          break;
      }

      case XMODEM_TRANSMIT_ACK_BLOCK:
      {
          //TODO: send ACK
          stopwatch = current_time;  // start the stopwatch to watch for a TRANSFER_ACK TIMEOUT
          transmit_state = XMODEM_TRANSMIT_WAIT_FOR_TRANSFER_ACK;
          break;
      }

#endif

      case XMODEM_TRANSMIT_UNKNOWN:
      {
          transmit_state = XMODEM_TRANSMIT_ABORT;
          break;
      }

      default:
      {
          transmit_state = XMODEM_TRANSMIT_UNKNOWN; 
      }


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







