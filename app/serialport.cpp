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

    SerialClass::SerialClass():
    port(io),quitFlag(false){};
            

        SerialClass::~SerialClass()
        {
            //Stop the I/O services
            io.stop();
            //Wait for the thread to finish
           // runner.join();
        }

        bool SerialClass::connect(const std::string& port_name, int baud)
        {
            using namespace asio;
            port.open(port_name);
            //Setup port
            port.set_option(serial_port::baud_rate(baud));
            port.set_option(serial_port::flow_control(
                    serial_port::flow_control::none));

            if (port.is_open())
            {
                //runner = 
                std::bind(static_cast<std::size_t (asio::io_service::*)()>(&asio::io_service::run), &io);
                startReceive();
            }

            return port.is_open();
        }

        void SerialClass::startReceive()
        {
            using namespace asio;
            //Issue a async receive and give it a callback
            //onData that should be called when "\r\n"
            //is matched.
            async_read_until(port, buffer,
                    "\r\n",
                    std::bind(&SerialClass::onData,
                            this, std::placeholders::_1,std::placeholders::_2));
        }

        void SerialClass::send(const std::string& text)
        {
            asio::write(port, asio::buffer(text));
        }
        void SerialClass::onData(const asio::error_code& e,
                std::size_t size)
        {
            if (!e)
            {
                std::istream is(&buffer);
                std::string data(size,'\0');
                is.read(&data[0],size);

                std::cout<<"Received data:"<<data;

                //If we receive quit()\r\n indicate
                //end of operations
                quitFlag = (data.compare("quit()\r\n") == 0);
            };

            startReceive();
        };

        bool SerialClass::quit(){return quitFlag;}




