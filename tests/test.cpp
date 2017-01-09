/* Test Suite for libxmodem */

#include <stdint.h>
#include <iostream>
#include <string.h>
#include "XModemTests.h"


static bool transmitter_is_inbound_empty()
{
   return transmitter_inbound_empty;
}

static bool transmitter_is_outbound_full()
{
   return transmitter_outbound_full;
}

static bool transmitter_read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   transmitter_requested_inbound_size = requested_size;
   memcpy(transmitter_buffer, transmitter_inbound_buffer, requested_size); // maybe swap requested_size with returned_size
   *returned_size = transmitter_returned_inbound_size;
   return transmitter_result_inbound_buffer;
}

static bool transmitter_write_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   transmitter_requested_outbound_size = requested_size;
   memcpy(transmitter_outbound_buffer, transmitter_buffer, requested_size);
   *returned_size = transmitter_returned_outbound_size;
   return transmitter_result_outbound_buffer;
}


TEST_F(XModemTests, XMODEM_CRC_CALCULATION)
{

   uint8_t    buffer[10];
   uint16_t   result;
 
   memset(buffer, 0, 10);
   memcpy(buffer,"helloworld", 10);
   EXPECT_EQ(true, xmodem_calculate_crc(buffer, 10, &result));
   EXPECT_EQ(0x4AB3, result); //reference value 0x4AB3 calculated here: http://www.tahapaksu.com/crc/

   memset(buffer, 0, 10);
   memcpy(buffer,"123456789", 9);
   EXPECT_EQ(true, xmodem_calculate_crc(buffer, 9, &result));
   EXPECT_EQ(0x31C3, result); //reference value 0x31C3 calculated here: http://www.tahapaksu.com/crc/

}

TEST_F(XModemTests, XMODEM_VERIFY_PACKET)
{
   uint8_t          buffer[20];
   xmodem_packet_t  p;

   memset(&p, 0, sizeof(xmodem_packet_t));  

   for (uint8_t i = 0; i < 9; ++i)
   {
      p.data[i] = i+1;
   }
  
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   p.preamble = SOH;
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   p.id = 1;
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   p.id_complement = 0xFF - p.id;
   p.crc           = 0;
   p.data_size     = 0;
   EXPECT_EQ(true, xmodem_verify_packet(p, 1));

   p.crc       = 0xBB3D; 
   p.data_size = 0;
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   memset(buffer, 0, 10);
   memcpy(buffer,p.data, 9);

   p.crc       = 0x2378; // expected value
   p.data_size = 9;
   EXPECT_EQ(true, xmodem_verify_packet(p, 1));

}

TEST_F(XModemTests, XMODEM_TRANSMIT_TIMEOUT_WAIT_WRITE_BLOCK)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  for (int i = 0; i < 20; ++i)
  {  
    EXPECT_EQ(true, xmodem_transmit_process(0));
    EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  }

  transmitter_returned_inbound_size = 1; 
  transmitter_inbound_buffer[0] = C;

  // attempt to send a SOH control character, but the outbound buffer is full 
  transmitter_returned_outbound_size = 0;
  EXPECT_EQ(true, xmodem_transmit_process(1));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_transmit_process(2));
 
  xmodem_transmitter_cleanup(); 

}


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_BLOCK)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  for (int i = 0; i < 20; ++i)
  {  
    EXPECT_EQ(true, xmodem_transmit_process(0));
    EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  EXPECT_EQ(true, xmodem_transmit_process(0));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));


#if 0
  EXPECT_EQ(true, xmodem_transmit_process(timer));
  ++timer;
  EXPECT_EQ(outbound_buffer[0], SOH);
  EXPECT_EQ(outbound_buffer[1], packet_number);
  EXPECT_EQ(outbound_buffer[2], 0xFF - packet_number);
  EXPECT_EQ(0, memcmp(outbound_buffer+3, buffer+buffer_position, 128));

  // clear outbound buffer on each iteration
  memset(outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

  EXPECT_EQ(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());
#endif
  xmodem_transmitter_cleanup(); 

}

TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_DOCUMENT)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  for (int i = 0; i < 20; ++i)
  {  
    EXPECT_EQ(true, xmodem_transmit_process(0));
    EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_result_outbound_buffer = true;
  transmitter_result_inbound_buffer  = true;
  transmitter_returned_outbound_size = sizeof(xmodem_packet_t);
  transmitter_timer                  = 1;

  EXPECT_EQ(true, xmodem_transmit_process(0));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

 
     EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));

  while (xmodem_transmit_state() != XMODEM_TRANSMIT_COMPLETE &&
         xmodem_transmit_state() != XMODEM_TRANSMIT_ABORT_TRANSFER)
  {
     ++transmitter_timer;
     EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
     EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
     EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
     EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, 1));
     transmitter_buffer_position = transmitter_buffer_position + XMODEM_BLOCK_SIZE;

     // clear outbound buffer on each iteration
     memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);
     ++transmitter_packet_number;

     EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  }

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], EOT); 
  xmodem_transmitter_cleanup(); 

}


//#if 0

TEST_F(XModemTests, XMODEM_TIMEOUT_TRANSFER_ACK)
{

  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  EXPECT_EQ(false, xmodem_receive_init());
  EXPECT_EQ(XMODEM_RECEIVE_UNKNOWN, xmodem_receive_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);


  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_receive_init());
  EXPECT_EQ(XMODEM_RECEIVE_INITIAL, xmodem_receive_state());


  for (int i = 0; i < 20; ++i)
  {  
    EXPECT_EQ(true, xmodem_receive_process(0));
    EXPECT_EQ(XMODEM_RECEIVE_WAIT_FOR_NACK, xmodem_receive_state());
  }

  transmitter_returned_inbound_size = 1; 
  transmitter_inbound_buffer[0] = NACK;


