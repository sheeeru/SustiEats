#ifndef ORDER_HPP
#define ORDER_HPP
#include "MenuItem.hpp"
#include <vector>
#include <string>
using namespace std;

struct OrderItem
{
    MenuItem itemSnapshot;
    int qty;
    double unitPrice;
    double subtotal() const { return unitPrice * qty; }
};

class Order
{
public:
    int id = 0;
    int customerId = -1;
    int restaurantId = -1;
    vector<OrderItem> items;
    double total = 0.0;
    string status = "Pending";

    bool place();
    bool cancel();
};

#endif 
