#include "Restaurant.hpp"
#include <algorithm>
using namespace std;

void Restaurant::addMenuItem(const MenuItem &m) { menu.push_back(m); }
void Restaurant::removeMenuItem(int menuItemId)
{
    menu.erase(remove_if(menu.begin(), menu.end(), [&](const MenuItem &mi)
                         { return mi.id == menuItemId; }),
               menu.end());
}