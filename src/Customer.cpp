#include "Customer.hpp"
#include "Cart.hpp"
#include "Order.hpp"
#include <memory>
#include <iostream>
#include <map>

using namespace std;

Customer::Customer()
{
    cart = make_unique<Cart>(); // Initialize an empty cart
}

Customer::Customer(const Customer &other)
    : User(other), address(other.address), loyaltyPoints(other.loyaltyPoints), orderIds(other.orderIds)
{
    if (other.cart)
    {
        cart = make_unique<Cart>(*other.cart);
    }
    else
    {
        cart.reset();
    }
}

Customer &Customer::operator=(const Customer &other)
{
    if (this == &other)
        return *this;
    this->id = other.id;
    this->name = other.name;
    this->phone = other.phone;
    this->email = other.email;
    this->password = other.password;
    this->isActive = other.isActive;
    address = other.address;
    loyaltyPoints = other.loyaltyPoints;
    orderIds = other.orderIds;

    if (other.cart)
        cart = make_unique<Cart>(*other.cart);
    else
        cart.reset();

    return *this;
}

void Customer::addToCart(const MenuItem &mi, int qty, int restId, const string &restName)
{
    if (!cart)
        cart = make_unique<Cart>();
    cart->addItem(mi, qty, restId, restName);
}

vector<shared_ptr<Order>> Customer::checkout()
{
    vector<shared_ptr<Order>> completedOrders;

    if (!cart || cart->items.empty())
        return completedOrders;

    map<int, shared_ptr<Order>> ordersMap;

    for (const auto &ci : cart->items)
    {
        if (ordersMap.find(ci.restaurantId) == ordersMap.end())
        {
            auto newOrder = make_shared<Order>();
            newOrder->customerId = this->id;
            newOrder->restaurantId = ci.restaurantId;
            ordersMap[ci.restaurantId] = newOrder;
        }

        OrderItem oi{ci.item, ci.qty, ci.item.price};
        ordersMap[ci.restaurantId]->items.push_back(oi);
    }

    for (auto &pair : ordersMap)
    {
        auto ord = pair.second;
        if (ord->place())
        {
            completedOrders.push_back(ord);
        }
    }

    if (!completedOrders.empty())
    {

        cart->clear(); 
    }

    return completedOrders;
}

vector<int> Customer::viewOrders() const { return orderIds; }

void Customer::applyPointsToOrder(int orderId)
{
    if (loyaltyPoints >= 5)
    {
        consumePoints(5);
    }
}

void Customer::consumePoints(int pts)
{
    if (loyaltyPoints >= pts)
        loyaltyPoints -= pts;
}

void Customer::displayDashboard()
{
    cout << "Customer: " << name << " points=" << loyaltyPoints << "\n";
}