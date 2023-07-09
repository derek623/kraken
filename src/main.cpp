#include <iostream>
#include "receiver.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <string>
#include "parser.hpp"
#include "displayHandler.hpp"
#include "common.hpp"

int main() {    
    MsgQueue msgQueue;
    boost::asio::io_service io_service;
    Receiver server{io_service, msgQueue};

    EventQueue eventQueue;
    DisplayHandler dh(eventQueue);
    Exchange exchange(eventQueue);
    Parser p(msgQueue, exchange);

    
    

    auto receiverThread = std::thread([&]() {io_service.run();});
    auto parsingThread = std::thread([&]() {p.parse();});
    auto displayHandlerThread = std::thread([&]() {dh.start();});
    receiverThread.join();
    parsingThread.join();
    displayHandlerThread.join();


    return 1;
}