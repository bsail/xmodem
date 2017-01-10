#include <stdint.h>
#include "xmodem/xmodem.h"
#include "xmodem/xmodem_receiver.h"
#include "xmodem/xmodem_transmitter.h"
#include "serialport/libserialport.h"
#include "docopt/docopt.h"
#include <iostream>
#include <string>


void transmit(std::string port, std::string baud)
{
   std::cout << "transmit: " << port << "," << baud << std::endl;
}

void receive(std::string port, std::string baud)
{
   std::cout << "receive: " << port << "," << baud << std::endl;
}

bool get_port(std::map<std::string, docopt::value> args, std::string &port)
{
  bool result = false;
  auto p = args.find("--port")->second;

  if (p.isString())
  {
     port   = p.asString();
     result = true;
  }

  return result;
}

bool get_baud(std::map<std::string, docopt::value> args, std::string &baud)
{
  bool result = false;
  auto b = args.find("--baud");

  if (b != args.end())
  {
	  if (b->second.isString())
	  {
	     baud   = b->second.asString();
	     result = true;
	  }
   }

  return result;
}



int main (int argc, char **argv )
{
  
    int exit_code = 0;

	static const char USAGE[] =
	R"(nanoXmodem.

	    Usage:
	      x --port=<port> --receive [--baud=<baudrate>]
	      x --port=<port> --transmit --file=<filename> [--baud=<baudrate>]
              x --enummerate
	      x (-h | --help)
	      x --version

	    Options:
	      -h --help           Show this screen.
	      --version           Show version.
	      --port=<comport>    Path to comport.
	      --baud=<baudrate>   Baudrate [default: 115200].
	      --receive           Start transfer as receiver.
	      --transmit          Start file transmission.
              --enumerate         Enumerate list of available ports.
	      --file=<filename>   File to send.
	)";

   std::map<std::string, docopt::value> args
        = docopt::docopt(USAGE,
                         { argv + 1, argv + argc },
                         true,                // show help if requested
                         "nanoXmodem 0.1");  // version string

  if(args.find("--transmit") != args.end())
  {
    std::string port = "";
    std::string baud = "";

    get_port(args, port);
    if (get_baud(args, baud))
    {
	    transmit(port, baud);
    }
    else
    {
       exit_code = 1;
    }

  }
  else if (args.find("--receive") != args.end())
  {
    std::string port = "";
    std::string baud = "";

    get_baud(args, baud);
    if (get_port(args, port))
    {
	    receive(port, baud);
    }
    else
    {
       exit_code = 1;
    }

  } 
  else if (args.find("--enumerate") != args.end())
  {
     struct sp_port **port_list;

      if (SP_OK == sp_list_ports(&port_list))
      {
//        sp_free_port_list(&port_list);
      }
      else
      {
         exit_code = 1;
      }
  }
  else
  {
    exit_code = 1;
  }

#if 0
   for(auto const& arg : args) 
   {
        std::cout << arg.first <<  arg.second << std::endl;
   }   
#endif

  return exit_code;
}


