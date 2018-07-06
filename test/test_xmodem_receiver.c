#include "unity.h"
#include "xmodem_receiver.h"
#include "xmodem.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define INBOUND_BUFFER_SIZE  1024
#define OUTBOUND_BUFFER_SIZE 1024
#define BUFFER_SIZE          1024

static uint8_t     receiver_inbound_empty           = false;
static uint8_t     receiver_result_inbound_buffer   = false;
static uint8_t     receiver_returned_write_success  = false;
static uint32_t receiver_returned_inbound_size   = 0;
static uint8_t  receiver_inbound_buffer[INBOUND_BUFFER_SIZE];
static uint32_t receiver_requested_inbound_size  = 0;

static uint8_t     receiver_outbound_full           = false;
static uint8_t     receiver_result_outbound_buffer  = false;
// static uint32_t receiver_returned_outbound_size  = 0;
static uint8_t  receiver_outbound_buffer[OUTBOUND_BUFFER_SIZE];
static uint32_t receiver_requested_outbound_size = 0;
// static uint8_t  receiver_block_counter           = 0;
// static uint8_t  receiver_tmp                     = 0;
static uint8_t  receiver_buffer[BUFFER_SIZE];
// static uint32_t receiver_buffer_position         = 0;
// static uint32_t receiver_timer                   = 0;
// static uint8_t  receiver_packet_number           = 0;

static uint8_t receiver_set_buffer_file(const uint32_t position,
                                           uint8_t * buffer)
{
  memcpy(buffer,&(receiver_buffer[position]),XMODEM_BLOCK_SIZE);
  return 0;
}

static uint8_t receiver_is_outbound_full()
{
   return receiver_outbound_full;
}

static uint8_t receiver_is_inbound_empty()
{
   return receiver_inbound_empty;
}

static uint8_t receiver_read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   receiver_requested_inbound_size = requested_size;
   memcpy(buffer, receiver_inbound_buffer, requested_size); 
   *returned_size = receiver_returned_inbound_size;
   return receiver_result_inbound_buffer;
}

static uint8_t receiver_write_data(const uint32_t requested_size, uint8_t *buffer, uint8_t *write_success)
{
   receiver_requested_outbound_size = requested_size;
   memcpy(receiver_outbound_buffer, buffer, requested_size);
   *write_success = receiver_returned_write_success;
   return receiver_result_outbound_buffer;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_XMODEM_RECEIVE_SUCCESS_READ_BLOCK(void)
{
  xmodem_receive_set_callback_write(&receiver_write_data);
  xmodem_receive_set_callback_read(&receiver_read_data);
  xmodem_receive_set_callback_is_outbound_full(&receiver_is_outbound_full);
  xmodem_receive_set_callback_is_inbound_empty(&receiver_is_inbound_empty);
  xmodem_receive_set_callback_set_buffer(&receiver_set_buffer_file);

  TEST_ASSERT_EQUAL(true, xmodem_receive_init());
  TEST_ASSERT_EQUAL(XMODEM_RECEIVE_INITIAL, xmodem_receive_state());

  TEST_ASSERT_EQUAL(true, xmodem_receive_process(0));
  TEST_ASSERT_EQUAL(XMODEM_RECEIVE_SEND_C, xmodem_receive_state());

  receiver_returned_inbound_size = 1; 
  receiver_outbound_buffer[0] = C;

#if 0
  // check that a SOH control character is sent to the receiver
  returned_outbound_size = 1;
  outbound_full = false;
  tmp = SOH;
  
  TEST_ASSERT_EQUAL(true, xmodem_process(1));
  TEST_ASSERT_EQUAL(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  TEST_ASSERT_EQUAL(0, memcmp(outbound_buffer, &tmp, 1));

  TEST_ASSERT_EQUAL(true, xmodem_process(1));
  TEST_ASSERT_EQUAL(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  TEST_ASSERT_EQUAL(true, xmodem_process(3));

  /* setup mock receive buffer */
  inbound_empty         = false; 
  result_inbound_buffer = true;
  returned_inbound_size = 1;
  inbound_buffer[0]     = ACK;
  
  TEST_ASSERT_EQUAL(true, xmodem_process(3));
  TEST_ASSERT_EQUAL(XMODEM_TRANSFER_ACK_RECEIVED, xmodem_state());
  TEST_ASSERT_EQUAL(true, xmodem_process(3));
  TEST_ASSERT_EQUAL(XMODEM_READ_BLOCK, xmodem_state());   
  TEST_ASSERT_EQUAL(true, xmodem_process(60001));
  TEST_ASSERT_EQUAL(XMODEM_READ_BLOCK, xmodem_state());   
  TEST_ASSERT_EQUAL(true, xmodem_process(60002));
#endif
#if 0
  TEST_ASSERT_EQUAL(XMODEM_TIMEOUT_WAIT_READ_BLOCK, xmodem_state());
  TEST_ASSERT_EQUAL(true, xmodem_process(60004));
  TEST_ASSERT_EQUAL(XMODEM_ABORT_TRANSFER, xmodem_state());  
  TEST_ASSERT_EQUAL(true, xmodem_process(60005));
 
  tmp = CAN; 
  TEST_ASSERT_EQUAL(0, memcmp(outbound_buffer, &tmp, 1));
#endif

  TEST_ASSERT_EQUAL(true, xmodem_receive_cleanup());

}

void test_XMODEM_RECEIVE_ABORT_TRANSFER(void)
{
  xmodem_receive_set_callback_write(&receiver_write_data);
  xmodem_receive_set_callback_read(&receiver_read_data);
  xmodem_receive_set_callback_is_outbound_full(&receiver_is_outbound_full);
  xmodem_receive_set_callback_is_inbound_empty(&receiver_is_inbound_empty);
  xmodem_receive_set_callback_set_buffer(&receiver_set_buffer_file);

  TEST_ASSERT_EQUAL(true, xmodem_receive_init());
  TEST_ASSERT_EQUAL(XMODEM_RECEIVE_INITIAL, xmodem_receive_state());

  // uint32_t timestamp = 0;

  //TODO: Implement unit tests here

  TEST_ASSERT_EQUAL(true, xmodem_receive_cleanup());
}

