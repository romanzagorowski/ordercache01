#pragma once

#include "OrderCacheInterface.h"

#include <unordered_set>

struct OrderCompare
{
    bool operator () (const Order& o1, const Order& o2) const
    {
        return o1.orderId() == o2.orderId();
    }
};

struct OrderHash
{
    std::hash<std::string> hash;

    std::size_t operator () (const Order& o) const
    {
        return hash(o.orderId());
    }
};

class OrderCacheImpl02 : public OrderCacheInterface
{
public:
    // add order to the cache
    void addOrder(Order order) override;

    // remove order with this unique order id from the cache
    void cancelOrder(const std::string& orderId) override;

    // remove all orders in the cache for this user
    void cancelOrdersForUser(const std::string& user) override;

    // remove all orders in the cache for this security with qty >= minQty
    void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override;

    // return the total qty that can match for the security id
    unsigned int getMatchingSizeForSecurity(const std::string& securityId) override;

    // return all orders in cache in a vector
    std::vector<Order> getAllOrders() const override;

private:
    std::unordered_set<Order, OrderHash, OrderCompare> orders;
};
