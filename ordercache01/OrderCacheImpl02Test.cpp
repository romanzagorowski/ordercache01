#include <gtest/gtest.h>

#include "OrderCacheImpl02.h"

bool operator == (const Order& o1, const Order& o2)
{
    return o1.orderId() == o2.orderId();
}

TEST(OrderCacheImpl02Test, ReturnsAllAddedOrders)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "sell", 100, "u1", "c1"},
        {"o2", "s1", "sell", 100, "u1", "c1"},
        {"o3", "s1", "sell", 100, "u1", "c1"},
        {"o4", "s1", "sell", 100, "u1", "c1"},
        {"o5", "s1", "sell", 100, "u1", "c1"},
        {"o6", "s1", "sell", 100, "u1", "c1"},
        {"o7", "s1", "sell", 100, "u1", "c1"},
        {"o8", "s1", "sell", 100, "u1", "c1"},
        {"o9", "s1", "sell", 100, "u1", "c1"},
    };

    OrderCacheImpl02 cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto returned_orders = cache.getAllOrders();

    EXPECT_EQ(added_orders, returned_orders);
}

TEST(OrderCacheImpl02Test, DoesNotReturnCanceledOrders)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "sell", 100, "u1", "c1"},
        {"o2", "s1", "sell", 100, "u1", "c1"},
        {"o3", "s1", "sell", 100, "u1", "c1"},
        {"o4", "s1", "sell", 100, "u1", "c1"},
        {"o5", "s1", "sell", 100, "u1", "c1"},
        {"o6", "s1", "sell", 100, "u1", "c1"},
        {"o7", "s1", "sell", 100, "u1", "c1"},
        {"o8", "s1", "sell", 100, "u1", "c1"},
        {"o9", "s1", "sell", 100, "u1", "c1"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "sell", 100, "u1", "c1"},
        {"o3", "s1", "sell", 100, "u1", "c1"},
        {"o5", "s1", "sell", 100, "u1", "c1"},
        {"o7", "s1", "sell", 100, "u1", "c1"},
        {"o9", "s1", "sell", 100, "u1", "c1"},
    };

    OrderCacheImpl02 cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    cache.cancelOrder("o2");
    cache.cancelOrder("o4");
    cache.cancelOrder("o6");
    cache.cancelOrder("o8");

    const auto returned_orders = cache.getAllOrders();

    EXPECT_EQ(expected_orders, returned_orders);
}