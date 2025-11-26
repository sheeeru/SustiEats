#ifndef RESTAURANT_HPP
#define RESTAURANT_HPP
#include "Address.hpp"
#include "MenuItem.hpp"
#include <vector>
#include <string>
using namespace std;

class Restaurant {
public:
    int id = 0;
    string name;
    Address address;
    vector<MenuItem> menu;
    int ownerId = -1; 
    void addMenuItem(const MenuItem &m);
    void removeMenuItem(int menuItemId);
};
#endif
