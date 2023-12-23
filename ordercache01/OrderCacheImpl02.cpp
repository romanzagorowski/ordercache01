#include "OrderCacheImpl02.h"

void OrderCacheImpl02::addOrder(Order order)
{
    auto p = orders.insert(order);
    //auto p = orders.insert(std::move(order));
}

void OrderCacheImpl02::cancelOrder(const std::string& orderId)
{
    throw "Not implemented yet";
}

void OrderCacheImpl02::cancelOrdersForUser(const std::string& user)
{
    throw "Not implemented yet";
}

void OrderCacheImpl02::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
    throw "Not implemented yet";
}

unsigned int OrderCacheImpl02::getMatchingSizeForSecurity(const std::string& securityId)
{
    throw "Not implemented yet";
}

std::vector<Order> OrderCacheImpl02::getAllOrders() const
{
    return std::vector<Order>{};
}
