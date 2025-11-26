#include "Persistence.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
using namespace std;

string Persistence::dataFolder = "data/";

void Persistence::ensureDataFolderExists()
{
    filesystem::create_directories(dataFolder);
}

// ----------------- Customers -----------------
void Persistence::saveCustomer(const Customer &c, const string &filename)
{
    ensureDataFolderExists();
    ofstream ofs(dataFolder + filename, ios::app);
    if (!ofs)
        return;
    // store: id|name|email|phone|password|isActive|loyaltyPoints
    ofs << c.id << "|" << c.name << "|" << c.email << "|" << c.phone << "|" << c.password << "|" << (c.isActive ? 1 : 0) << "|" << c.loyaltyPoints << "\n";
}

vector<Customer> Persistence::loadAllCustomers(const string &filename)
{
    ensureDataFolderExists();
    vector<Customer> out;
    ifstream ifs(dataFolder + filename);
    if (!ifs)
        return out;
    string line;
    while (getline(ifs, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string token;
        Customer c;
        getline(iss, token, '|');
        c.id = stoi(token);
        getline(iss, c.name, '|');
        getline(iss, c.email, '|');
        getline(iss, c.phone, '|');
        getline(iss, c.password, '|');
        getline(iss, token, '|');
        c.isActive = (token == "1");
        getline(iss, token, '|');
        c.loyaltyPoints = stoi(token);
        out.push_back(c);
    }
    return out;
}

// ----------------- Owners -----------------
void Persistence::saveOwner(const Owner &o, const string &filename)
{
    ensureDataFolderExists();
    ofstream ofs(dataFolder + filename, ios::app);
    if (!ofs)
        return;
    // store: id|name|email|phone|password|isActive
    ofs << o.id << "|" << o.name << "|" << o.email << "|" << o.phone << "|" << o.password << "|" << (o.isActive ? 1 : 0) << "\n";
}

vector<Owner> Persistence::loadAllOwners(const string &filename)
{
    ensureDataFolderExists();
    vector<Owner> out;
    ifstream ifs(dataFolder + filename);
    if (!ifs)
        return out;
    string line;
    while (getline(ifs, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string token;
        Owner o;
        getline(iss, token, '|');
        o.id = stoi(token);
        getline(iss, o.name, '|');
        getline(iss, o.email, '|');
        getline(iss, o.phone, '|');
        getline(iss, o.password, '|');
        getline(iss, token, '|');
        o.isActive = (token == "1");
        out.push_back(o);
    }
    return out;
}

// ----------------- Restaurants -----------------
// serialization format:
// id|name|line1|city|postal|ownerId|menu_count|mi_token|mi_token|...
// where mi_token is id,name,catid,catname,price,avail  (comma separated)
void Persistence::saveRestaurant(const Restaurant &r, const string &filename)
{
    ensureDataFolderExists();
    ofstream ofs(dataFolder + filename, ios::app);
    if (!ofs)
        return;
    ostringstream oss;
    oss << r.id << "|" << r.name << "|" << r.address.line1 << "|" << r.address.city << "|" << r.address.postalCode << "|" << r.ownerId << "|" << r.menu.size();
    for (const auto &mi : r.menu)
    {
        // escape commas in name? (simple approach: replace '|' not allowed; we assume names avoid '|')
        oss << "|" << mi.id << "," << mi.name << "," << mi.category.id << "," << mi.category.name << "," << mi.price << "," << (mi.available ? 1 : 0);
    }
    ofs << oss.str() << "\n";
}

// Load all restaurants (returns vector)
vector<Restaurant> Persistence::loadAllRestaurants(const string &filename)
{
    ensureDataFolderExists();
    vector<Restaurant> out;
    ifstream ifs(dataFolder + filename);
    if (!ifs)
        return out;
    string line;
    while (getline(ifs, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string token;
        Restaurant r;
        getline(iss, token, '|');
        r.id = stoi(token);
        getline(iss, r.name, '|');
        getline(iss, r.address.line1, '|');
        getline(iss, r.address.city, '|');
        getline(iss, r.address.postalCode, '|');
        getline(iss, token, '|');
        r.ownerId = stoi(token);
        getline(iss, token, '|');
        int menuCount = stoi(token);
        for (int i = 0; i < menuCount; ++i)
        {
            getline(iss, token, '|'); // token format: id,name,catid,catname,price,avail
            istringstream mit(token);
            string field;
            MenuItem mi;
            getline(mit, field, ',');
            mi.id = stoi(field);
            getline(mit, mi.name, ',');
            getline(mit, field, ',');
            mi.category.id = stoi(field);
            getline(mit, mi.category.name, ',');
            getline(mit, field, ',');
            mi.price = stod(field);
            getline(mit, field, ',');
            mi.available = (field == "1");
            r.menu.push_back(mi);
        }
        out.push_back(r);
    }
    return out;
}

// overwrite whole restaurants file
void Persistence::saveAllRestaurants(const vector<Restaurant> &restaurants, const string &filename)
{
    ensureDataFolderExists();
    ofstream ofs(dataFolder + filename, ios::trunc);
    if (!ofs)
        return;
    for (const auto &r : restaurants)
    {
        ostringstream oss;
        oss << r.id << "|" << r.name << "|" << r.address.line1 << "|" << r.address.city << "|" << r.address.postalCode << "|" << r.ownerId << "|" << r.menu.size();
        for (const auto &mi : r.menu)
        {
            oss << "|" << mi.id << "," << mi.name << "," << mi.category.id << "," << mi.category.name << "," << mi.price << "," << (mi.available ? 1 : 0);
        }
        ofs << oss.str() << "\n";
    }
}

// ----------------- Orders -----------------
void Persistence::saveOrder(const Order &o, const string &filename)
{
    ensureDataFolderExists();
    ofstream ofs(dataFolder + filename, ios::app);
    if (!ofs)
        return;
    ostringstream oss;
    oss << o.id << "|" << o.customerId << "|" << o.restaurantId << "|" << o.status << "|" << o.total << "|" << o.items.size();
    for (const auto &it : o.items)
    {
        oss << "|" << it.itemSnapshot.id << "," << it.itemSnapshot.name << "," << it.qty << "," << it.unitPrice;
    }
    ofs << oss.str() << "\n";
}

vector<Order> Persistence::loadAllOrders(const string &filename)
{
    ensureDataFolderExists();
    vector<Order> out;
    ifstream ifs(dataFolder + filename);
    if (!ifs)
        return out;
    string line;
    while (getline(ifs, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string token;
        Order o;
        getline(iss, token, '|');
        o.id = stoi(token);
        getline(iss, token, '|');
        o.customerId = stoi(token);
        getline(iss, token, '|');
        o.restaurantId = stoi(token);
        getline(iss, o.status, '|');
        getline(iss, token, '|');
        o.total = stod(token);
        getline(iss, token, '|');
        int itemCount = stoi(token);
        for (int i = 0; i < itemCount; ++i)
        {
            getline(iss, token, '|');
            istringstream mit(token);
            string field;
            OrderItem it;
            getline(mit, field, ',');
            it.itemSnapshot.id = stoi(field);
            getline(mit, it.itemSnapshot.name, ',');
            getline(mit, field, ',');
            it.qty = stoi(field);
            getline(mit, field, ',');
            it.unitPrice = stod(field);
            o.items.push_back(it);
        }
        out.push_back(o);
    }
    return out;
}
