#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "xmodem.h"


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

    crc_status = xmodem_calculate_crc(packet.data, XMODEM_BLOCK_SIZE, &calculated_crc);

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



