#include <stdint.h>
extern "C" {
#include "xmodem/xmodem.h"
#include "xmodem/xmodem_transmitter.h"
#include "xmodem/xmodem_receiver.h"
}
#include "docopt/docopt.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <iterator>
#include <fstream>
#include <assert.h>
#include <chrono>
#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE  1
#endif
#include <asio.hpp>
#include "serialport.h"

static SerialClass serial;


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

#define BUFFER_SIZE 1024
static uint8_t  transmitter_buffer[BUFFER_SIZE];

static bool transmitter_is_inbound_empty()
{
  return false;
  
}

static bool transmitter_is_outbound_full()
{
  return false;
}

static bool transmitter_read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   bool result = false;
#if 0
   if (0 != port)
   {
	*returned_size = sp_blocking_read(port, buffer, requested_size, 1000);
        if (*returned_size > 0 && *returned_size < requested_size)
        {
           std::cout << "read:" << *returned_size << std::endl;
           result = true;
           assert(0);
        } 
 
   }
#endif
   return result;

}

static bool transmitter_write_data(const uint32_t requested_size, uint8_t *buffer, bool *write_success)
{
   bool result = false;
#if 0
   if (0 != port)
   {
	auto returned_size = sp_blocking_write(port, buffer, requested_size, 1000);

        if (returned_size == requested_size)
        {
           result = true;
        } 
   }
#endif
   return result;
}

void padbuffer(std::vector<char> &buffer_transmit)
{
   auto pad = XMODEM_BLOCK_SIZE - (buffer_transmit.size() % XMODEM_BLOCK_SIZE);

   for (uint8_t i = 0; i < pad; ++i)
   {
      buffer_transmit.push_back(0xFF);
   }

   assert(0 == buffer_transmit.size() % XMODEM_BLOCK_SIZE);
}

void transmit(std::string port_name, std::string baud, bool socat, std::string file)
{
#if 0
   port = new struct sp_port(); 
   char portnamearray[200];
   memset(portnamearray, 0, 200);
   strncpy(portnamearray, port_name.c_str(), 200);
   char portdescarray[200];
   memset(portdescarray, 0, 200);
   strncpy(portdescarray, "socat port", 200);

   sp_return result = SP_OK;
#endif
   std::vector<char> buffer_transmit;
   read_file_to_buffer(file, buffer_transmit);

   xmodem_transmitter_set_callback_write(&transmitter_write_data);
   xmodem_transmitter_set_callback_read(&transmitter_read_data);
   xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
   xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

   padbuffer(buffer_transmit);

   // this works as std::vector guarantees contiguous memory allocation
   xmodem_transmit_init(reinterpret_cast<uint8_t*>(buffer_transmit.data()), buffer_transmit.size());
#if 0
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
	      std::cout << "transmit: " << port_name << "," << baud << std::endl;
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

           while (XMODEM_TRANSMIT_COMPLETE != xmodem_transmit_state() &&
                  XMODEM_TRANSMIT_ABORT_TRANSFER  != xmodem_transmit_state())
           {
              uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
              xmodem_transmit_process(now); 
              usleep(1000);
              std::cout << "state: " << xmodem_transmit_state() << std::endl;
           }

	   std::cout << "transmit: " << port_name << "," << baud << std::endl;
   }

   delete port;

  #endif 
}

void receive(std::string port_name, std::string baud, bool socat)
{
#if 0
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
#endif
}

bool get_file(std::map<std::string, docopt::value> args, std::string &file)
{
  bool result = false; 
  auto p = args.find("--file")->second;

  if (p.isString())
  {
     file = p.asString();
     result = true;
  }
  
  return result;
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
    std::string file = "";
   
    get_baud(args, baud);
    get_file(args, file);

	    if (get_port(args, port))
	    {
		    transmit(port, baud, socat_found, file);
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
     std::cout << "enumerate: no ports found" << std::endl;
     exit_code = 1;
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

#if 0
    int main(int argc, char* argv[])
    {
        if (serial.connect("/home/dnad/pts1", 115200))
        {
            std::cout<<"Port is open."<<std::endl;
        }
        else
        {
            std::cout<<"Port open failed."<<std::endl;
        }

        while (!serial.quit())
        {
            //Do something...
            serial.send("Text\r\n");
            usleep(1000*500);
        }

        return 0;
    }
#endif
