#include "unity.h"
#include "xmodem.h"
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_crc_calculation(void)
{
   uint8_t    buffer[10];
   uint16_t   result;
 
   memset(buffer, 0, 10);
   memcpy(buffer,"helloworld", 10);
   TEST_ASSERT_EQUAL(true, xmodem_calculate_crc(buffer, 10, &result));
   TEST_ASSERT_EQUAL(0x4AB3, result); //reference value 0x4AB3 calculated here: http://www.tahapaksu.com/crc/

   memset(buffer, 0, 10);
   memcpy(buffer,"123456789", 9);
   TEST_ASSERT_EQUAL(true, xmodem_calculate_crc(buffer, 9, &result));
   TEST_ASSERT_EQUAL(0x31C3, result); //reference value 0x31C3 calculated here: http://www.tahapaksu.com/crc/
}

void test_xmodem_verify_packet(void)
{
   uint8_t          buffer[20];
   xmodem_packet_t  p;

   memset(&p, 0, sizeof(xmodem_packet_t));  

   for (uint8_t i = 0; i < 9; ++i)
   {
      p.data[i] = i+1;
   }
//this needs to be fixed to verify against a full 128 byte packet
 #if 1
   TEST_ASSERT_EQUAL(false, xmodem_verify_packet(p, 1));

   p.preamble = SOH;
   TEST_ASSERT_EQUAL(false, xmodem_verify_packet(p, 1));

   p.id = 1;
   TEST_ASSERT_EQUAL(false, xmodem_verify_packet(p, 1));

   p.id_complement = 0xFF - p.id;
   p.crc           = 0x9B66;
   TEST_ASSERT_EQUAL(true, xmodem_verify_packet(p, 1));

   p.crc       = 0xBB3D; 
   TEST_ASSERT_EQUAL(false, xmodem_verify_packet(p, 1));

   memset(buffer, 0, 10);
   memcpy(buffer,p.data, 9);

   p.crc       = 0x9B66; // expected value
   TEST_ASSERT_EQUAL(true, xmodem_verify_packet(p, 1));
#endif
}
