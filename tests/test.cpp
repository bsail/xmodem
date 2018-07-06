/* Test Suite for libxmodem */

#include <stdint.h>
#include <iostream>
#include <string.h>
#include "XModemTests.h"


static bool receiver_is_outbound_full()
{
   return receiver_outbound_full;
}

static bool receiver_is_inbound_empty()
{
   return receiver_inbound_empty;
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


