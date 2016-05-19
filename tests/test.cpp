/* Test Suite for libxmodem */

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include "XModemTests.h"
#include <string.h>

#define INBOUND_BUFFER_SIZE  1024
#define OUTBOUND_BUFFER_SIZE 1024

/* Setup dummy functions for mocking out callbacks */
static bool     inbound_empty           = false;
static bool     result_inbound_buffer   = false;
static uint32_t returned_inbound_size   = 0;
static uint8_t  inbound_buffer[INBOUND_BUFFER_SIZE];
static uint32_t requested_inbound_size  = 0;

static bool     outbound_full           = false;
static bool     result_outbound_buffer  = false;
static uint32_t returned_outbound_size  = 0;
static uint8_t  outbound_buffer[OUTBOUND_BUFFER_SIZE];
static uint32_t requested_outbound_size = 0;

static bool is_inbound_empty()
{
   return inbound_empty;
}

static bool is_outbound_full()
{
   return outbound_full;
}

static bool read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   requested_inbound_size = requested_size;
   memcpy(buffer, inbound_buffer, requested_size); // maybe swap requested_size with returned_size
   *returned_size = returned_inbound_size;
   return result_inbound_buffer;
}

static bool write_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   requested_outbound_size = requested_size;
   memcpy(outbound_buffer, buffer, requested_size);
   *returned_size = returned_outbound_size;
   return result_outbound_buffer;
}

TEST_F(XModemTests, XMODEM_TIMEOUT_TRANSFER_ACK)
{
  EXPECT_EQ(false, xmodem_init());
  EXPECT_EQ(XMODEM_UNKNOWN, xmodem_state());

  xmodem_set_callback_write(&write_data);
  xmodem_set_callback_read(&read_data);
  xmodem_set_callback_is_outbound_full(&is_outbound_full);
  xmodem_set_callback_is_inbound_empty(&is_inbound_empty);

  EXPECT_EQ(true, xmodem_init());
  EXPECT_EQ(XMODEM_INITIAL, xmodem_state());
  EXPECT_EQ(true, xmodem_process(0));
  EXPECT_EQ(XMODEM_SEND_REQUEST_FOR_TRANSFER, xmodem_state());
  EXPECT_EQ(true, xmodem_process(2));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(3));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(59000));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60002));
  EXPECT_EQ(XMODEM_WAIT_FOR_TRANSFER_ACK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60003));
  EXPECT_EQ(XMODEM_TIMEOUT_TRANSFER_ACK, xmodem_state());
  //TODO: ABORT TRANSFER 
  EXPECT_EQ(true, xmodem_cleanup());

}


TEST_F(XModemTests, XMODEM_TIMEOUT_WAIT_READ_BLOCK)
{
  xmodem_set_callback_write(&write_data);
  xmodem_set_callback_read(&read_data);
  xmodem_set_callback_is_outbound_full(&is_outbound_full);
  xmodem_set_callback_is_inbound_empty(&is_inbound_empty);

  EXPECT_EQ(true, xmodem_init());
  EXPECT_EQ(XMODEM_INITIAL, xmodem_state());
  EXPECT_EQ(true, xmodem_process(0));
  EXPECT_EQ(XMODEM_SEND_REQUEST_FOR_TRANSFER, xmodem_state());
  EXPECT_EQ(true, xmodem_process(2));
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
  EXPECT_EQ(true, xmodem_process(60002));
  EXPECT_EQ(XMODEM_READ_BLOCK, xmodem_state());   
  EXPECT_EQ(true, xmodem_process(60003));
  EXPECT_EQ(XMODEM_TIMEOUT_WAIT_READ_BLOCK, xmodem_state());
  EXPECT_EQ(true, xmodem_process(60004));
  EXPECT_EQ(XMODEM_ABORT_TRANSFER, xmodem_state());  
   


  EXPECT_EQ(true, xmodem_cleanup());

}

int main (int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void yyerror(char *s) {fprintf (stderr, "%s", s);}


