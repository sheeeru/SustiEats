#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP
#include "User.hpp"
#include "Customer.hpp"
#include "Owner.hpp"
#include "Admin.hpp"
#include "Restaurant.hpp"
#include "Order.hpp"
#include <string>
#include <vector>
using namespace std;

struct Persistence {
    static string dataFolder; 
    static void ensureDataFolderExists();
    static void saveCustomer(const Customer &c, const string &filename = "customers.txt");
    static vector<Customer> loadAllCustomers(const string &filename = "customers.txt");
    static void saveOwner(const Owner &o, const string &filename = "owners.txt");
    static vector<Owner> loadAllOwners(const string &filename = "owners.txt");
    static void saveRestaurant(const Restaurant &r, const string &filename = "restaurants.txt");
    static vector<Restaurant> loadAllRestaurants(const string &filename = "restaurants.txt");
    static void saveAllRestaurants(const vector<Restaurant> &restaurants, const string &filename = "restaurants.txt");
    static void saveOrder(const Order &o, const string &filename = "orders.txt");
    static vector<Order> loadAllOrders(const string &filename = "orders.txt");
};

#endif 
