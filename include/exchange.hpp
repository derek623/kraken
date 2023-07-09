#ifndef __EXCHANGE_HPP__
#define __EXCHANGE_HPP__


#include <unordered_map>
#include <vector>
#include "orderBook.hpp"
#include "displayHandler.hpp"
#include <optional>
#include <memory>

class Exchange {

    public:           
        Exchange(EventQueue& queue) : _queue(queue) {}
        
        void newOrder(const uint32_t userId, const uint32_t orderId, const std::string_view symbol, const char side, const uint64_t qty, const int32_t price)
        {
            size_t hashCode = std::hash<std::string_view>{}(symbol);
            //Check if symbol(the hashed code) has an orderbook. If not, create one
            auto itr = _hashcodeToOrderbookIdMap.find(hashCode);
            if(itr == _hashcodeToOrderbookIdMap.end())
            {
                _orderBooks.emplace_back(OrderBook());
                itr = _hashcodeToOrderbookIdMap.emplace(hashCode, _orderBooks.size() - 1).first;
            }

            //insert the order to the map if it's a limit order
            Side bs = 'B' == side ? Side::Buy : Side::Sell;
            auto order_itr = _orderMap[userId].emplace(orderId, std::make_shared<Order>(Order{userId, orderId, price, qty, bs ,itr->second, price == 0})).first;
            

            //Update the orderBook
            OrderBook& ob = _orderBooks[itr->second];            
            auto results = ob.newOrder(order_itr->second);
            for (auto& msg : results)            
                _queue.push(msg);
        }
       
        void deleteOrder(const uint32_t userId, const uint64_t orderId)
        {
           //find the order
            auto itr = _orderMap.find(userId);
            if(_orderMap.end() == itr)
                return;
            
            auto& internal_map = itr->second;
            auto itr2 = internal_map.find(orderId);
            if(internal_map.end() == itr2)
                return;

            auto o = itr2->second;
            //Somehow the order is not map to a book id
            if(DEFAULT_BOOKID == o->bookid)
                return;
            
            OrderBook& ob = _orderBooks[o->bookid];                        
            auto results = ob.deleteOrder(o);
            for (auto& msg : results)
                _queue.push(msg);

            //remove the order from the map
            internal_map.erase(itr2);            
        }

        void flush()
        {
            _hashcodeToOrderbookIdMap.clear();
            for(auto& ob : _orderBooks)
            {
                ob.flush();
            }
            for(auto& map : _orderMap)
            {
                map.second.clear();
            }
            _orderMap.clear();
        }

        void print() 
        {
            for(auto x : _orderMap)
            {
                std::cout << x.first << ":" << std::endl;
                for(auto y : x.second)
                {
                    std::cout << " " << y.first<< ", " << y.second << std::endl;
                }
            }
        }        

    private:        
        boost::container::flat_map<std::size_t, uint16_t> _hashcodeToOrderbookIdMap;
        std::vector<OrderBook> _orderBooks;
        boost::container::flat_map<uint32_t, boost::container::flat_map<uint32_t, std::shared_ptr<Order>>> _orderMap;
        EventQueue& _queue;
};

#endif