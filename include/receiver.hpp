#ifndef __RECEIVER_HPP__
#define __RECEIVER_HPP__

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "common.hpp"

using boost::asio::ip::udp;

class Receiver {
    public:
        Receiver(boost::asio::io_service& io_service, MsgQueue& msg_queue)
            : _socket{io_service, udp::endpoint(udp::v4(), 1111)}, _msg_queue{msg_queue}
        {
            startReceive();
        }
    private:
        void startReceive() {
            _socket.async_receive_from(
                boost::asio::buffer(_recvBuffer), _remoteEndpoint,
                boost::bind(&Receiver::handleReceive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        void handleReceive(const boost::system::error_code& error,
                        std::size_t bytes_transferred) {
            if (!error || error == boost::asio::error::message_size) {

                if(!_recvBuffer.empty())
                {                    
                    if(!_msg_queue.push(_recvBuffer.data()))
                        std::cout << "Message queue is full, data is dropped!" << std::endl;                                        
                }
                                
                _recvBuffer.fill(0);
                startReceive();
            }
        }

        udp::socket _socket;
        udp::endpoint _remoteEndpoint;
        std::array<char, 50> _recvBuffer;
        MsgQueue& _msg_queue;
};

#endif