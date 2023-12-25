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
    const auto it_order = order_uset.find(IdOnlyOrder{ orderId });

    if(it_order != order_uset.end())
    {
        removeOrderFromIndex(user_order_map, it_order);
        removeOrderFromIndex(security_order_map, it_order);
        removeOrderFromIndex(company_order_map, it_order);
    }

    order_uset.erase(it_order);
}

void OrderCache::cancelOrdersForUser(const std::string& user)
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

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
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

bool operator < (
    const std::pair<std::unordered_set<Order>::const_iterator, unsigned int>& lhs,
    const std::pair<std::unordered_set<Order>::const_iterator, unsigned int>& rhs
    )
{
    return lhs.first->orderId() < rhs.first->orderId();
}

bool operator > (
    const std::pair<std::unordered_set<Order>::const_iterator, unsigned int>& lhs,
    const std::pair<std::unordered_set<Order>::const_iterator, unsigned int>& rhs
    )
{
    return lhs.first->orderId() > rhs.first->orderId();
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId)
{
    unsigned int total_matched_qty = 0;

    std::vector<std::pair<std::unordered_set<Order>::const_iterator, unsigned int>> sell_orders, buy_orders;

    // From security index extract sell and buy orders.
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

    // Sort sell orders ascending by order id.
    std::sort(sell_orders.begin(), sell_orders.end(), std::less{});

    // Sort buy orders descending by order id.
    std::sort(buy_orders.begin(), buy_orders.end(), std::greater{});

    //---

    for(auto& [sel_it_order, sell_qty_left] : sell_orders)
    {
        for(auto& [buy_it_order, buy_qty_left] : buy_orders)
        {
            if(sel_it_order->company() == buy_it_order->company() || 0 == buy_qty_left)
            {
                continue; // continue to the next buy order
            }

            if(sell_qty_left > buy_qty_left)
            {
                const unsigned int matched_qty = buy_qty_left;

                total_matched_qty += matched_qty;

                sell_qty_left -= matched_qty;
                buy_qty_left = 0;

                continue; // continue to the next buy order
            }
            else if(sell_qty_left < buy_qty_left)
            {
                const unsigned int matched_qty = sell_qty_left;

                total_matched_qty += matched_qty;

                sell_qty_left = 0;
                buy_qty_left -= matched_qty;

                break; // stop processing buy orders and continue to the next sell order
            }
            else if(sell_qty_left == buy_qty_left)
            {
                const unsigned int matched_qty = sell_qty_left;

                total_matched_qty += matched_qty;

                sell_qty_left = 0;
                buy_qty_left = 0;

                break; // stop processing buy orders and continue to the next sell order
            }
        }
    }

    return total_matched_qty;
}

std::vector<Order> OrderCache::getAllOrders() const
{
    return std::vector<Order>{ std::begin(order_uset), std::end(order_uset) };
}
