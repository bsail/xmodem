#pragma once
#include "gtest/gtest.h"
#include <assert.h>
extern "C" {
#include "xmodem/xmodem.h"
}


static char buffer[1024];

static void sendserial(char* data, const size_t data_size)
{
    memset(buffer, 0, data_size);
    memcpy(buffer, data, data_size);
}



class XModemTestModel {

    public:
    uint8_t version;
/*    FIFO_BUFFER client_buffer;
    FIFO_BUFFER server_buffer; */
};


class XModemTests : public ::testing::Test {

public:

virtual void SetUp()
{

    memset(buffer, 0, 1024);
 
}

virtual void TearDown()
{
}

};


