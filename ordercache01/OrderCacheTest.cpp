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
        {"o1", "s1", "Sell", 100, "u1", "c1"},
        {"o2", "s1", "Sell", 100, "u1", "c1"},
        {"o3", "s1", "Sell", 100, "u1", "c1"},
        {"o4", "s1", "Sell", 100, "u1", "c1"},
        {"o5", "s1", "Sell", 100, "u1", "c1"},
        {"o6", "s1", "Sell", 100, "u1", "c1"},
        {"o7", "s1", "Sell", 100, "u1", "c1"},
        {"o8", "s1", "Sell", 100, "u1", "c1"},
        {"o9", "s1", "Sell", 100, "u1", "c1"},
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
        {"o1", "s1", "Sell", 100, "u1", "c1"},
        {"o2", "s1", "Sell", 100, "u1", "c1"},
        {"o3", "s1", "Sell", 100, "u1", "c1"},
        {"o4", "s1", "Sell", 100, "u1", "c1"},
        {"o5", "s1", "Sell", 100, "u1", "c1"},
        {"o6", "s1", "Sell", 100, "u1", "c1"},
        {"o7", "s1", "Sell", 100, "u1", "c1"},
        {"o8", "s1", "Sell", 100, "u1", "c1"},
        {"o9", "s1", "Sell", 100, "u1", "c1"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "Sell", 100, "u1", "c1"},
        {"o3", "s1", "Sell", 100, "u1", "c1"},
        {"o5", "s1", "Sell", 100, "u1", "c1"},
        {"o7", "s1", "Sell", 100, "u1", "c1"},
        {"o9", "s1", "Sell", 100, "u1", "c1"},
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
        {"o1", "s1", "Sell", 100, "u1", "c1"},
        {"o2", "s1", "Sell", 100, "u1", "c1"},
        {"o3", "s1", "Sell", 100, "u1", "c1"},
        {"o4", "s1", "Sell", 100, "u2", "c1"},
        {"o5", "s1", "Sell", 100, "u2", "c1"},
        {"o7", "s1", "Sell", 100, "u3", "c1"},
        {"o6", "s1", "Sell", 100, "u2", "c1"},
        {"o8", "s1", "Sell", 100, "u3", "c1"},
        {"o9", "s1", "Sell", 100, "u3", "c1"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "Sell", 100, "u1", "c1"},
        {"o2", "s1", "Sell", 100, "u1", "c1"},
        {"o3", "s1", "Sell", 100, "u1", "c1"},
        {"o7", "s1", "Sell", 100, "u3", "c1"},
        {"o8", "s1", "Sell", 100, "u3", "c1"},
        {"o9", "s1", "Sell", 100, "u3", "c1"},
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
        {"o1", "s1", "Sell", 300, "u1", "c1"},
        {"o2", "s2", "Sell", 199, "u1", "c1"},
        {"o3", "s3", "Sell", 100, "u1", "c1"},
        {"o4", "s1", "Sell", 200, "u2", "c1"},
        {"o5", "s2", "Sell", 200, "u2", "c1"},
        {"o7", "s3", "Sell", 100, "u3", "c1"},
        {"o6", "s1", "Sell", 300, "u2", "c1"},
        {"o8", "s2", "Sell", 300, "u3", "c1"},
        {"o9", "s3", "Sell", 400, "u3", "c1"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "Sell", 300, "u1", "c1"},
        {"o2", "s2", "Sell", 199, "u1", "c1"},
        {"o3", "s3", "Sell", 100, "u1", "c1"},
        {"o4", "s1", "Sell", 200, "u2", "c1"},
        {"o6", "s1", "Sell", 300, "u2", "c1"},
        {"o7", "s3", "Sell", 100, "u3", "c1"},
        {"o9", "s3", "Sell", 400, "u3", "c1"},
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
        {"o1", "s1", "Sell", 300, "u1", "c1"},
        {"o2", "s1", "Sell", 200, "u1", "c2"},
        {"o3", "s1", "Sell", 100, "u1", "c3"},
        {"o4", "s1", "Sell", 400, "u1", "c4"},
        {"o5", "s1", "Buy" , 100, "u1", "c1"},
        {"o6", "s1", "Buy" , 200, "u1", "c2"},
        {"o7", "s1", "Buy" , 200, "u1", "c3"},
        {"o8", "s1", "Buy" , 100, "u1", "c4"},
        {"o9", "s1", "Buy" , 300, "u1", "c5"},
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

        {"o1", "s1", "Sell", 100, "u1", "a"},
        {"o2", "s1", "Sell", 150, "u1", "b"},
        {"o3", "s1", "Sell",  50, "u1", "c"},
        {"o4", "s1", "Sell",  75, "u1", "d"},

        {"o7", "s1", "Buy", 300, "u1", "c"},
        {"o6", "s1", "Buy",  25, "u1", "b"},
        {"o5", "s1", "Buy",  20, "u1", "a"},
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
        {"o1", "s1", "Buy" , 100, "u1", "a"},
        {"o2", "s1", "Sell", 200, "u1", "a"},
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
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
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
        {"o1", "s1", "Sell", 100, "u1", "a"},
        {"o2", "s1", "Sell", 200, "u1", "b"},
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
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
        {"o3", "s1", "Sell", 200, "u1", "a"},
        {"o4", "s1", "Sell", 100, "u1", "b"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("s1");

    EXPECT_EQ(matched_amount, 300);
}

TEST(OrderCacheTest, MatchesOrders07)
{
    const std::vector<Order> added_orders{
        {"OrdId1", "SecId1", "Buy" , 1000, "User1", "CompanyA"},
        {"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"},
        {"OrdId3", "SecId1", "Sell",  500, "User3", "CompanyA"},
        {"OrdId4", "SecId2", "Buy" ,  600, "User4", "CompanyC"},
        {"OrdId5", "SecId2", "Buy" ,  100, "User5", "CompanyB"},
        {"OrdId6", "SecId3", "Buy" , 1000, "User6", "CompanyD"},
        {"OrdId7", "SecId2", "Buy" , 2000, "User7", "CompanyE"},
        {"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"},
    };

    OrderCache cache;

    for(const auto& order : added_orders)
    {
        cache.addOrder(order);
    }

    const auto matched_amount = cache.getMatchingSizeForSecurity("SecId2");

    EXPECT_EQ(matched_amount, 2700);
}

TEST(OrderCacheTest, CancelsNotAddedOrder)
{
    const std::vector<Order> added_orders{
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
        {"o3", "s1", "Sell", 200, "u1", "a"},
        {"o4", "s1", "Sell", 100, "u1", "b"},
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
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
        {"o3", "s1", "Sell", 200, "u1", "a"},
        {"o4", "s1", "Sell", 100, "u1", "b"},
        {"o1", "s1", "Buy", 100, "u1", "a"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
        {"o3", "s1", "Sell", 200, "u1", "a"},
        {"o4", "s1", "Sell", 100, "u1", "b"},
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
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
        {"o3", "s1", "Sell", 200, "u1", "a"},
        {"o4", "s1", "Sell", 100, "u1", "b"},
        {"o1", "s7", "Buy", 700, "u7", "f"},
    };

    const std::vector<Order> expected_orders{
        {"o1", "s1", "Buy", 100, "u1", "a"},
        {"o2", "s1", "Buy", 200, "u1", "b"},
        {"o3", "s1", "Sell", 200, "u1", "a"},
        {"o4", "s1", "Sell", 100, "u1", "b"},
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
