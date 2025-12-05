#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP

#include "User.hpp"
#include "Customer.hpp"
#include "Owner.hpp"
#include "Restaurant.hpp"
#include "Order.hpp"
#include <string>
#include <vector>

using namespace std;

struct Persistence {
    static string dataFolder; 
    static void ensureDataFolderExists();
    static int getNextId(const string &filename);

    // --- NEW: Admin Verification ---
    static bool verifyAdmin(int id, const string &password, const string &filename = "admin.txt");

    // Customers
    static void saveCustomer(const Customer &c, const string &filename = "customers.txt");
    static vector<Customer> loadAllCustomers(const string &filename = "customers.txt");
    static void saveAllCustomers(const vector<Customer> &customers, const string &filename = "customers.txt");

    // Owners
    static void saveOwner(const Owner &o, const string &filename = "owners.txt");
    static vector<Owner> loadAllOwners(const string &filename = "owners.txt");
    // --- NEW: Needed to save ban status ---
    static void saveAllOwners(const vector<Owner> &owners, const string &filename = "owners.txt");

    // Restaurants
    static void saveRestaurant(const Restaurant &r, const string &filename = "restaurants.txt");
    static vector<Restaurant> loadAllRestaurants(const string &filename = "restaurants.txt");
    static void saveAllRestaurants(const vector<Restaurant> &restaurants, const string &filename = "restaurants.txt");

    // Orders
    static void saveOrder(const Order &o, const string &filename = "orders.txt");
    static vector<Order> loadAllOrders(const string &filename = "orders.txt");
    static void saveAllOrders(const vector<Order> &orders, const string &filename = "orders.txt");
};

#endif