#include "LoyaltyManager.hpp"
#include "Persistence.hpp"
#include <iostream>

using namespace std;

bool LoyaltyManager::isEligibleForDiscount(const Customer &c) {
    return c.loyaltyPoints >= 1000;
}

void LoyaltyManager::processCheckout(Customer &c, vector<Order> &orders, bool useDiscount) {
    
    // Step 1: Get a unique Order ID for this entire transaction
    // We use the same ID for all restaurants in this cart
    int sharedOrderId = Persistence::getNextId("orders.txt");

    // Step 2: Handle Discount Logic
    bool discountApplied = false;
    if (useDiscount && isEligibleForDiscount(c)) {
        c.loyaltyPoints -= 1000; // Deduct points
        discountApplied = true;
    }

    // Step 3: Process each order (one order per restaurant involved)
    for (auto &ord : orders) {
        // A. Assign the Shared ID
        ord.id = sharedOrderId;
        ord.customerId = c.id;
        ord.status = "Placed"; // Default status

        // B. Calculate Total & Apply Discount if needed
        double orderTotal = 0.0;
        for (auto &item : ord.items) {
            // Recalculate item total just to be safe
            double itemCost = item.unitPrice * item.qty;
            orderTotal += itemCost;
        }

        if (discountApplied) {
            // Apply 10% off (multiply by 0.90)
            ord.total = orderTotal * 0.90;
        } else {
            ord.total = orderTotal;
        }

        // C. Save the Order to file
        Persistence::saveOrder(ord);
    }

    // Step 4: Reward the customer
    // We give +10 points for the transaction (regardless of size)
    c.loyaltyPoints += 10;

    // Step 5: Save the updated Customer data (Points changed)
    // We need to update the specific customer in the full list
    vector<Customer> allCustomers = Persistence::loadAllCustomers("customers.txt");
    
    for (auto &dbCust : allCustomers) {
        if (dbCust.id == c.id) {
            dbCust.loyaltyPoints = c.loyaltyPoints; // Update points
            break;
        }
    }
    
    // Save everyone back to file
    Persistence::saveAllCustomers(allCustomers, "customers.txt");
}