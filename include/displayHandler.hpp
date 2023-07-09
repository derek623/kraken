#ifndef __DISPLAYHANDLER_HPP__
#define __DISPLAYHANDLER_HPP__

#include "common.hpp"
#include "orderBook.hpp"

class DisplayHandler {
    //onQuote and onTrade have the same behaviour in this test, but in real life they should behave differently
    public:
        DisplayHandler(EventQueue& queue) : _queue{queue}{}
        void start()
        {
            while(1)
            {
                Events msg;
                while(_queue.pop(msg))
                {
                    std::visit([](auto & m)
                        {
                            m.print();
                        },
                        msg);
                }
            }
        }

    private:       
        EventQueue& _queue;
};

#endif