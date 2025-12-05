#include "Cart.hpp"
#include <algorithm>
using namespace std;

void Cart::addItem(const MenuItem &mi, int qty, int restId, const string &restName)
{
    for (auto &ci : items)
    {
        if (ci.item.id == mi.id && ci.restaurantId == restId)
        {
            ci.qty += qty;
            return;
        }
    }
    // Create new item with restaurant info
    CartItem ci{mi, qty, restId, restName};
    items.push_back(ci);
}

void Cart::removeItem(int menuId)
{
    items.erase(remove_if(items.begin(), items.end(), [&](const CartItem &c)
                          { return c.item.id == menuId; }),
                items.end());
}

double Cart::getTotal() const
{
    double t = 0.0;
    for (const auto &ci : items)
        t += ci.subtotal();
    return t;
}

void Cart::clear() { items.clear(); }