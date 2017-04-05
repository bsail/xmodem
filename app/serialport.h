class SerialClass
{
  public:
      SerialClass();
      ~SerialClass();
      bool connect(const std::string& port_name, int baud = 115200);
      void write(const uint32_t requested_size, uint8_t *buffer);
      uint32_t read(const uint32_t requested_size, uint8_t *buffer);

  private:
      //Boost.Asio I/O service required for asynchronous 
      asio::io_service io;
      //Serial port accessor class
      asio::serial_port port;
};

