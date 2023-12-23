#pragma once

#include "Order.h"

struct OrderHash
{
    std::hash<std::string> hash;

    std::size_t operator () (const Order& o) const
    {
        return hash(o.orderId());
    }
};
