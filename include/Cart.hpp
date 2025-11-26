#ifndef CART_HPP
#define CART_HPP
#include "MenuItem.hpp"
#include <vector>
using namespace std;

struct CartItem
{
    MenuItem item;
    int qty = 1;
    double subtotal() const { return item.price * qty; }
};

class Cart
{
public:
    int id = 0;
    vector<CartItem> items;
    void addItem(const MenuItem &mi, int qty);
    void removeItem(int menuId);
    double getTotal() const;
    void clear();
};

#endif
 