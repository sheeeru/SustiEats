#include "Owner.hpp"
#include <iostream>
#include "Restaurant.hpp"
using namespace std;

void Owner::addMenuItem(int restaurantId, const MenuItem &mi)
{
    cout << "Owner::addMenuItem called for restaurant " << restaurantId << " item " << mi.name<<"\n";
}

void Owner::updateOrderStatus(int orderId, const string &status)
{
    cout << "Owner::updateOrderStatus(" << orderId << "," << status << ")\n";
}