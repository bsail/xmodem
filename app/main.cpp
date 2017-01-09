#include <stdint.h>
#include "xmodem/xmodem.h"
#include "xmodem/xmodem_receiver.h"
#include "xmodem/xmodem_transmitter.h"
#include "serialport/libserialport.h"
#include "docopt/docopt.h"
#include <iostream>

int main (int argc, char **argv )
{

static const char USAGE[] =
R"(nanoXmodem.

    Usage:
      x --port=<port node>
      x --baud=<baud rate>
      x --receive
      x --transmit --file=foo
      x (-h | --help)
      x --version

    Options:
      -h --help           Show this screen.
      --version           Show version.
      --port=<comport>    Path to comport
      --baud=<baudrate>   9600,19200,38400,57600,115200
      --receive           Start transfer as receiver
      --transmit          Start file transmission
      --file=<filename>   File to send
)";

   std::map<std::string, docopt::value> args
        = docopt::docopt(USAGE,
                         { argv + 1, argv + argc },
                         true,                // show help if requested
                         "nanoeXmodem 0.1");  // version string

   for(auto const& arg : args) 
   {
        std::cout << arg.first <<  arg.second << std::endl;
   }   
  return 0;
}
