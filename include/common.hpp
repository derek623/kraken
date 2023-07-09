#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <boost/lockfree/spsc_queue.hpp>
#include <queue>
#include <string>
#include <variant>

typedef boost::lockfree::spsc_queue<std::string, boost::lockfree::capacity<1024> > MsgQueue;

#define MAX_MSG_SIZE 100

template<typename T>
struct Message {    
    void print(){
        T &type = static_cast<T&>(*this);
        type.print();
    }
};

struct Trade : Message<Trade>{
    uint32_t buyUserId;
    uint32_t buyUserOrderId;
    uint32_t sellUserId;
    uint32_t sellUserOrderId;
    int32_t price;
    int32_t qty;    
    void print()
    {
        std::cout <<"T, "<< buyUserId << ", " << buyUserOrderId << ", " << sellUserId << ", " << sellUserOrderId << ", " << price << ", " << qty << std::endl;
    }
};

struct Ack : Message<Ack>{
    uint32_t userId;
    uint32_t userOrderId;
    void print()
    {
        std::cout << "A, "<< userId << ", " << userOrderId << std::endl;
    }
};

struct TopOfBook : Message<TopOfBook>{
    char side;
    int32_t price;
    uint32_t qty;
    void print()
    {
        if (price == 0 || qty == 0)
            std::cout <<"B, "<< side << ", -, - "<< std::endl;
        else
            std::cout <<"B, "<< side << ", " << price << ", " << qty << std::endl;
    }
};

template <typename ... Ts>
using poly_T = std::variant<Ts...>;
using Events = poly_T<Trade, Ack, TopOfBook>;

typedef boost::lockfree::spsc_queue<Events, boost::lockfree::capacity<1024> > EventQueue;

#endif