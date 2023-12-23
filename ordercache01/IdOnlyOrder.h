#pragma once

#include "Order.h"

class IdOnlyOrder : public Order
{
public:
    explicit IdOnlyOrder(const std::string& orderId) :
        Order{ orderId, "", "", 0, "", "" }
    {
    }
};
