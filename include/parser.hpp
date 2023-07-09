#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <iostream>
#include <fstream>
#include <memory>
#include "exchange.hpp"
#include "common.hpp"
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

using namespace std;

class Parser {
    public:
        Parser(MsgQueue& msg_queue, Exchange& exchange) : _msg_queue{msg_queue}, _exchange{exchange}{}
        void parse()
        {
            std::string msg;            
            while(1) {                
                while(_msg_queue.pop(msg))
                {                                        
                    char& msgType = msg[0];
                    switch(msgType)
                    {
                        case 'N':
                            addOrder(msg);
                            break;
                        case 'F':
                            _exchange.flush();
                            break;
                        case 'C':
                            cancelOrder(msg);
                            break;
                        default:
                            std::cout << "Incorrect message type!" << std::endl;
                            break;
                    }

                    msg = "";                
                }
            }

        }      

        std::optional<std::vector<std::string>> splitAndCheck(std::string& msg, int size)
        {
            //split the message up
            std::vector<std::string> info;            
            boost::split(info, msg, boost::is_any_of(", "), boost::token_compress_on);
            if(info.size() != size)
            {
                std::cout << "Message with incorrect size: " << msg << std::endl;
                return std::nullopt;
            }

            return std::optional<std::vector<std::string>>{info};
        }
        
        void addOrder(std::string& msg)
        {
            if(auto info = splitAndCheck(msg, 7); info.has_value())
            {

                //ignore the first item as that's the message type
                int32_t userId = stoi((*info)[1]);
                std::string symbol = (*info)[2];
                int32_t price = stoi((*info)[3]);
                int32_t qty = stoi((*info)[4]);
                char side = (*info)[5][0];
                int32_t userOrderId = stoi((*info)[6]);

                _exchange.newOrder(userId, userOrderId, symbol, side, qty, price);
            }
        }
        
        void cancelOrder(std::string& msg)
        {         
            if(auto info = splitAndCheck(msg, 3); info.has_value())
            {
                int32_t userId = stoi((*info)[1]);
                int32_t userOrderId = stoi((*info)[2]);

                _exchange.deleteOrder(userId, userOrderId);
            }
        }
      
    private:                
        MsgQueue& _msg_queue;
        Exchange& _exchange;
};

#endif