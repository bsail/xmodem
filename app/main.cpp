#include <stdint.h>
#include "xmodem/xmodem.h"
#include "xmodem/xmodem_receiver.h"
#include "xmodem/xmodem_transmitter.h"
#include "serialport/libserialport.h"
#include "serialport/libserialport_internal.h"
#include "docopt/docopt.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <iterator>
#include <fstream>

bool read_file_to_buffer(std::string file_path, std::vector<char>& buffer) {

    std::ifstream file(file_path, std::ios::binary);

    if (file.fail()) {
        perror(file_path.c_str());
        return false;
    }

    // copies all data into buffer
    std::vector<char> prov(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>()));

    buffer = prov;

    file.close();

    return true;
}


bool write_buffer_to_file(std::string file_path, std::vector<char>& buffer)
{
  std::ofstream file (file_path, std::ofstream::out);

  for (std::vector<char>::const_iterator ii = buffer.begin(); ii != buffer.end(); ++ii)
  {
     file << *ii;
  }

  file.close();

  return true;
}

#if 0
template < class T >
std::ostream& operator << (std::ostream& os, const std::vector<T>& v) 
{
    os << "[";
    for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii)
    {
        os << " " << *ii;
    }
    os << "]";
    return os;
}
#endif

void transmit(std::string port_name, std::string baud, bool socat)
{
   struct sp_port *port;
   port = new struct sp_port(); 
   char portnamearray[200];
   memset(portnamearray, 0, 200);
   strncpy(portnamearray, port_name.c_str(), 200);
   char portdescarray[200];
   memset(portdescarray, 0, 200);
   strncpy(portdescarray, "socat port", 200);

   sp_return result = SP_OK;

   if (!socat)
   {
     auto result = sp_get_port_by_name(port_name.c_str(), &port); 
   }
   else
   {
      port->transport     = SP_TRANSPORT_PTY;
      port->name          = portnamearray;
      port->description   = portdescarray;
     
   }

   if (socat || SP_OK == result)
   {
	   result = sp_open(port, SP_MODE_WRITE);
	   if (SP_OK == result)
	   {
	      std::cout << "trasmit: " << port_name << "," << baud << std::endl;
	   } 
	   else
	   {
	      std::cout << "transmit: error opening port " << port_name << " error: " << result << std::endl;
	   }
   }
   else
   {
      std::cout << "transmit: error configuring port " << port_name << " error: " << result << std::endl;

   }
   
   if (0 == result)
   {
	   char buffer[2048];
	   strcpy(buffer, "*");
	   uint16_t bytes_written = 0;
           uint32_t cummulative_written = 0;

	   while(1)
	   {
	      bytes_written = sp_blocking_write(port, buffer, 1, 2000);
              cummulative_written = cummulative_written + bytes_written;

	      if (bytes_written > 0)
	      {
		 std::cout << "block write: ";
		 std::cout << std::endl;
	      }

              if (!(cummulative_written % 50))
              {             
  	        bytes_written = sp_blocking_write(port, "\n", 1, 2000);
//                cummulative_written = cummuulative_written + bytes_written;
              }


	      usleep(100);



	   }
	   std::cout << "transmit: " << port_name << "," << baud << std::endl;
   }
   delete port;
}

void receive(std::string port_name, std::string baud, bool socat)
{
   struct sp_port *port;
   port = new struct sp_port(); 
   char portnamearray[200];
   memset(portnamearray, 0, 200);
   strncpy(portnamearray, port_name.c_str(), 200);
   char portdescarray[200];
   memset(portdescarray, 0, 200);
   strncpy(portdescarray, "socat port", 200);

   sp_return result = SP_OK;

   if (!socat)
   {
     auto result = sp_get_port_by_name(port_name.c_str(), &port); 
   }
   else
   {
      port->transport     = SP_TRANSPORT_PTY;
      port->name          = portnamearray;
      port->description   = portdescarray;
     
   }

   if (socat || SP_OK == result)
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

   if (0 == result)
   {
	   char buffer[2048];
	   uint16_t bytes_read = 0;

	   while(1)
	   {
	      bytes_read = sp_blocking_read(port, buffer, 2048, 1000);

	      if (bytes_read > 0)
	      {
		 std::cout << "block read: ";
		 uint8_t i = 0;
		 for (int i = 0; i < bytes_read; ++i)
		 {
		    std::cout << buffer[i];
		 } 
		 std::cout << std::endl;
	      }
	      sleep(2);
	   }
  }
  delete port;  // need to free structure's allocated memory, name, desc, etc, currently leaking
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


void debug_print(const char*format, ...)
{
    va_list args;
    va_start (args, format);
    vprintf(format, args);
    va_end (args); 
}

int main (int argc, char **argv )
{

    sp_debug_handler = debug_print;
 
    int exit_code = 0;

	static const char USAGE[] =
	R"(nanoXmodem.

	    Usage:
	      x --port=<port> --receive [--baud=<baudrate>] [--socat]
	      x --port=<port> --transmit --file=<filename> [--baud=<baudrate>] [--socat]
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
              --socat             Using a socat virtual comport
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
  bool socat_found     = false;
 
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

  arg = args.find("--socat");
  if(arg != args.end())
  { 
     if (arg->second.isBool())
     {
        socat_found = arg->second.asBool();
     }
  }

    if (transmit_found)
    {
    std::string port = "";
    std::string baud = "";
   
    get_baud(args, baud);

	    if (get_port(args, port))
	    {
		    transmit(port, baud, socat_found);
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
	    receive(port, baud, socat_found);
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


