#pragma once
#include "gtest/gtest.h"
#include <assert.h>
#include <iomanip>
#include <string.h>
extern "C" {
#include "xmodem/xmodem.h"
#include "xmodem/xmodem_transmitter.h"
#include "xmodem/xmodem_receiver.h"
}

#define INBOUND_BUFFER_SIZE  1024
#define OUTBOUND_BUFFER_SIZE 1024
#define BUFFER_SIZE          1024

/* Setup dummy functions for mocking out callbacks */
static bool     transmitter_inbound_empty           = false;
static bool     transmitter_result_inbound_buffer   = false;
static bool     transmitter_returned_write_success  = false;
static uint32_t transmitter_returned_inbound_size   = 0;
static uint8_t  transmitter_inbound_buffer[INBOUND_BUFFER_SIZE];
static uint32_t transmitter_requested_inbound_size  = 0;

static bool     transmitter_outbound_full           = false;
static bool     transmitter_result_outbound_buffer  = false;
static uint32_t transmitter_returned_outbound_size  = 0;
static uint8_t  transmitter_outbound_buffer[OUTBOUND_BUFFER_SIZE];
static uint32_t transmitter_requested_outbound_size = 0;
static uint8_t  transmitter_block_counter           = 0;
static uint8_t  transmitter_tmp                     = 0;
static uint8_t  transmitter_buffer[BUFFER_SIZE];
static uint32_t transmitter_buffer_position         = 0;
static uint32_t transmitter_timer                   = 0;
static uint8_t  transmitter_packet_number           = 0;

static bool     receiver_inbound_empty           = false;
static bool     receiver_result_inbound_buffer   = false;
static bool     receiver_returned_write_success  = false;
static uint32_t receiver_returned_inbound_size   = 0;
static uint8_t  receiver_inbound_buffer[INBOUND_BUFFER_SIZE];
static uint32_t receiver_requested_inbound_size  = 0;

static bool     receiver_outbound_full           = false;
static bool     receiver_result_outbound_buffer  = false;
static uint32_t receiver_returned_outbound_size  = 0;
static uint8_t  receiver_outbound_buffer[OUTBOUND_BUFFER_SIZE];
static uint32_t receiver_requested_outbound_size = 0;
static uint8_t  receiver_block_counter           = 0;
static uint8_t  receiver_tmp                     = 0;
static uint8_t  receiver_buffer[BUFFER_SIZE];
static uint32_t receiver_buffer_position         = 0;
static uint32_t receiver_timer                   = 0;
static uint8_t  receiver_packet_number           = 0;




static void sendserial(char* data, const size_t data_size)
{
//    memset(buffer, 0, data_size);
//    memcpy(buffer, data, data_size);
}



class XModemTestModel {

    public:
    uint8_t version;
/*    FIFO_BUFFER client_buffer;
    FIFO_BUFFER server_buffer; */
};


class XModemTests : public ::testing::Test {

public:

private:


bool generate_document(uint8_t* const storage, const uint32_t length )
{
   bool result = false;

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

virtual void SetUp()
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

    /* generate random document to send */
    generate_document(transmitter_buffer, 1024);
    transmitter_buffer_position         = 0;
    transmitter_timer                   = 0;
    transmitter_packet_number           = 1; //xmodem counts from 1 

}

virtual void TearDown()
{
}

};


