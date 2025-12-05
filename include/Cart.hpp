#ifndef CART_HPP
#define CART_HPP
#include "MenuItem.hpp"
#include <vector>
#include <string>
using namespace std;

struct CartItem
{
    MenuItem item;
    int qty = 1;
    int restaurantId = -1;
    string restaurantName;
    double subtotal() const { return item.price * qty; }
};

class Cart
{
public:
    int id = 0;
    vector<CartItem> items; // items in the cart
    void addItem(const MenuItem &mi, int qty, int restId, const string &restName);
    void removeItem(int menuId);
    double getTotal() const;
    void clear();
};

#endif