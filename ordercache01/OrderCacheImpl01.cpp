#include "OrderCacheImpl01.h"

void OrderCacheImpl01::addOrder(Order order)
{
    if(const auto& [it, inserted] = order_umap.insert(std::make_pair(order.orderId(), order)); inserted)
    {
    }
}

void OrderCacheImpl01::cancelOrder(const std::string& orderId)
{
    auto it = order_umap.find(orderId);

//    this->removeFromUserOrderMap(it->user(), it->orderId());
//    this->removeFromSecurityOrderMap(it->security, it->orderId());

    //orders.erase(orderId);
}

void OrderCacheImpl01::cancelOrdersForUser(const std::string& user)
{
    std::unordered_map<std::string, std::string> user_order_map;

    if(auto it = user_order_map.find(user); it != user_order_map.end())
    {
        auto& [user, order_it] = *it;

    }
}

void OrderCacheImpl01::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
    throw "Not implemented yet";
}

unsigned int OrderCacheImpl01::getMatchingSizeForSecurity(const std::string& securityId)
{
    throw "Not implemented yet";
}

std::vector<Order> OrderCacheImpl01::getAllOrders() const
{
    return std::vector<Order>{};
}
