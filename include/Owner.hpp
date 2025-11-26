#ifndef OWNER_HPP
#define OWNER_HPP
#include "User.hpp"
#include <vector>
#include "MenuItem.hpp"
using namespace std;

class Owner : public User
{
public:
    vector<int> restaurantIds;
    Owner() = default;
    void addMenuItem(int restaurantId, const MenuItem &mi);
    void updateOrderStatus(int orderId, const string &status);
};

#endif
