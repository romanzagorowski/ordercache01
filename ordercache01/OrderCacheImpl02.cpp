#include "OrderCacheImpl02.h"
#include "IdOnlyOrder.h"

#include <algorithm>
#include <set>

void OrderCacheImpl02::addOrder(Order order)
{
    if(const auto& [it, inserted] = order_uset.insert(order); inserted)
    {
        security_order_map[it->securityId()].push_back(it);
        user_order_map[it->user()].push_back(it);
        company_order_map[it->company()].push_back(it);
    }
}

static
void removeOrderFromIndex(
    std::map<std::string, std::vector<std::unordered_set<Order>::const_iterator>>& index,
    std::unordered_set<Order>::const_iterator it_order
)
{
    auto it_key_orders = index.begin();

    while(it_key_orders != index.end())
    {
        auto& [user, orders] = *it_key_orders;

        orders.erase(std::find(orders.begin(), orders.end(), it_order));

        if(orders.empty())
        {
            it_key_orders = index.erase(it_key_orders);
        }
        else
        {
            it_key_orders = std::next(it_key_orders);
        }
    }
}

void OrderCacheImpl02::cancelOrder(const std::string& orderId)
{
    // I need the iterator to the order to remove it from indexes...
    const auto it_order = order_uset.find(IdOnlyOrder{ orderId });

    if(it_order != order_uset.end())
    {
        removeOrderFromIndex(user_order_map, it_order);
        removeOrderFromIndex(security_order_map, it_order);
        removeOrderFromIndex(company_order_map, it_order);
    }

    order_uset.erase(it_order);
}

void OrderCacheImpl02::cancelOrdersForUser(const std::string& user)
{
    // Remove user's orders from...
    for(const auto& it_order : user_order_map[user])
    {
        // ...other indexes.
        removeOrderFromIndex(security_order_map, it_order);
        removeOrderFromIndex(company_order_map, it_order);

        // ...orders table.
        order_uset.erase(it_order);
    }

    // Remove user and its orders from the index.
    user_order_map.erase(user);
}

void OrderCacheImpl02::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
    auto& security_orders = security_order_map[securityId]; // a reference to a collection of iterators to orders

    auto it_it_order = security_orders.begin();

    while(it_it_order != security_orders.end())
    {
        auto it_order = *it_it_order;

        if(it_order->qty() >= minQty)
        {
            // Remove order from other indexes.
            removeOrderFromIndex(user_order_map, it_order);
            removeOrderFromIndex(company_order_map, it_order);

            // Remove the order from the security index itself.
            it_it_order = security_orders.erase(it_it_order);

            // Remove the order from the orders table.
            this->order_uset.erase(it_order);
        }
        else
        {
            it_it_order = std::next(it_it_order);
        }
    }

    // If all orders for security has been removed - remove the security from the index.
    if(security_orders.empty())
    {
        security_order_map.erase(securityId);
    }
}

unsigned int OrderCacheImpl02::getMatchingSizeForSecurity(const std::string& securityId)
{
    /*
    std::set<
        std::pair<std::unordered_set<Order>::iterator, unsigned int>
    > sell_orders, buy_orders
        ;

    // Get all security sell orders ordered by company

    auto& security_orders = security_order_map[securityId]; // a reference to a collection of iterators to orders

    for(auto& it_order : security_orders)
    {
        if(it_order->side() == "sell")
        {
            sell_orders.insert(std::make_pair(it_order, it_order->qty()));
        }
        else
        {
            buy_orders.insert(std::make_pair(it_order, it_order->qty()));
        }
    }

    auto it_buy_order = buy_orders.begin();

    for(auto& pair : sell_orders)
    {
        // TODO: Continue from here...

    }
    */

    /*
    for(auto& [it_sell_order, remaining_qty] : sell_orders)
    {
        for(auto& [it_buy_order, remaining_qty] : buy_orders)
        {

        }
    }
    */

    return 0;
}

std::vector<Order> OrderCacheImpl02::getAllOrders() const
{
    return std::vector<Order>{ std::begin(order_uset), std::end(order_uset) };
}
