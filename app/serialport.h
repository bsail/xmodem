#include <string>
#include <asio.hpp>

class SerialClass
{
  public:
      //Class constructor
      SerialClass();
      //Class destructor
      ~SerialClass();
      //Connection method that will setup the serial port
      bool connect(const std::string& port_name, int baud = 9600);
      //The method that will be called to issue a new 
      //asynchronous read
      void startReceive();
      //Function for sending a data string
      void send(const std::string& text);
  private:
      //The callback function that will be executed when data 
      //arrives.
//      void onData(const asio::system::error_code& e, 
 //                                            std::size_t size)

      //Boost.Asio I/O service required for asynchronous 
      //operation
      asio::io_service io;
      //Serial port accessor class
      asio::serial_port port;
      //Background thread
//      thread runner;
      //Buffer in which to read the serial data
      asio::streambuf buffer; 
};

