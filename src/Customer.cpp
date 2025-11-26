#include "Customer.hpp"
#include "Cart.hpp"
#include "Order.hpp"
#include "LoyaltyManager.hpp"
#include <memory>
#include <iostream>
using namespace std;

Customer::Customer()
{
    cart = make_unique<Cart>();
}

Customer::Customer(const Customer &other)
    : User(other), address(other.address), loyaltyPoints(other.loyaltyPoints), orderIds(other.orderIds)
{
    if (other.cart)
    {
        cart = make_unique<Cart>(*other.cart); // uses Cart's copy ctor (vector copy)
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

void Customer::addToCart(const MenuItem &mi, int qty)
{
    if (!cart)
        cart = make_unique<Cart>();
    cart->addItem(mi, qty);
}

shared_ptr<Order> Customer::checkout(int restaurantId)
{
    if (!cart || cart->items.empty())
        return nullptr;
    auto o = make_shared<Order>();
    o->restaurantId = restaurantId;
    for (const auto &ci : cart->items)
    {
        OrderItem oi{ci.item, ci.qty, ci.item.price};
        o->items.push_back(oi);
    }
    if (o->place())
    {
        LoyaltyManager::addPoints(*this, LoyaltyManager::POINTS_PER_ORDER);
        cart->clear();
        return o;
    }
    return nullptr;
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
