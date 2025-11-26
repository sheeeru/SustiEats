#ifndef CUSTOMER_HPP
#define CUSTOMER_HPP
#include "User.hpp"
#include "Address.hpp"
#include "Cart.hpp"
#include <memory>
#include <vector>
using namespace std;

class Order;

class Customer : public User {
public:
    Address address;
    int loyaltyPoints = 0;
    unique_ptr<Cart> cart;
    vector<int> orderIds; 
    Customer();
    Customer(const Customer &other);
    Customer& operator=(const Customer &other);
    Customer(Customer &&) = default;
    Customer& operator=(Customer &&) = default;
    void addToCart(const MenuItem &mi, int qty);
    shared_ptr<Order> checkout(int restaurantId);
    vector<int> viewOrders() const;
    void applyPointsToOrder(int orderId);
    void consumePoints(int pts);
    void displayDashboard() override;
    ~Customer() = default;
};

#endif
