#include <gtest/gtest.h>
#include <ostream>
#include <algorithm>

#include "OrderCache.h"

std::ostream& operator << (std::ostream& os, const Order& o)
{
    return os 
        << "id: '" << o.orderId()
        << "', security: " << o.securityId()
        << "', side: '" << o.side()
        << "', user: '" << o.user()
        << "', company: '" << o.company()
        << "', qty: " << o.qty()
        << std::endl
        ;
}

bool operator == (const Order& o1, const Order& o2)
{
    return o1.orderId() == o2.orderId();
}

bool operator < (const Order& o1, const Order& o2)
{
    return o1.orderId() < o2.orderId();
}

TEST(OrderCacheTest, ReturnsAllAddedOrders)
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

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));

    EXPECT_EQ(added_orders, returned_orders);
}

TEST(OrderCacheTest, ReturnsNoOrderIfNoneAdded)
{
    OrderCache cache;
    const auto returned_orders = cache.getAllOrders();

    EXPECT_TRUE(returned_orders.empty());
}

TEST(OrderCacheTest, CancelsOrders)
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

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    cache.cancelOrder("o2");
    cache.cancelOrder("o4");
    cache.cancelOrder("o6");
    cache.cancelOrder("o8");

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));

    EXPECT_EQ(expected_orders, returned_orders);
}

TEST(OrderCacheTest, CancelsUserOrders)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "sell", 100, "u1", "c1"},
        {"o2", "s1", "sell", 100, "u1", "c1"},
        {"o3", "s1", "sell", 100, "u1", "c1"},
        {"o4", "s1", "sell", 100, "u2", "c1"},
        {"o5", "s1", "sell", 100, "u2", "c1"},
        {"o7", "s1", "sell", 100, "u3", "c1"},
        {"o6", "s1", "sell", 100, "u2", "c1"},
        {"o8", "s1", "sell", 100, "u3", "c1"},
        {"o9", "s1", "sell", 100, "u3", "c1"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "sell", 100, "u1", "c1"},
        {"o2", "s1", "sell", 100, "u1", "c1"},
        {"o3", "s1", "sell", 100, "u1", "c1"},
        {"o7", "s1", "sell", 100, "u3", "c1"},
        {"o8", "s1", "sell", 100, "u3", "c1"},
        {"o9", "s1", "sell", 100, "u3", "c1"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    cache.cancelOrdersForUser("u2");

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));

    EXPECT_EQ(expected_orders, returned_orders);
}

TEST(OrderCacheTest, CancelsSecurityOrdersWithMinQty)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "sell", 300, "u1", "c1"},
        {"o2", "s2", "sell", 199, "u1", "c1"},
        {"o3", "s3", "sell", 100, "u1", "c1"},
        {"o4", "s1", "sell", 200, "u2", "c1"},
        {"o5", "s2", "sell", 200, "u2", "c1"},
        {"o7", "s3", "sell", 100, "u3", "c1"},
        {"o6", "s1", "sell", 300, "u2", "c1"},
        {"o8", "s2", "sell", 300, "u3", "c1"},
        {"o9", "s3", "sell", 400, "u3", "c1"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "sell", 300, "u1", "c1"},
        {"o2", "s2", "sell", 199, "u1", "c1"},
        {"o3", "s3", "sell", 100, "u1", "c1"},
        {"o4", "s1", "sell", 200, "u2", "c1"},
        {"o6", "s1", "sell", 300, "u2", "c1"},
        {"o7", "s3", "sell", 100, "u3", "c1"},
        {"o9", "s3", "sell", 400, "u3", "c1"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    cache.cancelOrdersForSecIdWithMinimumQty("s2", 200);

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));


    EXPECT_EQ(expected_orders, returned_orders);
}

TEST(OrderCacheTest, MatchesOrders)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "sell", 300, "u1", "c1"},
        {"o2", "s1", "sell", 200, "u1", "c2"},
        {"o3", "s1", "sell", 100, "u1", "c3"},
        {"o4", "s1", "sell", 400, "u1", "c4"},
        {"o5", "s1", "buy" , 100, "u1", "c1"},
        {"o6", "s1", "buy" , 200, "u1", "c2"},
        {"o7", "s1", "buy" , 200, "u1", "c3"},
        {"o8", "s1", "buy" , 100, "u1", "c4"},
        {"o9", "s1", "buy" , 300, "u1", "c5"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 900);
}

TEST(OrderCacheTest, MatchesOrders02)
{
    const std::vector<Order> added_orders{

        {"o1", "s1", "sell", 100, "u1", "a"},
        {"o2", "s1", "sell", 150, "u1", "b"},
        {"o3", "s1", "sell",  50, "u1", "c"},
        {"o4", "s1", "sell",  75, "u1", "d"},

        {"o7", "s1", "buy", 300, "u1", "c"},
        {"o6", "s1", "buy",  25, "u1", "b"},
        {"o5", "s1", "buy",  20, "u1", "a"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 345);
}

TEST(OrderCacheTest, MatchesOrders03)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "buy" , 100, "u1", "a"},
        {"o2", "s1", "sell", 200, "u1", "a"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 0);
}

TEST(OrderCacheTest, MatchesOrders04)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 0);
}

TEST(OrderCacheTest, MatchesOrders05)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "sell", 100, "u1", "a"},
        {"o2", "s1", "sell", 200, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 0);
}

TEST(OrderCacheTest, MatchesOrders06)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
        {"o3", "s1", "sell", 200, "u1", "a"},
        {"o4", "s1", "sell", 100, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 300);
}

TEST(OrderCacheTest, CancelsNotAddedOrder)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
        {"o3", "s1", "sell", 200, "u1", "a"},
        {"o4", "s1", "sell", 100, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    cache.cancelOrder("o9999");

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));

    EXPECT_EQ(added_orders, returned_orders);
}

TEST(OrderCacheTest, HandlesAddingTheSameOrderTwice)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
        {"o3", "s1", "sell", 200, "u1", "a"},
        {"o4", "s1", "sell", 100, "u1", "b"},
        {"o1", "s1", "buy", 100, "u1", "a"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
        {"o3", "s1", "sell", 200, "u1", "a"},
        {"o4", "s1", "sell", 100, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));

    EXPECT_EQ(expected_orders, returned_orders);
}

TEST(OrderCacheTest, HandlesAddingOrderWithTheSameIdTwice)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
        {"o3", "s1", "sell", 200, "u1", "a"},
        {"o4", "s1", "sell", 100, "u1", "b"},
        {"o1", "s7", "buy", 700, "u7", "f"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "buy", 100, "u1", "a"},
        {"o2", "s1", "buy", 200, "u1", "b"},
        {"o3", "s1", "sell", 200, "u1", "a"},
        {"o4", "s1", "sell", 100, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    auto returned_orders = cache.getAllOrders();
    std::sort(std::begin(returned_orders), std::end(returned_orders));

    EXPECT_EQ(expected_orders, returned_orders);
}