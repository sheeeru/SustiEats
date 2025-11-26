#ifndef LOYALTYMANAGER_HPP
#define LOYALTYMANAGER_HPP
class Customer; 

struct LoyaltyManager {
    static const int POINTS_PER_ORDER = 1;
    static double calculateDiscount(const Customer &c);
    static void addPoints(Customer &c, int pts);
    static void consumePoints(Customer &c, int pts);
};

#endif 
