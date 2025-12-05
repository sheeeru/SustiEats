#ifndef LOYALTYMANAGER_HPP
#define LOYALTYMANAGER_HPP

#include "Customer.hpp"
#include "Order.hpp"
#include <vector>

class LoyaltyManager {
public:
    static bool isEligibleForDiscount(const Customer &c);
    static void processCheckout(Customer &c, std::vector<Order> &orders, bool useDiscount); 
};

#endif