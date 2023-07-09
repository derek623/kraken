#ifndef __ORDERBOOK_HPP__
#define __ORDERBOOK_HPP__

#include <iostream>
#include <boost/container/flat_map.hpp>
#include "order.hpp"
#include "common.hpp"
#include <deque>
#include <memory>


class OrderBook 
{
    public:        
        struct Level {
            uint64_t aggregatedQty = 0;
            int32_t price = 0;
            Side buy_or_sell;
            std::deque<std::shared_ptr<Order>> orderQueue;

            Level(Side bs) : buy_or_sell{bs} {}
            Level(const uint64_t q, const int32_t p) : aggregatedQty{q}, price{p}{}

            friend std::ostream &operator<<(std::ostream &out, const Level &level) {
                int32_t newPrice = Side::Buy == level.buy_or_sell ? level.price : -level.price;
                out << "(" << newPrice << ", " << level.aggregatedQty << ")";
                return out;
            }
        };
        OrderBook() : _bestBid{0}, _bestAsk{0}, _bestBidQty{0}, _bestAskQty{0} {}
        constexpr static uint64_t INVALID_LEVEL = std::numeric_limits<uint64_t>::max();

        std::vector<Events> match(std::shared_ptr<Order> order)
        {
            std::vector<poly_T<Trade, Ack, TopOfBook>> result;
             //get the opposite book
            auto& book = Side::Buy == order->side ? _sell : _buy;
            //If the book is empty, nothing to match
            if(!book.empty())
            {
                uint32_t unmatchedQty = order->remainingQty;
                
                for(auto itr = book.begin(); itr != book.end();)
                {
                    Level& l = itr->second;
                    //if it's a buy order, the level is a sell level, so need to flip the price
                    const int32_t levelPrice = Side::Buy == order->side ? -l.price : l.price;                    

                    //only match with the correct price level for limit order
                    if(!order->isMarket && (Side::Buy == order->side && order->price < levelPrice || Side::Sell == order->side && order->price > levelPrice))                                            
                        break;                    

                    while(unmatchedQty > 0)
                    {
                        auto o = l.orderQueue.front();
                        //Generate the trade
                        uint32_t matchedQty = unmatchedQty >= o->remainingQty ? o->remainingQty : unmatchedQty;
                        unmatchedQty -= matchedQty;
                        Trade t;
                        if(Side::Buy == order->side)
                        {
                            t.buyUserId = order->userId;
                            t.buyUserOrderId = order->orderId;
                            t.sellUserId = o->userId;
                            t.sellUserOrderId = o->orderId;
                        } else {
                            t.sellUserId = order->userId;
                            t.sellUserOrderId = order->orderId;
                            t.buyUserId = o->userId;
                            t.buyUserOrderId = o->orderId;
                        }
                        t.price = o->price;
                        t.qty = matchedQty;                        
                        result.push_back(t);
                        //Reduce the aggregated qty
                        l.aggregatedQty -= matchedQty;                            
                        //Reduce the order qty
                        o->remainingQty -= matchedQty;
                        order->remainingQty -= matchedQty;
                        //Remove the order from the deque
                        if(0 == o->qty)
                            l.orderQueue.erase(std::remove(l.orderQueue.begin(), l.orderQueue.end(), o), l.orderQueue.end());
                        //Remove the level if the aggregated qty is 0
                        if(l.aggregatedQty <= 0)
                        {                            
                            itr = book.erase(itr);
                            break;
                        }                           
                    }

                    if(0 == unmatchedQty)
                        break;
                    
                }
            }
            return result;
        }

        std::vector<Events> newOrder (std::shared_ptr<Order> order)
        {   
            //insert the new order ack
            std::vector<Events> result;         
            Ack a;            
            a.userId = order->userId;
            a.userOrderId = order->orderId;
            result.push_back(a);        

            //If any trades generated after matching, also add to the result vector
            auto matchedResult = match(order);
            if(!matchedResult.empty())
                result.insert(result.end(), matchedResult.begin(), matchedResult.end());
            
            //If the order is not fully filled, then add the order to the book
            if(!order->isMarket && order->remainingQty > 0)
                add(order);

            //Need to check both buy and sell side and see if best prices have changed
            auto event = setBestPrice(Side::Buy);
            if(event.has_value())
                result.push_back(*event);

            event = setBestPrice(Side::Sell);
            if(event.has_value())
                result.push_back(*event);
            return result;            
        }

