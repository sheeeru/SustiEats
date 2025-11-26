#include "Order.hpp"
using namespace std;

bool Order::place()
{
    if (items.empty())
        return false;
    double t = 0.0;
    for (const auto &it : items)
        t += it.subtotal();
    total = t;
    status = "Placed";
    return true;
}

bool Order::cancel()
{
    if (status == "Placed" || status == "Pending")
    {
        status = "Cancelled";
        return true;
    }
    return false;
}