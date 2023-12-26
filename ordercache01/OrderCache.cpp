#include "OrderCache.h"

#include <algorithm>
#include <set>
#include <map>

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
    /*
       The idea is to:
       -    aggregate qty of companies to limit the number of iterations
       -    sort buy and sell company qty in opposite direction to let qty from the same company
            to be matched with other companies before they hit themselves
    */

    unsigned int total_matched_qty = 0;

    // SELECT company, SUM(qty) FROM orders WHERE securityId = ? AND side = 'Sell' GROUP BY company ORDER BY company ASC
    std::map<std::string, unsigned int, std::less<std::string>> sell_company_qty;

    // SELECT company, SUM(qty) FROM orders WHERE securityId = ? AND side = 'Buy' GROUP BY company ORDER BY company DESC
    std::map<std::string, unsigned int, std::greater<std::string>> buy_company_qty;

    // Get a range with all orders for the security.
    auto [range_begin, range_end] = security_index.equal_range(securityId);

    // Get orders qty aggregated by company.
    for(auto it = range_begin; it != range_end; ++it)
    {
        const auto& [key, it_order] = *it;

        if(it_order->side() == "Sell")
        {
            sell_company_qty[it_order->company()] += it_order->qty();
        }
        else
        {
            buy_company_qty[it_order->company()] += it_order->qty();
        }
    }

    //---

    for(auto& [sell_company, sell_remaining_qty] : sell_company_qty)
    {
        for(auto& [buy_company, buy_remaining_qty] : buy_company_qty)
        {
            // Do not match qty from the same company.
            // Skip already fully matched qty.
            if(sell_company == buy_company || 0 == buy_remaining_qty)
            {
                continue; // Continue to the buy qty of the next company.
            }

            if(sell_remaining_qty > buy_remaining_qty)
            {
                const unsigned int matched_qty = buy_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty -= matched_qty;
                buy_remaining_qty = 0;

                continue; // The remaining qty for the long side has been depleted.
                          // Continue to the buy qty of the next company.
            }
            else if(sell_remaining_qty <= buy_remaining_qty)
            {
                const unsigned int matched_qty = sell_remaining_qty;

                total_matched_qty += matched_qty;

                sell_remaining_qty = 0;
                buy_remaining_qty -= matched_qty;

                break; // The remaining qty for the short side has been depleted.
                       // Stop processing the buy qty and continue to the sell qty of the next company.
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