#if 0
   
  // attempt to send a SOH control character, but the outbound buffer is full 
  returned_outbound_size = 0;
  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_SEND_REQUEST_FOR_TRANSFER, xmodem_state());

  // check that a SOH control character is sent to the receiver
  returned_outbound_size = 1;
  outbound_full = false;
  tmp = SOH;
  
  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));

  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());

  EXPECT_EQ(true, xmodem_process(2));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(3));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(59000));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60001));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60002));
  EXPECT_EQ(XMODEM_TIMEOUT_TRANSFER_ACK, xmodem_state());

  EXPECT_EQ(true, xmodem_process(60003));
  EXPECT_EQ(XMODEM_ABORT_TRANSFER, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60004));
 
  tmp = CAN; 
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));
  EXPECT_EQ(true, xmodem_cleanup());

#endif
}


#if 0
TEST_F(XModemTests, XMODEM_TIMEOUT_WAIT_READ_BLOCK)
{
  xmodem_set_callback_write(&write_data);
  xmodem_set_callback_read(&read_data);
  xmodem_set_callback_is_outbound_full(&is_outbound_full);
  xmodem_set_callback_is_inbound_empty(&is_inbound_empty);

  EXPECT_EQ(true, xmodem_init());
  EXPECT_EQ(XMODEM_INITIAL, xmodem_state());

  EXPECT_EQ(true, xmodem_process(0));
  EXPECT_EQ(XMODEM_WAIT_FOR_NACK, xmodem_state());

  returned_inbound_size = 1; 
  inbound_buffer[0] = NACK;

  EXPECT_EQ(true, xmodem_process(0));
  EXPECT_EQ(XMODEM_SEND_REQUEST_FOR_TRANSFER, xmodem_state());

  // check that a SOH control character is sent to the receiver
  returned_outbound_size = 1;
  outbound_full = false;
  tmp = SOH;
  
  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));

  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(3));

  /* setup mock receive buffer */
  inbound_empty         = false; 
  result_inbound_buffer = true;
  returned_inbound_size = 1;
  inbound_buffer[0]     = ACK;
  
  EXPECT_EQ(true, xmodem_process(3));
  EXPECT_EQ(XMODEM_TRANSFER_ACK_RECEIVED, xmodem_state());
  EXPECT_EQ(true, xmodem_process(3));
  EXPECT_EQ(XMODEM_READ_BLOCK, xmodem_state());   
  EXPECT_EQ(true, xmodem_process(60001));
  EXPECT_EQ(XMODEM_READ_BLOCK, xmodem_state());   
  EXPECT_EQ(true, xmodem_process(60002));
  EXPECT_EQ(XMODEM_TIMEOUT_WAIT_READ_BLOCK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60004));
  EXPECT_EQ(XMODEM_ABORT_TRANSFER, xmodem_state());  
  EXPECT_EQ(true, xmodem_process(60005));
 
  tmp = CAN; 
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));
  EXPECT_EQ(true, xmodem_cleanup());

}

#endif


#if 0

TEST_F(XModemTests, XMODEM_SUCCESS_READ_BLOCK)
{
  xmodem_set_callback_write(&write_data);
  xmodem_set_callback_read(&read_data);
  xmodem_set_callback_is_outbound_full(&is_outbound_full);
  xmodem_set_callback_is_inbound_empty(&is_inbound_empty);

  EXPECT_EQ(true, xmodem_init());
  EXPECT_EQ(XMODEM_INITIAL, xmodem_state());

  EXPECT_EQ(true, xmodem_process(0));
  EXPECT_EQ(XMODEM_WAIT_FOR_NACK, xmodem_state());

  returned_inbound_size = 1; 
  inbound_buffer[0] = NACK;

  EXPECT_EQ(true, xmodem_process(0));
  EXPECT_EQ(XMODEM_SEND_REQUEST_FOR_TRANSFER, xmodem_state());

  // check that a SOH control character is sent to the receiver
  returned_outbound_size = 1;
  outbound_full = false;
  tmp = SOH;
  
  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));

  EXPECT_EQ(true, xmodem_process(1));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(3));

  /* setup mock receive buffer */
  inbound_empty         = false; 
  result_inbound_buffer = true;
  returned_inbound_size = 1;
  inbound_buffer[0]     = ACK;
  
  EXPECT_EQ(true, xmodem_process(3));
  EXPECT_EQ(XMODEM_TRANSFER_ACK_RECEIVED, xmodem_state());
  EXPECT_EQ(true, xmodem_process(3));
  EXPECT_EQ(XMODEM_READ_BLOCK, xmodem_state());   
  EXPECT_EQ(true, xmodem_process(60001));
  EXPECT_EQ(XMODEM_READ_BLOCK, xmodem_state());   
  EXPECT_EQ(true, xmodem_process(60002));

#if 0
  EXPECT_EQ(XMODEM_TIMEOUT_WAIT_READ_BLOCK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60004));
  EXPECT_EQ(XMODEM_ABORT_TRANSFER, xmodem_state());  
  EXPECT_EQ(true, xmodem_process(60005));
 
  tmp = CAN; 
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));
#endif

  EXPECT_EQ(true, xmodem_cleanup());

}


#endif

int main (int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void yyerror(char *s) {fprintf (stderr, "%s", s);}


