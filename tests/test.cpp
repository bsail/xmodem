/* Test Suite for libxmodem */

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include "XModemTests.h"
#include <string.h>

TEST_F(XModemTests, XMODEM_INIT)
{
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
 
  EXPECT_EQ(true, xmodem_cleanup());

}


int main (int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void yyerror(char *s) {fprintf (stderr, "%s", s);}


