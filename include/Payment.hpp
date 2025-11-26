#ifndef PAYMENT_HPP
#define PAYMENT_HPP
#include <string>
using namespace std;

class Payment
{
public:
    int id = 0;
    int orderId = -1;
    string method = "Cash";
    string status = "NotPaid";
    bool process();
};

#endif 
