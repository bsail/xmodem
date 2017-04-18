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
   memcpy(buffer, transmitter_inbound_buffer, requested_size); // maybe swap requested_size with returned_size
   *returned_size = transmitter_returned_inbound_size;
   return transmitter_result_inbound_buffer;
}

static bool transmitter_write_data(const uint32_t requested_size, uint8_t *buffer, bool *write_success)
{
   transmitter_requested_outbound_size = requested_size;
   memcpy(transmitter_outbound_buffer, buffer, requested_size);
   *write_success = transmitter_returned_write_success;
   return transmitter_result_outbound_buffer;
}

static bool receiver_is_inbound_empty()
{
   return receiver_inbound_empty;
}

static bool receiver_is_outbound_full()
{
   return receiver_outbound_full;
}

static bool receiver_read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   receiver_requested_inbound_size = requested_size;
   memcpy(buffer, receiver_inbound_buffer, requested_size); 
   *returned_size = receiver_returned_inbound_size;
   return receiver_result_inbound_buffer;
}

static bool receiver_write_data(const uint32_t requested_size, uint8_t *buffer, bool *write_success)
{
   receiver_requested_outbound_size = requested_size;
   memcpy(receiver_outbound_buffer, buffer, requested_size);
   *write_success = receiver_returned_write_success;
   return receiver_result_outbound_buffer;
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
//this needs to be fixed to verify against a full 128 byte packet
 #if 1
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   p.preamble = SOH;
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   p.id = 1;
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   p.id_complement = 0xFF - p.id;
   p.crc           = 0x9B66;
   EXPECT_EQ(true, xmodem_verify_packet(p, 1));

   p.crc       = 0xBB3D; 
   EXPECT_EQ(false, xmodem_verify_packet(p, 1));

   memset(buffer, 0, 10);
   memcpy(buffer,p.data, 9);

   p.crc       = 0x9B66; // expected value
   EXPECT_EQ(true, xmodem_verify_packet(p, 1));
#endif
}

TEST_F(XModemTests, XMODEM_TRANSMIT_TIMEOUT_WAIT_WRITE_BLOCK_SINGLE_CYCLE)
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
    EXPECT_EQ(true, xmodem_transmit_process(10));
    EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  }

  transmitter_returned_inbound_size = 1; 
  transmitter_inbound_buffer[0] = C;

  // attempt to send a SOH control character, but the outbound buffer is full 
  transmitter_returned_outbound_size = 0;

  EXPECT_EQ(true, xmodem_transmit_process(11));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_transmit_process(12));
 
  EXPECT_EQ(true, xmodem_transmit_process(59999+10));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_transmit_process(60000+11));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_transmit_process(60001+11));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_transmit_process(60001+12));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

  EXPECT_EQ(true, xmodem_transmit_process(60001+12));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}


TEST_F(XModemTests, XMODEM_TRANSMIT_TIMEOUT_WAIT_WRITE_BLOCK_MULTI_CYCLE)
{
 

  uint32_t current_time = 0;
 
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  current_time = 10;
  for (int i = 0; i < 20; ++i)
  {  
    EXPECT_EQ(true, xmodem_transmit_process(current_time));
    EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  }

  transmitter_returned_inbound_size = 1; 
  transmitter_inbound_buffer[0] = C;
  transmitter_returned_write_success = false;

  // attempt to send a SOH control character, but the outbound buffer is full 
  transmitter_returned_outbound_size = 0;

  current_time = 11;
  EXPECT_EQ(true, xmodem_transmit_process(current_time));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  current_time = 11;
  EXPECT_EQ(true, xmodem_transmit_process(current_time));

  uint8_t test_retries = 0;

  // retry after 10 timeouts
  for (test_retries = 0; test_retries < 11; ++test_retries)
  {
	  current_time = current_time+(59999);
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

	  ++current_time;
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

	  ++current_time;
	  ++current_time;
	
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT, xmodem_transmit_state());

	  ++current_time;
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  }

          // after 10th timeout, transfer is aborted

	  current_time = current_time+(59999);
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

	  ++current_time;
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

	  ++current_time;
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT, xmodem_transmit_state());

	  ++current_time;
	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

	  EXPECT_EQ(true, xmodem_transmit_process(current_time));
	  EXPECT_EQ(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

          // abort is final state
	  EXPECT_EQ(true, xmodem_transmit_process(current_time+100000));
	  EXPECT_EQ(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());


  xmodem_transmitter_cleanup(); 

}


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
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

  transmitter_returned_write_success = false;
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_ETB_NACK)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
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

  transmitter_returned_write_success = false;
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = NACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}



TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_ETB_MAX_RETRIES)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
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

  transmitter_returned_write_success = false;
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  for (uint8_t i = 0; i < 5; ++i)
  {

	  ++transmitter_timer;
	  transmitter_inbound_buffer[0] = 0x0;
	  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
	  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
	  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

	  transmitter_timer = transmitter_timer + 9999;
	  transmitter_inbound_buffer[0] = 0x0;
	  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
	  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
	  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

	  ++transmitter_timer;
	  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
	  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
	  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

	  ++transmitter_timer;
	  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
	  EXPECT_EQ(XMODEM_TRANSMIT_TIMEOUT_ETB, xmodem_transmit_state());

	  ++transmitter_timer;
	  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
	  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());
  }


  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  transmitter_timer = transmitter_timer + 9999;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_TIMEOUT_ETB, xmodem_transmit_state());

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());


  xmodem_transmitter_cleanup(); 

}


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_NACK_EOT)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
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

  transmitter_returned_write_success = false;
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = NACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 
}

