#include "OrderCacheImpl02.h"
#include "IdOnlyOrder.h"

#include <algorithm>

void OrderCacheImpl02::addOrder(Order order)
{
    if(const auto& [it, inserted] = order_uset.insert(order); inserted)
    {
        security_order_map[it->securityId()].push_back(it);
        user_order_map[it->securityId()].push_back(it);
        company_order_map[it->securityId()].push_back(it);
    }
}

void OrderCacheImpl02::cancelOrder(const std::string& orderId)
{
    // I need the iterator to the order to remove it from indexes...
    const auto it_order = order_uset.find(IdOnlyOrder{ orderId });

    // If there is such an order...
    if(it_order != order_uset.end())
    {

        {
            //--- user_order_map
            for(auto it = user_order_map.begin(); it != user_order_map.end();)
            {
                auto& [user, orders] = *it;

                if(orders.erase(std::find(orders.begin(), orders.end(), it_order)) == orders.end())
                {
                    // It was the last order for this "user" in the map. Erase it from the map.
                    it = user_order_map.erase(it);
                }
                else
                {
                    it = std::next(it);
                }
            }

            // TODO: Remove the order from the other indexes.
        }

        // Finaly remove the order from orders.
        order_uset.erase(it_order);
    }
}

void OrderCacheImpl02::cancelOrdersForUser(const std::string& user)
{
    for(const auto& it : user_order_map[user])
    {

    }

    for(const auto& it : user_order_map[user])
    {
        order_uset.erase(it);
    }

    user_order_map.erase(user);
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
    return std::vector<Order>{ std::begin(order_uset), std::end(order_uset) };
}
