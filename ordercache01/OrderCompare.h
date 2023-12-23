#pragma once

#include "Order.h"

struct OrderCompare
{
    bool operator () (const Order& o1, const Order& o2) const
    {
        return o1.orderId() == o2.orderId();
    }
};
