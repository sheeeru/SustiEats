#include "LoyaltyManager.hpp"
#include "Customer.hpp"
using namespace std;

double LoyaltyManager::calculateDiscount(const Customer &c)
{
    if (c.loyaltyPoints >= 20)
        return 0.20;
    if (c.loyaltyPoints >= 10)
        return 0.10;
    if (c.loyaltyPoints >= 5)
        return 0.05;
    return 0.0;
}

void LoyaltyManager::addPoints(Customer &c, int pts) { c.loyaltyPoints += pts; }
void LoyaltyManager::consumePoints(Customer &c, int pts)
{
    if (c.loyaltyPoints >= pts)
        c.loyaltyPoints -= pts;
}