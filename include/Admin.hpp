#ifndef ADMIN_HPP
#define ADMIN_HPP
#include "User.hpp"
#include <string>

class Admin : public User
{
public:
    Admin() = default;
    void disableUser(int userId);
};

#endif