        std::optional<Events> setBestPrice(Side side)
        {            
            uint32_t price = 0;
            uint32_t qty = 0;
            
            if(Side::Buy == side)
            {                
                
                const int32_t currBestBid = _buy.empty() ? 0 : _buy.begin()->second.price;
                const int32_t currBestBidQty = _buy.empty() ? 0 : _buy.begin()->second.aggregatedQty;
                if(currBestBid == _bestBid && currBestBidQty == _bestBidQty)
                    return std::nullopt;
                
                _bestBid = currBestBid;
                _bestBidQty = currBestBidQty;
                price = _bestBid;
                qty = _bestBidQty;
                
            } else {                
                
                const int32_t currBestAsk = _sell.empty() ? 0 : -_sell.begin()->second.price;
                const int32_t currBestAskQty = _sell.empty() ? 0 : _sell.begin()->second.aggregatedQty;
                if(currBestAsk == _bestAsk && currBestAskQty == _bestAskQty)
                    return std::nullopt;
                
                _bestAsk = currBestAsk;
                _bestAskQty = currBestAskQty;
                price = _bestAsk;
                qty = _bestAskQty;                
                
            }

            TopOfBook tob;
            tob.side = side == Side::Buy ? 'B' : 'S';
            tob.price = price;
            tob.qty = qty;
            return std::optional<Events>(tob);
        }

        void add(std::shared_ptr<Order> order)
        {            
            auto& book = Side::Buy == order->side ? _buy : _sell;
            int32_t newPrice = Side::Buy == order->side ? order->price : -order->price; //flip the price for sell order

            auto itr = book.emplace(newPrice, Level{order->side}).first;
            
            Level& l = itr->second;
            l.price = newPrice;
            l.aggregatedQty += order->remainingQty;
            l.orderQueue.push_back(order);                     
        }
        
        std::vector<Events> deleteOrder(std::shared_ptr<Order> order)
        {
            std::vector<Events> result;
            auto& book = Side::Buy == order->side ? _buy : _sell;
            int32_t newPrice = Side::Buy == order->side ? order->price : -order->price; //flip the price for sell order
            //Find the book
            auto itr = book.find(newPrice);
            if(book.end() == itr)
                return result;
            //Remove the order from the queue in the Level
            Level& l = itr->second;
            l.orderQueue.erase(std::remove(l.orderQueue.begin(), l.orderQueue.end(), order), l.orderQueue.end());            
            reduce(order->side, order->qty, order->price);

            //Generate the Cancel Ack
            Ack a;            
            a.userId = order->userId;
            a.userOrderId = order->orderId;
            result.push_back(a);

            //Check and set best price
            auto event = setBestPrice(order->side);
            if(event.has_value())
                result.push_back(*event);

            return result;
        }       

        void reduce(Side side, const uint64_t qty, const int32_t price)
        {
            auto& book = Side::Buy == side ? _buy : _sell;
            int32_t newPrice = Side::Buy == side ? price : -price; //flip the price for sell order

            auto itr = book.find(newPrice);
            if(book.end() == itr)
                return; // flat_map max entry is smaller than max of uint64_t

            Level& l = itr->second;
            if(qty <= l.aggregatedQty)
                l.aggregatedQty -= qty;
            else
                l.aggregatedQty = 0;
            
            if(l.aggregatedQty <= 0)
                book.erase(itr);
            
        }

        void flush()
        {
            _buy.clear();
            _sell.clear();
            _bestBid = 0;
            _bestAsk = 0;
            _bestBidQty = 0;
            _bestAskQty = 0;
        }

        friend std::ostream &operator<<(std::ostream &out, const OrderBook &book)
        {
            std::cout << " Buy:" << std::endl;
            for(auto b : book._buy)
                std::cout << "\t" << b.second << std::endl;
            std::cout << " Sell:" << std::endl;
            for(auto s : book._sell)
                std::cout << "\t" << s.second << std::endl;
            return out;
        }

        void print(const uint64_t level) const
        {
            int count = 1;
            std::cout << "[";
            auto bitr = _buy.begin();
            if(_buy.end() != bitr)
            {
                std::cout << bitr->second;

                for(++bitr; bitr != _buy.end() && count < level; ++bitr,++count )
                    std::cout << ", " << bitr->second;
                std::cout << "], [";
            }
            count = 1;
            auto sitr = _sell.begin();
            if(_sell.end() != sitr)
            {
                std::cout << sitr->second;
                for(++sitr; sitr != _sell.end() && count < level; ++sitr,++count )
                    std::cout << ", " << sitr->second;
            }
            std::cout << "]" << std::endl;
        }

#ifdef DEBUG_BUILD
        //For testing
        boost::container::flat_map<int64_t, Level, std::greater<int64_t>>& getBuy()
        {
            return _buy;
        }

        boost::container::flat_map<int64_t, Level, std::greater<int64_t>>& getSell()
        {
            return _sell;
        }
#endif
    private:
        boost::container::flat_map<int64_t, Level, std::greater<int64_t>> _buy, _sell;
        uint32_t _bestBid, _bestAsk, _bestBidQty, _bestAskQty;
};

#endif