#include "OrderCache.h"

#include <algorithm>
#include <set>
#include <cassert>

class IdOnlyOrder : public Order
{
public:
    explicit IdOnlyOrder(const std::string& orderId) :
        Order{ orderId, "", "", 0, "", "" }
    {
    }
};

void OrderCache::addOrder(Order order)
{
    if(const auto& [it, inserted] = orders_table.insert(order); inserted)
    {
        security_index[it->securityId()].push_back(it);
        user_index[it->user()].push_back(it);
        company_index[it->company()].push_back(it);
    }
}

void OrderCache::removeOrderFromIndex(IndexType& index, OrderIterator it_order)
{
    auto it_key_orders = index.begin();

    while(it_key_orders != index.end())
    {
        auto& [key, orders] = *it_key_orders;

        if(auto it_found = std::find(orders.begin(), orders.end(), it_order); it_found != orders.end())
        {
            orders.erase(it_found);
        }

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

void OrderCache::cancelOrder(const std::string& orderId)
{
    // I need the iterator to the order to remove it from indexes...
    const auto it_order = orders_table.find(IdOnlyOrder{ orderId }); // an ugly hack but works...

    if(it_order != orders_table.end())
    {
        removeOrderFromIndex(user_index, it_order);
        removeOrderFromIndex(security_index, it_order);
        removeOrderFromIndex(company_index, it_order);
    }

    orders_table.erase(it_order);
}

void OrderCache::cancelOrdersForUser(const std::string& user)
{
    // Remove user's orders from...
    for(const auto& it_order : user_index[user])
    {
        // ...other indexes.
        removeOrderFromIndex(security_index, it_order);
        removeOrderFromIndex(company_index, it_order);

        // ...orders table.
        orders_table.erase(it_order);
    }

    // Remove user and its orders from the index.
    user_index.erase(user);
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
    auto& security_orders = security_index[securityId]; // a reference to a collection of iterators to orders

    auto it_it_order = security_orders.begin();

    while(it_it_order != security_orders.end())
    {
        auto it_order = *it_it_order;

        if(it_order->qty() >= minQty)
        {
            // Remove order from other indexes.
            removeOrderFromIndex(user_index, it_order);
            removeOrderFromIndex(company_index, it_order);

            // Remove the order from the security index itself.
            it_it_order = security_orders.erase(it_it_order);

            // Remove the order from the orders table.
            this->orders_table.erase(it_order);
        }
        else
        {
            it_it_order = std::next(it_it_order);
        }
    }

    // If all orders for security has been removed - remove the security from the index.
    if(security_orders.empty())
    {
        security_index.erase(securityId);
    }
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId)
{
    unsigned int total_matched_qty = 0;

    std::vector<std::pair<std::unordered_set<Order>::const_iterator, unsigned int>> sell_orders, buy_orders;

    // From security index select sell and buy orders
    // and attach 'remaining qty' field to each order,
    // initialized by the order's qty.
    for(auto& it_order : security_index[securityId])
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

    //---

    for(auto& [sel_it_order, sell_remaining_qty] : sell_orders)
    {
        for(auto& [buy_it_order, buy_remaining_qty] : buy_orders)
        {
            // Do not match orders from the same company.
            // Skip already fully matched orders.
            if(sel_it_order->company() == buy_it_order->company() || 0 == buy_remaining_qty)
            {
                continue; // continue to the next buy order
            }

            if(sell_remaining_qty > buy_remaining_qty)
            {
                const unsigned int matched_qty = buy_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty -= matched_qty;
                buy_remaining_qty = 0;

                continue; // continue to the next buy order
            }
            else if(sell_remaining_qty < buy_remaining_qty)
            {
                const unsigned int matched_qty = sell_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty = 0;
                buy_remaining_qty -= matched_qty;

                break; // stop processing buy orders and continue to the next sell order
            }
            else if(sell_remaining_qty == buy_remaining_qty)
            {
                const unsigned int matched_qty = sell_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty = 0;
                buy_remaining_qty = 0;

                break; // stop processing buy orders and continue to the next sell order
            }
        }
    }

    return total_matched_qty;
}

std::vector<Order> OrderCache::getAllOrders() const
{
    return std::vector<Order>{ std::begin(orders_table), std::end(orders_table) };
}
