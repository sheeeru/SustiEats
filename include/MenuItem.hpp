#ifndef MENUITEM_HPP
#define MENUITEM_HPP
#include <string>
#include "Category.hpp"
using namespace std;

struct MenuItem
{
    int id = 0;
    string name;
    Category category;
    double price = 0.0;
    bool available = true;
};

#endif