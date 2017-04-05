#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE 1
#endif
#include <cstdio>
#include <string>
#include <asio.hpp>
#include <cstddef>
#include <iostream>
#include <thread>
#include <functional>
#include "serialport.h"

    SerialClass::SerialClass():port(io)
    {
    }

    SerialClass::~SerialClass()
    {
       io.stop();
    }

    bool SerialClass::connect(const std::string& port_name, int baud)
    {
        using namespace asio;
        asio::error_code error;
        port.open(port_name, error);

        std::cout << "error: " << error << std::endl;
        std::cout << "port: " << port_name << std::endl;

        //Setup port
        port.set_option(serial_port::baud_rate(baud));
        port.set_option(serial_port::flow_control(
                serial_port::flow_control::none));


        return port.is_open();
     }

        uint32_t SerialClass::read(const uint32_t requested_size, uint8_t *buffer)
        {
           uint32_t read_bytes = 0;
           bool failed = false;
           try
           {
              read_bytes = asio::read(port, asio::buffer(buffer, requested_size));
           }
           catch(std::system_error)
           {
             failed = true;
           }

           return read_bytes;
        }

        void SerialClass::write(const uint32_t requested_size, uint8_t *buffer)
        {
            try
            {
               asio::write(port, asio::buffer(buffer, requested_size));
            }
            catch(std::system_error)
            {
            }
        }


