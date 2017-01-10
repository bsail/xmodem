#include <stdint.h>
#include "xmodem/xmodem.h"
#include "xmodem/xmodem_receiver.h"
#include "xmodem/xmodem_transmitter.h"
#include "serialport/libserialport.h"
#include "serialport/libserialport_internal.h"
#include "docopt/docopt.h"
#include <iostream>
#include <string>


void transmit(std::string port_name, std::string baud)
{
   std::cout << "transmit: " << port_name << "," << baud << std::endl;
}

void receive(std::string port_name, std::string baud)
{
   struct sp_port *port;
   port = new struct sp_port(); 
//   char port_path[100];
//   strncpy(port_path, port_name.c_str(), 100);
//   port.name = port_path;

   auto result = sp_get_port_by_name(port_name.c_str(), &port); 

   if (SP_OK == result)
   {
	   result = sp_open(port, SP_MODE_READ);
	   if (SP_OK == result)
	   {
	      std::cout << "receive: " << port_name << "," << baud << std::endl;
	   } 
	   else
	   {
	      std::cout << "receive: error opening port " << port_name << " error: " << result << std::endl;
	   }
   }
   else
   {
      std::cout << "receive: error configuring port " << port_name << " error: " << result << std::endl;

   }
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
              x --enumerate
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

  bool transmit_found  = false;
  bool receive_found   = false;
  bool enumerate_found = false;
 
  auto arg = args.find("--transmit");

  if(arg != args.end())
  {
      if (arg->second.isBool())
      {
         transmit_found = arg->second.asBool();
      }
  }
   
  arg = args.find("--receive");
 
  if(arg != args.end())
  {
     if (arg->second.isBool())
     {
        receive_found = arg->second.asBool();
     }
  }
 
  arg = args.find("--enumerate");
  
  if(arg != args.end())
  {
     if (arg->second.isBool())
     {
       enumerate_found = arg->second.asBool();
     }
  }

    if (transmit_found)
    {
    std::string port = "";
    std::string baud = "";
   
    get_baud(args, baud);

	    if (get_port(args, port))
	    {
		    transmit(port, baud);
	    }
	    else
	    {
	       std::cout << "transmit: invalid port defined" << std::endl;
	       exit_code = 1;
	    }

  }
  else if (receive_found)
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
       std::cout << "receive: invalid port defined" << std::endl;
       exit_code = 1;
    }

  } 
  else if (enumerate_found)
  {
     struct sp_port **port_list;

      if (SP_OK == sp_list_ports(&port_list))
      {
          if (NULL != port_list)
          {
             uint8_t index = 0;
             while (0 != port_list[index])
             {
                std::string str(port_list[index]->name);
                std::cout << "port: " << str << std::endl; 
                ++index;
             }

             sp_free_port_list(port_list);
          }
      }
      else
      {
         std::cout << "enumerate: no ports found" << std::endl;
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


