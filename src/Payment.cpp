#include "Payment.hpp"

bool Payment::process() {
    status = "Paid";
    return true;
}