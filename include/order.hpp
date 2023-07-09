#ifndef __Order_HPP__
#define __Order_HPP__

constexpr static uint16_t DEFAULT_BOOKID = std::numeric_limits<uint16_t>::max();        

enum class Side {
    Buy = 0,
    Sell,
    Unknown
};

struct Order {
    uint32_t userId = 0;
    uint32_t orderId = 0;
    int32_t price = 0;
    uint64_t qty = 0;
    uint64_t remainingQty = 0;
    Side side = Side::Unknown;
    uint16_t bookid = DEFAULT_BOOKID;
    bool isMarket = false;

    Order(uint32_t userId, uint32_t orderId, int32_t price, uint64_t qty, Side side, uint16_t bookid, bool isMarket)
        : userId{userId}, orderId(orderId), price(price), qty(qty), remainingQty{qty}, side(side), bookid(bookid), isMarket{isMarket} {}
    Order() {}

    friend std::ostream& operator<<(std::ostream& out, const Order& o)
    {
        char bs = Side::Buy == o.side ? 'B' : 'S';
        out << " Order: " << o.userId << ", " << o.orderId << ", " << o.price << ", " << o.qty << ", " << bs << ", " << o.bookid << ", " << o.isMarket;
        return out;
    }
};

#endif