TEST_F(XModemTests, XMODEM_TRANSMIT_REINIT)
{
  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
  EXPECT_EQ(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  xmodem_transmitter_cleanup();  

  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

}


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_EOT_TIMEOUT)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
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

  transmitter_returned_write_success = false;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], EOT); // verify that EOT is written to the outbound buffer
  
  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  transmitter_timer = transmitter_timer + 9997;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_TIMEOUT_EOT, xmodem_transmit_state());

EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

  // clear outbound buffer on each iteration
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);
  xmodem_transmitter_cleanup(); 

}


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_EOT_NACK)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, XMODEM_BLOCK_SIZE)); // send only a single block
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

  transmitter_returned_write_success = false;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  
  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  transmitter_timer = transmitter_timer + 9997;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // edge base, just before a potential timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // receiver responds with NACK
  transmitter_inbound_buffer[0] = NACK;
 
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

  // clear outbound buffer on each iteration
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);
  xmodem_transmitter_cleanup(); 

}





#if 0
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
//     EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
 //    EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  //   EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
//     EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, 1));
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
#endif

#if 0

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


//#if 0
   
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

}
#endif


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


TEST_F(XModemTests, XMODEM_TRANSMIT_WRITE_2_BLOCK_DOCUMENT)
{
  
  EXPECT_EQ(false, xmodem_transmit_init(transmitter_buffer, BUFFER_SIZE));
  EXPECT_EQ(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

  EXPECT_EQ(true, xmodem_transmit_init(transmitter_buffer, 2*XMODEM_BLOCK_SIZE)); // send only a single block
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

  // write and fail as output buffer is full
  transmitter_returned_write_success = false;
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  // clear outbound buffer
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

  // write first block of data
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  // check data is written to outbound buffer
  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  // clear outbound buffer
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());


  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));


  // write second block of data
  ++transmitter_packet_number;
  transmitter_buffer_position = transmitter_buffer_position + XMODEM_BLOCK_SIZE;
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));


  EXPECT_EQ(transmitter_outbound_buffer[0], SOH);
  EXPECT_EQ(transmitter_outbound_buffer[1], transmitter_packet_number);
  EXPECT_EQ(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  EXPECT_EQ(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0;
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  // finished writing data
  ++transmitter_timer;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  EXPECT_EQ(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  EXPECT_EQ(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  EXPECT_EQ(true, xmodem_transmit_process(transmitter_timer));
  EXPECT_EQ(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}




TEST_F(XModemTests, XMODEM_RECEIVE_SUCCESS_READ_BLOCK)
{
  xmodem_receive_set_callback_write(&receiver_write_data);
  xmodem_receive_set_callback_read(&receiver_read_data);
  xmodem_receive_set_callback_is_outbound_full(&receiver_is_outbound_full);
  xmodem_receive_set_callback_is_inbound_empty(&receiver_is_inbound_empty);

  EXPECT_EQ(true, xmodem_receive_init());
  EXPECT_EQ(XMODEM_RECEIVE_INITIAL, xmodem_receive_state());

  EXPECT_EQ(true, xmodem_receive_process(0));
  EXPECT_EQ(XMODEM_RECEIVE_SEND_C, xmodem_receive_state());

  receiver_returned_inbound_size = 1; 
  receiver_outbound_buffer[0] = C;

#if 0
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
#endif
#if 0
  EXPECT_EQ(XMODEM_TIMEOUT_WAIT_READ_BLOCK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60004));
  EXPECT_EQ(XMODEM_ABORT_TRANSFER, xmodem_state());  
  EXPECT_EQ(true, xmodem_process(60005));
 
  tmp = CAN; 
  EXPECT_EQ(0, memcmp(outbound_buffer, &tmp, 1));
#endif

  EXPECT_EQ(true, xmodem_receive_cleanup());

}

TEST_F(XModemTests, XMODEM_RECEIVE_ABORT_TRANSFER)
{
  xmodem_receive_set_callback_write(&receiver_write_data);
  xmodem_receive_set_callback_read(&receiver_read_data);
  xmodem_receive_set_callback_is_outbound_full(&receiver_is_outbound_full);
  xmodem_receive_set_callback_is_inbound_empty(&receiver_is_inbound_empty);

  EXPECT_EQ(true, xmodem_receive_init());
  EXPECT_EQ(XMODEM_RECEIVE_INITIAL, xmodem_receive_state());

  uint32_t timestamp = 0;

  //TODO: Implement unit tests here

  EXPECT_EQ(true, xmodem_receive_cleanup());
}

int main (int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void yyerror(char *s) {fprintf (stderr, "%s", s);}


