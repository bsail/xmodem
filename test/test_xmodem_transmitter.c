#include "unity.h"
#include "xmodem_transmitter.h"
#include "xmodem.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define INBOUND_BUFFER_SIZE  1024
#define OUTBOUND_BUFFER_SIZE 1024
#define BUFFER_SIZE          1024

/* Setup dummy functions for mocking out callbacks */
static uint8_t     transmitter_inbound_empty           = false;
static uint8_t     transmitter_result_inbound_buffer   = false;
static uint8_t     transmitter_returned_write_success  = false;
static uint32_t transmitter_returned_inbound_size   = 0;
static uint8_t  transmitter_inbound_buffer[INBOUND_BUFFER_SIZE];
static uint32_t transmitter_requested_inbound_size  = 0;

static uint8_t     transmitter_outbound_full           = false;
static uint8_t     transmitter_result_outbound_buffer  = false;
static uint32_t transmitter_returned_outbound_size  = 0;
static uint8_t  transmitter_outbound_buffer[OUTBOUND_BUFFER_SIZE];
static uint8_t transmitter_outbound_buffer_position = 0;
static uint32_t transmitter_requested_outbound_size = 0;
static uint8_t  transmitter_block_counter           = 0;
static uint8_t  transmitter_tmp                     = 0;
static uint8_t  transmitter_buffer[BUFFER_SIZE];
static uint32_t transmitter_buffer_position         = 0;
static uint32_t transmitter_timer                   = 0;
static uint8_t  transmitter_packet_number           = 0;

static uint8_t transmitter_get_buffer(const uint32_t position,
                                      uint8_t * buffer)
{
  memcpy(buffer,&(transmitter_buffer[position]),XMODEM_BLOCK_SIZE);
}

static uint8_t transmitter_write_data(const uint32_t requested_size, uint8_t *buffer, uint8_t *write_success)
{
   transmitter_requested_outbound_size = requested_size;
   memcpy(&(transmitter_outbound_buffer[transmitter_outbound_buffer_position]), buffer, requested_size);
   transmitter_outbound_buffer_position += requested_size;
   *write_success = transmitter_returned_write_success;
   return transmitter_result_outbound_buffer;
}

static uint8_t transmitter_is_outbound_full()
{
   return transmitter_outbound_full;
}

static uint8_t transmitter_is_inbound_empty()
{
   return transmitter_inbound_empty;
}

static uint8_t transmitter_read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   transmitter_requested_inbound_size = requested_size;
   memcpy(buffer, transmitter_inbound_buffer, requested_size); // maybe swap requested_size with returned_size
   *returned_size = transmitter_returned_inbound_size;
   return transmitter_result_inbound_buffer;
}

uint8_t generate_document(uint8_t* const storage, const uint32_t length )
{
   uint8_t result = false;

   srand(time(0));
   memset((void*)storage, 0, (size_t)length);

   if (0 != storage)
   {
      for (uint32_t i = 0; i < length; i++)
      {
         storage[i] = rand()%255;
      }
   }

   return result;
}

void setUp(void)
{
  memset(transmitter_inbound_buffer, 0, INBOUND_BUFFER_SIZE);
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

  transmitter_inbound_empty           = false;
  transmitter_result_inbound_buffer   = false; 
  transmitter_returned_inbound_size   = 0; 
  transmitter_requested_inbound_size  = 0; 
  transmitter_outbound_full           = false;
  transmitter_result_outbound_buffer  = false;
  transmitter_returned_outbound_size  = 0;
  transmitter_requested_outbound_size = 0;
  transmitter_block_counter           = 0;
  transmitter_outbound_buffer_position = 0;

  /* generate random document to send */
  generate_document(transmitter_buffer, 1024);
  transmitter_buffer_position         = 0;
  transmitter_timer                   = 0;
  transmitter_packet_number           = 1; //xmodem counts from 1 
}

void tearDown(void)
{
}

void test_XMODEM_TRANSMIT_TIMEOUT_WAIT_WRITE_BLOCK_SINGLE_CYCLE(void)
{
 
 
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(10));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  // transmitter_returned_inbound_size = 1; 
  // transmitter_inbound_buffer[0] = C;

  // // attempt to send a SOH control character, but the outbound buffer is full 
  // transmitter_returned_outbound_size = 0;

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(11));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(12));
 
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(59999+10));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(60000+11));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(60001+11));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT, xmodem_transmit_state());

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(60001+12));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(60001+12));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_TIMEOUT_WAIT_WRITE_BLOCK_MULTI_CYCLE()
{
 

  uint32_t current_time = 0;
 
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  current_time = 10;
  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  // transmitter_returned_inbound_size = 1; 
  // transmitter_inbound_buffer[0] = C;
  // transmitter_returned_write_success = false;

  // attempt to send a SOH control character, but the outbound buffer is full 
  // transmitter_returned_outbound_size = 0;

  current_time = 11;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

  current_time = 11;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));

  uint8_t test_retries = 0;

  // retry after 10 timeouts
  for (test_retries = 0; test_retries < 11; ++test_retries)
  {
    current_time = current_time+(59999);
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

    ++current_time;
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

    ++current_time;
    ++current_time;
  
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT, xmodem_transmit_state());

    ++current_time;
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  }

          // after 10th timeout, transfer is aborted

    current_time = current_time+(59999);
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

    ++current_time;
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());

    ++current_time;
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_TIMEOUT, xmodem_transmit_state());

    ++current_time;
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

          // abort is final state
    TEST_ASSERT_EQUAL(true, xmodem_transmit_process(current_time+100000));
    TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());


  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = false;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  
  // transmitter_outbound_buffer_position = 0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(133,transmitter_outbound_buffer_position);
  TEST_ASSERT_EQUAL(SOH, transmitter_outbound_buffer[0]);
  TEST_ASSERT_EQUAL(transmitter_packet_number, transmitter_outbound_buffer[1]);
  TEST_ASSERT_EQUAL(0xFF - transmitter_packet_number, transmitter_outbound_buffer[2]);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  transmitter_outbound_buffer_position = 0;

  ++transmitter_timer;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = ACK;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_ETB_NACK(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = false;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  transmitter_outbound_buffer_position = 0;

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], EOT);

  // // process again without an ACK but do not timeout
  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // // process an ACK
  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = ACK;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = NACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_ETB_MAX_RETRIES(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = false;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  transmitter_outbound_buffer_position = 0;

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // // process an ACK
  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = ACK;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  // for (uint8_t i = 0; i < 5; ++i)
  // {

  //   ++transmitter_timer;
  //   transmitter_inbound_buffer[0] = 0x0;
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  //   TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  //   transmitter_timer = transmitter_timer + 9999;
  //   transmitter_inbound_buffer[0] = 0x0;
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  //   TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  //   ++transmitter_timer;
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  //   TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  //   ++transmitter_timer;
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_TIMEOUT_ETB, xmodem_transmit_state());

  //   ++transmitter_timer;
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());
  // }


  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  // transmitter_timer = transmitter_timer + 9999;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  // ++transmitter_timer;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  // ++transmitter_timer;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_TIMEOUT_ETB, xmodem_transmit_state());

  // ++transmitter_timer;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());


  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_NACK_EOT(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = false;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = NACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK_FAILED, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 
}

