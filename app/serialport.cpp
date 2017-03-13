#include "serialport.h"

    SerialClass::SerialClass():
    port(io){};
//            quitFlag(false){};

        SerialClass::~SerialClass()
        {
            //Stop the I/O services
            io.stop();
            //Wait for the thread to finish
//            runner.join();
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
                //Start io-service in a background thread.
                //boost::bind binds the ioservice instance
                //with the method call
/*                runner = boost::thread(
                        boost::bind(
                                &boost::asio::io_service::run,
                                &io));
*/
                startReceive();
            }

            return port.is_open();
        }

        void SerialClass::startReceive()
        {
#if 0
            using namespace asio;
            //Issue a async receive and give it a callback
            //onData that should be called when "\r\n"
            //is matched.
            async_read_until(port, buffer,
                    "\r\n",
                    boost::bind(&SerialClass::onData,
                            this, _1,_2));
#endif
        }

        void SerialClass::send(const std::string& text)
        {
            asio::write(port, asio::buffer(text));
        }
#if 0
        void onData(const system::error_code& e,
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
#endif
//        bool SerialClass::quit(){return quitFlag;}




