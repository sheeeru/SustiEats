#ifndef USER_HPP
#define USER_HPP
#include <string>
using namespace std;

class User
{
public:
    int id = 0;
    string name;
    string phone;
    string email;
    string password;
    bool isActive = true;
    User() = default;
    User(int id_, const string &n) : id(id_), name(n) {}
    virtual bool login(const string &pw) { return pw == password; }
    virtual void logout() {}
    virtual void displayDashboard() {}
    virtual ~User() = default;
};

#endif 