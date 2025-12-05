#ifndef MENUITEM_HPP
#define MENUITEM_HPP
#include <string>
using namespace std;

struct MenuItem
{
    int id = 0;
    string name;
    double price = 0.0;
    bool available = true;
};

#endif