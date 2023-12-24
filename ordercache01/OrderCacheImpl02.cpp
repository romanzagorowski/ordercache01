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

/*
bool operator < (
    const std::pair<std::unordered_set<Order>::iterator, unsigned int>& lhs,
    const std::pair<std::unordered_set<Order>::iterator, unsigned int>& rhs
    )
{
    return lhs.first->orderId() < rhs.first->orderId();
}
*/

unsigned int OrderCacheImpl02::getMatchingSizeForSecurity(const std::string& securityId)
{
    unsigned int total_matched_qty = 0;
/*
    std::vector<std::pair<std::unordered_set<Order>::iterator, unsigned int>> sell_orders, buy_orders;

    // TODO: Populate sell and buy orders vector.
    for(auto& it_order : security_order_map[securityId])
    {
        if(it_order->side() == "sell")
        {
            sell_orders.push_back(std::make_pair(it_order, it_order->qty()));
        }
        else
        {
            buy_orders.push_back(std::make_pair(it_order, it_order->qty()));
        }
    }

    // TODO: Sort one vector ascending and another descending.
    std::sort(sell_orders.begin(), sell_orders.end());
    std::sort(buy_orders.begin(), buy_orders.end());

    //---

    for(auto&& it_sell_pair = sell_orders.begin(); it_sell_pair != sell_orders.end(); ++it_sell_pair)
    {
        auto& [it_sell_order, sell_qty_left] = *it_sell_pair;

        for(auto&& it_buy_pair = buy_orders.rbegin(); it_buy_pair != buy_orders.rend(); ++it_buy_pair)
        {
            auto& [it_buy_order, buy_qty_left] = *it_buy_pair;

            if(it_sell_order->company() != it_buy_order->company() && buy_qty_left != 0)
            {
                if(sell_qty_left > buy_qty_left)
                {
                    const unsigned int matched_qty = buy_qty_left;

                    total_matched_qty += matched_qty;

                    sell_qty_left -= matched_qty;
                    buy_qty_left = 0;

                    // buy_qty_left has been depleted
                    // take the next buy_order
                    continue;
                }
                else if(sell_qty_left < buy_qty_left)
                {
                    const unsigned int matched_qty = sell_qty_left;

                    total_matched_qty += matched_qty;

                    sell_qty_left = 0;
                    buy_qty_left -= matched_qty;

                    // sell_qty_left has been depleted
                    // take the next sell_order
                    break;
                }
                else // sell_qty_left == buy_qty_left
                {
                    const unsigned int matched_qty = sell_qty_left;

                    total_matched_qty += matched_qty;

                    sell_qty_left = 0;
                    buy_qty_left = 0;

                    // sell_qty_left has been depleted
                    // take the next sell_order
                    break;
                }
            }
        }
    }
*/
    return total_matched_qty;
}

std::vector<Order> OrderCacheImpl02::getAllOrders() const
{
    return std::vector<Order>{ std::begin(order_uset), std::end(order_uset) };
}
