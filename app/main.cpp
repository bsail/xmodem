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
#define ASIO_STANDALONE 1
#endif
#include <asio.hpp>
#include "serialport.h"

#include <string>
#include <fstream>
#include <streambuf>


static SerialClass serial;


bool read_file_to_buffer(std::string file_path, std::vector<char>& buffer) {

    std::ifstream file(file_path); //, std::ios::binary);

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
   *returned_size = serial.read(requested_size, buffer);
   return result;

}

static bool transmitter_write_data(const uint32_t requested_size, uint8_t *buffer, bool *write_success)
{
   bool result = false;
   std::string str((char*)buffer);
   serial.write(requested_size, buffer);
   
   if (0 != write_success)
   {
      *write_success = true;
   } 

   return result;
}


static uint8_t  receiver_buffer[BUFFER_SIZE];

static bool receiver_is_inbound_empty()
{
  return false;
  
}

static bool receiver_is_outbound_full()
{
  return false;
}

static bool receiver_read_data(const uint32_t requested_size, uint8_t *buffer, uint32_t *returned_size)
{
   bool result = false;
   *returned_size = serial.read(requested_size, buffer);
   return result;

}

static bool receiver_write_data(const uint32_t requested_size, uint8_t *buffer, bool *write_success)
{
   bool result = false;
   std::string str((char*)buffer);
   serial.write(requested_size, buffer);
   
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

void transmit(std::string port_name, std::string baud, std::string file)
{

   std::vector<char> buffer_transmit;
   read_file_to_buffer(file, buffer_transmit);

   xmodem_transmitter_set_callback_write(&transmitter_write_data);
   xmodem_transmitter_set_callback_read(&transmitter_read_data);
   xmodem_transmitter_set_callback_is_outbound_full(&transmitter_is_outbound_full);
   xmodem_transmitter_set_callback_is_inbound_empty(&transmitter_is_inbound_empty);

   padbuffer(buffer_transmit);

   // this works as std::vector guarantees contiguous memory allocation
   xmodem_transmit_init(reinterpret_cast<uint8_t*>(buffer_transmit.data()), buffer_transmit.size());

 
  if (!serial.connect(port_name, 115200))
  {
      assert(0);
  }


   while (XMODEM_TRANSMIT_COMPLETE != xmodem_transmit_state() &&
	  XMODEM_TRANSMIT_ABORT_TRANSFER  != xmodem_transmit_state())
   {
      uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      xmodem_transmit_process(now); 
      usleep(1000);
      if (1 != xmodem_transmit_state())
      {
        std::cout << "state: " << xmodem_transmit_state() << std::endl;
      }     
   }

}



void receive(std::string port_name, std::string baud, std::string file)
{


   std::vector<char> buffer_receive;
   xmodem_receive_set_callback_write(&receiver_write_data);
   xmodem_receive_set_callback_read(&receiver_read_data);
   xmodem_receive_set_callback_is_outbound_full(&receiver_is_outbound_full);
   xmodem_receive_set_callback_is_inbound_empty(&receiver_is_inbound_empty);
   padbuffer(buffer_receive);

   // this works as std::vector guarantees contiguous memory allocation
   xmodem_receive_init();

 
  if (!serial.connect(port_name, 115200))
  {
      assert(0);
  }


   while (XMODEM_RECEIVE_TRANSFER_COMPLETE != xmodem_receive_state() &&
	  XMODEM_RECEIVE_ABORT_TRANSFER  != xmodem_receive_state())
   {
      uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      xmodem_receive_process(now); 
      usleep(1000);

      std::cout << "state: " << xmodem_receive_state() << std::endl;

#if 0
      if (1 != xmodem_receive_state())
      {
      }     
#endif
   }


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
	      x --port=<port> --receive  --file=<filename> [--baud=<baudrate>]
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
      std::string file = "";
   
      get_baud(args, baud);
      get_file(args, file);

      if (get_port(args, port))
      {
         transmit(port, baud, file);
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
     std::string file = "";

     get_baud(args, baud);

     if (get_port(args, port) && get_file(args,file))
     {
       receive(port, baud, file);
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

  return exit_code;
}

