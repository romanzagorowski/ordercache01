#include "OrderCache.h"

#include <algorithm>
#include <set>

void OrderCache::addOrder(Order order)
{
    if(const auto& [it_order, inserted] = orders_table.insert(order); inserted)
    {
        addOrderToIndex(security_index, it_order->securityId(), it_order);
        addOrderToIndex(user_index, it_order->user(), it_order);
        addOrderToIndex(company_index, it_order->company(), it_order);
    }
}

void OrderCache::cancelOrder(const std::string& orderId)
{
    class IdOnlyOrder : public Order
    {
    public:
        explicit IdOnlyOrder(const std::string& orderId) :
            Order{ orderId, "", "", 0, "", "" }
        {
        }
    };

    // An iterator to the order is needed to remove it form the indexes.
    const auto it_order = orders_table.find(IdOnlyOrder{ orderId });

    if(it_order != orders_table.end())
    {
        removeOrderFromIndex(user_index, it_order);
        removeOrderFromIndex(security_index, it_order);
        removeOrderFromIndex(company_index, it_order);

        orders_table.erase(it_order);
    }
}

void OrderCache::cancelOrdersForUser(const std::string& user)
{
    // Get a range with all orders for the user.
    auto [range_begin, range_end] = user_index.equal_range(user);

    // For every order in range...
    for(auto& it = range_begin; it != range_end; ++it)
    {
        auto& [key, it_order] = *it;

        // ...remove it from the other indexes.
        removeOrderFromIndex(security_index, it_order);
        removeOrderFromIndex(company_index, it_order);

        // ...remove it from the orders table.
        orders_table.erase(it_order);
    }

    // Finally remove the key (and all its orders) from the index.
    user_index.erase(user);
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
    // Get a range with all orders for the security.
    auto [range_begin, range_end] = security_index.equal_range(securityId);

    auto& it = range_begin;

    while(it != range_end)
    {
        auto& [key, it_order] = *it;

        // If order's qty is greater than minQty...
        if(it_order->qty() >= minQty)
        {
            // ...remove the order from the other indexes.
            removeOrderFromIndex(user_index, it_order);
            removeOrderFromIndex(company_index, it_order);

            // ...remove the order from the orders table.
            orders_table.erase(it_order);

            // ...remove the order from the security index itself.
            it = security_index.erase(it);
        }
        else
        {
            // The order has qty smaller than minQty. Move to the next order.
            it = std::next(it);
        }
    }
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId)
{
    unsigned int total_matched_qty = 0;

    std::vector<std::pair<OrderIterator, unsigned int>> sell_orders, buy_orders;

    // Get a range with all orders for the security.
    auto [range_begin, range_end] = security_index.equal_range(securityId);

    // Select all 'Sell' and 'Buy' orders for the given security
    // and attach 'remaining qty' field to each of it (initialized by the order's qty).
    for(auto it = range_begin; it != range_end; ++it)
    {
        const auto& [key, it_order] = *it;

        if(it_order->side() == "Sell")
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
                continue; // Continue to the next buy order.
            }

            if(sell_remaining_qty > buy_remaining_qty)
            {
                const unsigned int matched_qty = buy_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty -= matched_qty;
                buy_remaining_qty = 0;

                continue; // The remaining qty for the buy order has been depleted.
                          // Continue to the next buy order.
            }
            else if(sell_remaining_qty < buy_remaining_qty)
            {
                const unsigned int matched_qty = sell_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty = 0;
                buy_remaining_qty -= matched_qty;

                break; // The remaining qty for the sell order has been depleted.
                       // Stop processing the buy orders and continue to the next sell order.
            }
            else if(sell_remaining_qty == buy_remaining_qty)
            {
                const unsigned int matched_qty = sell_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty = 0;
                buy_remaining_qty = 0;

                break; // The remaining qty for the sell order has been depleted.
                       // Stop processing the buy orders and continue to the next sell order.
            }
        }
    }

    return total_matched_qty;
}

std::vector<Order> OrderCache::getAllOrders() const
{
    return std::vector<Order>{ std::begin(orders_table), std::end(orders_table) };
}

void OrderCache::addOrderToIndex(IndexType& index, const std::string& key, const OrderIterator& it_order)
{
    index.insert(std::make_pair(key, it_order));
}

void OrderCache::removeOrderFromIndex(IndexType& index, const OrderIterator& it_order)
{
    auto it = index.begin(); 
    
    while(it != index.end())
    {
        if(it->second == it_order)
        {
            it = index.erase(it);
        }
        else
        {
            it = std::next(it);
        }
    }
}
