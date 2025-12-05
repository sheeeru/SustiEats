#include "Order.hpp"
using namespace std;

bool Order::place()
{
    if (items.empty()) return false;
    double t = 0.0;
    for (const auto &it : items) t += it.subtotal();
    total = t;
    status = "Placed";
    return true;
}

// Only change to Dispatched if it is currently 'Placed'
bool Order::dispatch()
{
    if (status == "Placed") {
        status = "Dispatched";
        return true;
    }
    return false;
}

// Only Cancel if it is 'Placed' (Cannot cancel if already Dispatched)
bool Order::cancel()
{
    if (status == "Placed") {
        status = "Cancelled";
        return true;
    }
    return false;
}