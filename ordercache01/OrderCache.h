#pragma once

#include "OrderCacheInterface.h"

#include <unordered_set>
#include <map>

// Compare orders by comparing their ids.
struct OrderCompare
{
    bool operator () (const Order& o1, const Order& o2) const noexcept
    {
        return o1.orderId() == o2.orderId();
    }
};

// Hash an order by hashing its id.
struct OrderHash
{
    std::size_t operator () (const Order& o) const noexcept
    {
        return hash(o.orderId());
    }

private:
    std::hash<std::string> hash;
};

class OrderCache : public OrderCacheInterface
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
    using OrdersTableType = std::unordered_set<Order, OrderHash, OrderCompare>;
    using OrderIterator = OrdersTableType::const_iterator;
    using IndexType = std::multimap<std::string, OrderIterator>;

private:
    OrdersTableType orders_table;

    IndexType security_index;
    IndexType     user_index;
    IndexType  company_index;

private:
    static void addOrderToIndex(
        IndexType& index,
        const std::string& key,
        const OrderIterator& it_order
    );

    static void removeOrderFromIndex(
        IndexType& index,
        const OrderIterator& it_order
    );
};
