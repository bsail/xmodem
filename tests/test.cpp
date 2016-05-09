/* Test Suite for libxmodem */

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include "XModemTests.h"
#include <string.h>

TEST_F(XModemTests, XMODEM_INIT)
{
  EXPECT_EQ(true, xmodem_init());
  EXPECT_EQ(true, xmodem_process());
  EXPECT_EQ(true, xmodem_cleanup());

}


int main (int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void yyerror(char *s) {fprintf (stderr, "%s", s);}