void test_XMODEM_TRANSMIT_REINIT(void)
{
  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  xmodem_transmitter_cleanup();  

  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

}

void test_XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_EOT_TIMEOUT(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = false;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  transmitter_outbound_buffer_position = 0;

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], EOT); // verify that EOT is written to the outbound buffer
  
  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  transmitter_timer = transmitter_timer + 9997;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_TIMEOUT_EOT, xmodem_transmit_state());

TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

  // clear outbound buffer on each iteration
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);
  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_WRITE_SINGLE_BLOCK_DOCUMENT_EOT_NACK(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = false;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  ++transmitter_timer;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  
  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // process an ACK
  transmitter_timer = transmitter_timer + 9997;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // edge base, just before a potential timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // receiver responds with NACK
  transmitter_inbound_buffer[0] = NACK;
 
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_ABORT_TRANSFER, xmodem_transmit_state());

  // clear outbound buffer on each iteration
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);
  xmodem_transmitter_cleanup(); 

}

void test_XMODEM_TRANSMIT_WRITE_2_BLOCK_DOCUMENT(void)
{
  
  TEST_ASSERT_EQUAL(false, xmodem_transmit_init(BUFFER_SIZE));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_UNKNOWN, xmodem_transmit_state());

  xmodem_transmitter_set_callback_write(&transmitter_write_data);
  xmodem_transmitter_set_callback_read(&transmitter_read_data);
  xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
  xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);
  xmodem_transmitter_set_callback_get_buffer(&transmitter_get_buffer);

  TEST_ASSERT_EQUAL(true, xmodem_transmit_init(2*XMODEM_BLOCK_SIZE)); // send only a single block
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_INITIAL, xmodem_transmit_state());

  // for (int i = 0; i < 20; ++i)
  // {  
  //   TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));
  //   TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());
  // }

  transmitter_returned_inbound_size  = 1; 
  transmitter_inbound_buffer[0]      = C;
  transmitter_outbound_full          = false;
  transmitter_inbound_empty          = false;
  transmitter_returned_outbound_size = 1;
  transmitter_timer                  = 1;

  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C, xmodem_transmit_state());  
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  // write and fail as output buffer is full
  transmitter_returned_write_success = false;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(0));

  transmitter_returned_write_success = true;

  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  // clear outbound buffer
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

  // write first block of data
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());

  // check data is written to outbound buffer
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  // clear outbound buffer
  memset(transmitter_outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());


  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));


  // write second block of data
  ++transmitter_packet_number;
  transmitter_buffer_position = transmitter_buffer_position + XMODEM_BLOCK_SIZE;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_BLOCK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));


  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], SOH);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[1], transmitter_packet_number);
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[2], 0xFF - transmitter_packet_number);
  TEST_ASSERT_EQUAL(0, memcmp(transmitter_outbound_buffer+3, transmitter_buffer+transmitter_buffer_position, XMODEM_BLOCK_SIZE));

  transmitter_outbound_buffer_position = 0;

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0;
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_C_ACK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_C_ACK_RECEIVED, xmodem_transmit_state());

  // finished writing data
  ++transmitter_timer;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_EOT, xmodem_transmit_state());

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());
  TEST_ASSERT_EQUAL(transmitter_outbound_buffer[0], EOT);

  // process again without an ACK but do not timeout
  ++transmitter_timer;
  transmitter_inbound_buffer[0] = 0x0;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_EOT_ACK, xmodem_transmit_state());

  // // process an ACK
  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = ACK;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WRITE_ETB, xmodem_transmit_state());


  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  // ++transmitter_timer;
  // transmitter_inbound_buffer[0] = 0x0;
  // TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  // TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_WAIT_FOR_ETB_ACK, xmodem_transmit_state());
  // TEST_ASSERT_EQUAL(ETB, transmitter_outbound_buffer[0]);

  ++transmitter_timer;
  transmitter_inbound_buffer[0] = ACK;
  TEST_ASSERT_EQUAL(true, xmodem_transmit_process(transmitter_timer));
  TEST_ASSERT_EQUAL(XMODEM_TRANSMIT_COMPLETE, xmodem_transmit_state());

  xmodem_transmitter_cleanup(); 

}


