#pragma once
#include "gtest/gtest.h"
#include <assert.h>
#include <iomanip>
#include <string.h>
extern "C" {
#include "xmodem/xmodem.h"
}

#define INBOUND_BUFFER_SIZE  1024
#define OUTBOUND_BUFFER_SIZE 1024
#define BUFFER_SIZE          1024

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
static uint8_t  block_counter           = 0;
static uint8_t  tmp = 0;
static uint8_t  buffer[BUFFER_SIZE];
static uint32_t buffer_position         = 0;
static uint32_t timer                   = 0;
static uint8_t  packet_number           = 0;



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

    memset(inbound_buffer, 0, INBOUND_BUFFER_SIZE);
    memset(outbound_buffer, 0, OUTBOUND_BUFFER_SIZE);

    inbound_empty           = false;
    result_inbound_buffer   = false; 
    returned_inbound_size   = 0; 
    requested_inbound_size  = 0; 
    outbound_full           = false;
    result_outbound_buffer  = false;
    returned_outbound_size  = 0;
    requested_outbound_size = 0;
    block_counter           = 0;

    /* generate random document to send */
    generate_document(buffer, 1024);
    buffer_position         = 0;
    timer                   = 0;
    packet_number           = 1; //xmodem counts from 1 

}

virtual void TearDown()
{
}

};


