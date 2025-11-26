#include <iostream>
#include <memory>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Customer.hpp"
#include "Owner.hpp"
#include "Restaurant.hpp"
#include "Persistence.hpp"
#include "VoiceManager.hpp"

// Simple role enum
enum class Role { Guest, CustomerRole, OwnerRole, AdminRole };

struct AppUser {
    Role role = Role::Guest;
    int userId = -1;
    std::shared_ptr<Customer> cust; // for customer
    int ownerId = -1; // for owner
};

// helper for join
static std::string joinLines(const std::vector<std::string>& lines) {
    std::ostringstream oss;
    for (const auto &l : lines) oss << l << "\n";
    return oss.str();
}

// ---------- On-screen UI helpers (modal-like) ----------

// Render a centered modal with a prompt and current input. Return input string on Enter, empty string on Esc.
static std::string showTextInput(sf::RenderWindow &window, sf::Font &font, const std::string &prompt, const std::string &initial = "") {
    std::string input = initial;
    sf::Text promptText(prompt, font, 18);
    promptText.setFillColor(sf::Color::White);

    sf::Text inputText("", font, 20);
    inputText.setFillColor(sf::Color::White);

    const float W = (float)window.getSize().x;
    const float H = (float)window.getSize().y;
    sf::RectangleShape modal(sf::Vector2f(W * 0.78f, H * 0.28f));
    modal.setFillColor(sf::Color(20,20,20,220));
    modal.setOutlineThickness(2.f);
    modal.setOutlineColor(sf::Color::White);
    modal.setPosition(W * 0.11f, H * 0.36f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return ""; }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Enter) return input;
                if (ev.key.code == sf::Keyboard::Escape) return std::string();
                if (ev.key.code == sf::Keyboard::BackSpace && !input.empty()) {
                    input.pop_back();
                }
            } else if (ev.type == sf::Event::TextEntered) {
                // accept printable characters (ignore ctrl/alt)
                if (ev.text.unicode >= 32 && ev.text.unicode < 128) {
                    input.push_back(static_cast<char>(ev.text.unicode));
                }
            }
        }

        // draw modal
        window.clear(sf::Color(28,28,28));
        // underlying UI should remain visible but dimmed — caller will redraw after returning if needed
        // draw modal contents
        promptText.setPosition(modal.getPosition().x + 12.f, modal.getPosition().y + 12.f);
        inputText.setString(input + ((int)(std::floor((sf::Clock().getElapsedTime().asMilliseconds() / 500.0))) % 2 ? "|" : ""));
        inputText.setPosition(modal.getPosition().x + 12.f, modal.getPosition().y + 46.f);

        // We'll display a faint snapshot of existing UI by not clearing everything here - but we must clear to draw modal
        window.draw(modal);
        window.draw(promptText);
        window.draw(inputText);
        window.display();

        // small sleep to avoid high CPU
        sf::sleep(sf::milliseconds(12));
    }
    return std::string();
}

// Simple informational message; waits for any key or click
static void showMessage(sf::RenderWindow &window, sf::Font &font, const std::string &msg) {
    sf::Text text(msg, font, 18);
    text.setFillColor(sf::Color::White);
    const float W = (float)window.getSize().x;
    const float H = (float)window.getSize().y;
    sf::RectangleShape modal(sf::Vector2f(W * 0.78f, H * 0.28f));
    modal.setFillColor(sf::Color(20,20,20,220));
    modal.setOutlineThickness(2.f);
    modal.setOutlineColor(sf::Color::White);
    modal.setPosition(W * 0.11f, H * 0.36f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return; }
            if (ev.type == sf::Event::KeyPressed || ev.type == sf::Event::MouseButtonPressed) return;
        }
        text.setPosition(modal.getPosition().x + 12.f, modal.getPosition().y + 12.f);
        window.draw(modal);
        window.draw(text);
        window.display();
        sf::sleep(sf::milliseconds(12));
    }
}

// Yes/No prompt: returns true for Yes, false for No or cancel (Esc).
static bool showYesNo(sf::RenderWindow &window, sf::Font &font, const std::string &prompt) {
    sf::Text promptText(prompt + "  (Y = Yes / N = No)", font, 18);
    promptText.setFillColor(sf::Color::White);
    const float W = (float)window.getSize().x;
    const float H = (float)window.getSize().y;
    sf::RectangleShape modal(sf::Vector2f(W * 0.78f, H * 0.20f));
    modal.setFillColor(sf::Color(20,20,20,220));
    modal.setOutlineThickness(2.f);
    modal.setOutlineColor(sf::Color::White);
    modal.setPosition(W * 0.11f, H * 0.40f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return false; }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Y) return true;
                if (ev.key.code == sf::Keyboard::N || ev.key.code == sf::Keyboard::Escape) return false;
            }
            if (ev.type == sf::Event::MouseButtonPressed) {
                // treat any click as cancel/no
                return false;
            }
        }
        promptText.setPosition(modal.getPosition().x + 12.f, modal.getPosition().y + 12.f);
        window.draw(modal);
        window.draw(promptText);
        window.display();
        sf::sleep(sf::milliseconds(12));
    }
    return false;
}

// ---------- Non-blocking app flows that use on-screen UI ----------

static void performOnScreenLogin(AppUser &current, const std::vector<Customer> &customers, const std::vector<Owner> &owners, VoiceManager &vm, sf::RenderWindow &window, sf::Font &font) {
    // Role prompt (single char)
    std::string r = showTextInput(window, font, "Login role (c = customer, o = owner, a = admin, q = cancel). Type letter and press Enter:");
    if (r.empty()) { showMessage(window, font, "Login cancelled."); return; }
    char roleChar = std::tolower(r[0]);

    if (roleChar == 'c') {
        std::string sid = showTextInput(window, font, "Customer ID (number):");
        if (sid.empty()) { showMessage(window, font, "Login cancelled."); return; }
        std::string pw = showTextInput(window, font, "Password:");
        if (pw.empty()) { showMessage(window, font, "Login cancelled."); return; }

        int id = -1;
        try { id = std::stoi(sid); } catch(...) { showMessage(window, font, "Invalid ID format."); vm.play("error"); return; }
        bool found = false;
        for (const auto &c : customers) {
            if (c.id == id && c.password == pw) {
                current.role = Role::CustomerRole;
                current.userId = id;
                current.cust = std::make_shared<Customer>(c);
                showMessage(window, font, "Logged in as customer: " + c.name);
                vm.play("welcome");
                found = true;
                break;
            }
        }
        if (!found) { showMessage(window, font, "Invalid customer credentials."); vm.play("error"); }
    }
    else if (roleChar == 'o') {
        std::string sid = showTextInput(window, font, "Owner ID (number):");
        if (sid.empty()) { showMessage(window, font, "Login cancelled."); return; }
        std::string pw = showTextInput(window, font, "Password:");
        if (pw.empty()) { showMessage(window, font, "Login cancelled."); return; }

        int id = -1;
        try { id = std::stoi(sid); } catch(...) { showMessage(window, font, "Invalid ID format."); vm.play("error"); return; }
        bool found = false;
        for (const auto &o : owners) {
            if (o.id == id && o.password == pw) {
                current.role = Role::OwnerRole;
                current.ownerId = id;
                showMessage(window, font, "Logged in as owner: " + o.name);
                vm.play("welcome");
                found = true;
                break;
            }
        }
        if (!found) { showMessage(window, font, "Invalid owner credentials."); vm.play("error"); }
    }
    else if (roleChar == 'a') {
        std::string sid = showTextInput(window, font, "Admin ID (number):");
        if (sid.empty()) { showMessage(window, font, "Login cancelled."); return; }
        std::string pw = showTextInput(window, font, "Password:");
        if (pw.empty()) { showMessage(window, font, "Login cancelled."); return; }

        int id = -1;
        try { id = std::stoi(sid); } catch(...) { showMessage(window, font, "Invalid ID format."); vm.play("error"); return; }
        if (id == 300 && pw == "admin") {
            current.role = Role::AdminRole;
            current.userId = id;
            showMessage(window, font, "Logged in as admin.");
            vm.play("welcome");
        } else { showMessage(window, font, "Invalid admin credentials."); vm.play("error"); }
    }
    else {
        showMessage(window, font, "No role selected.");
    }
}

// Owner edit with on-screen prompts
static void performOwnerEdit(std::vector<Restaurant> &restaurants, size_t selRestaurant, VoiceManager &vm, sf::RenderWindow &window, sf::Font &font) {
    if (selRestaurant >= restaurants.size()) { showMessage(window, font, "Invalid restaurant."); vm.play("error"); return; }
    Restaurant &r = restaurants[selRestaurant];

    std::string action = showTextInput(window, font, "Owner Edit: Type A to Add item, D to Delete item, Q to Cancel and press Enter:");
    if (action.empty()) { showMessage(window, font, "Cancelled."); return; }
    char ch = std::tolower(action[0]);
    if (ch == 'a') {
        std::string sid = showTextInput(window, font, "Enter new menu id (integer):");
        if (sid.empty()) { showMessage(window, font, "Add cancelled."); return; }
        std::string name = showTextInput(window, font, "Enter name (no | or commas):");
        if (name.empty()) { showMessage(window, font, "Add cancelled."); return; }
        std::string sprice = showTextInput(window, font, "Enter price (e.g. 120.0):");
        if (sprice.empty()) { showMessage(window, font, "Add cancelled."); return; }

        try {
            int mid = std::stoi(sid);
            double price = std::stod(sprice);
            MenuItem mi;
            mi.id = mid;
            mi.name = name;
            mi.price = price;
            mi.available = true;
            mi.category.id = 1; mi.category.name = "Mains";
            r.addMenuItem(mi);
            Persistence::saveAllRestaurants(restaurants);
            showMessage(window, font, "Added menu item and saved restaurants file.");
            vm.play("order_success");
        } catch(...) {
            showMessage(window, font, "Invalid numeric input. Aborting add."); vm.play("error");
        }
    } else if (ch == 'd') {
        std::string sid = showTextInput(window, font, "Enter menu id to remove (integer):");
        if (sid.empty()) { showMessage(window, font, "Delete cancelled."); return; }
        try {
            int mid = std::stoi(sid);
            r.removeMenuItem(mid);
            Persistence::saveAllRestaurants(restaurants);
            showMessage(window, font, "Removed (if existed) and saved restaurants file.");
            vm.play("item_removed");
        } catch(...) {
            showMessage(window, font, "Invalid id. Cancelled."); vm.play("error");
        }
    } else {
        showMessage(window, font, "Cancelled owner edit.");
    }
}

// Checkout confirmation using on-screen modal
static bool performCheckoutConfirm(std::shared_ptr<Customer> cust, std::shared_ptr<Order> &outOrder, int restaurantId, VoiceManager &vm, sf::RenderWindow &window, sf::Font &font) {
    if (!cust) { showMessage(window, font, "No customer."); vm.play("error"); return false; }
    if (!cust->cart || cust->cart->items.empty()) { showMessage(window, font, "Cart empty."); vm.play("error"); return false; }
    std::ostringstream oss;
    oss << "Cart total: " << cust->cart->getTotal() << " PKR\nConfirm checkout?";
    bool ok = showYesNo(window, font, oss.str());
    if (!ok) { showMessage(window, font, "Checkout cancelled."); return false; }
    auto order = cust->checkout(restaurantId);
    if (!order) { showMessage(window, font, "Checkout failed."); vm.play("error"); return false; }
    outOrder = order;
    std::ostringstream msg; msg << "Checkout succeeded. Order total: " << order->total;
    showMessage(window, font, msg.str());
    vm.play("order_success");
    return true;
}

// ---------- main (window-driven) ----------
int main() {
    std::cout << "SustiEats - Windowed flow demo\n";

    // load existing data
    auto restaurants = Persistence::loadAllRestaurants("data/restaurants.txt");
    auto owners = Persistence::loadAllOwners();
    auto customers = Persistence::loadAllCustomers();

    // if no owners or restaurants, create demo data (safe)
    if (owners.empty()) {
        Owner o1; o1.id = 200; o1.name = "DemoOwnerA"; o1.email="ownera@demo"; o1.password="owner";
        Owner o2; o2.id = 201; o2.name = "DemoOwnerB"; o2.email="ownerb@demo"; o2.password="ownerb";
        Persistence::saveOwner(o1); Persistence::saveOwner(o2);
        owners = Persistence::loadAllOwners();
    }
    if (restaurants.empty()) {
        Restaurant r1; r1.id=1; r1.name="Demo Deli"; r1.ownerId = 200; r1.address.line1="42 Campus Rd"; r1.address.city="Karachi"; r1.address.postalCode="75350";
        MenuItem m1; m1.id=1; m1.name="Falafel"; m1.price=120.0; m1.available=true; m1.category.id=1; m1.category.name="Mains";
        MenuItem m2; m2.id=2; m2.name="Chai"; m2.price=40.0; m2.available=true; m2.category.id=2; m2.category.name="Drinks";
        r1.addMenuItem(m1); r1.addMenuItem(m2);
        Restaurant r2; r2.id=2; r2.name="Campus Grill"; r2.ownerId=201; r2.address.line1="10 Student Ln"; r2.address.city="Karachi"; r2.address.postalCode="75351";
        MenuItem m3; m3.id=3; m3.name="Wrap"; m3.price=220.0; m3.available=true; m3.category.id=1; m3.category.name="Mains";
        r2.addMenuItem(m3);
        restaurants.push_back(r1); restaurants.push_back(r2);
        Persistence::saveAllRestaurants(restaurants);
    }
    if (customers.empty()) {
        Customer c; c.id=100; c.name="Shaheer Q"; c.email="shaheer@example.com"; c.password="pass"; c.phone="0300-0000000";
        Persistence::saveCustomer(c);
        customers = Persistence::loadAllCustomers();
    }

    // create a live customer shared_ptr preloaded from file (if exists)
    std::shared_ptr<Customer> liveCustomer = nullptr;
    if (!customers.empty()) liveCustomer = std::make_shared<Customer>(customers.front());

    // voice manager
    VoiceManager vm;
    vm.loadVoice("welcome", "assets/audio/voice/welcome.ogg");
    vm.loadVoice("item_added", "assets/audio/voice/item_added.ogg");
    vm.loadVoice("item_removed", "assets/audio/voice/item_removed.ogg");
    vm.loadVoice("loyalty", "assets/audio/voice/loyalty.ogg");
    vm.loadVoice("no_loyalty", "assets/audio/voice/no_loyalty.ogg");
    vm.loadVoice("order_success", "assets/audio/voice/order_success.ogg");
    vm.loadVoice("payment_success", "assets/audio/voice/payment_success.ogg");
    vm.loadVoice("payment_fail", "assets/audio/voice/payment_fail.ogg");
    vm.loadVoice("error", "assets/audio/voice/error.ogg");
    vm.play("welcome");

    // SFML window
    const unsigned int WIN_W = 1000, WIN_H = 640;
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "SustiEats Sequential Demo");
    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) std::cerr << "Missing font\n";
    sf::Text header("SustiEats - Press L to login - 1 Home, 2 Restaurants", font, 20);
    header.setPosition(12.f,12.f); header.setFillColor(sf::Color::White);
    sf::Text body("", font, 18); body.setPosition(12.f,56.f); body.setFillColor(sf::Color::White);

    sf::RectangleShape sidePanel(sf::Vector2f(320.f, WIN_H - 24.f));
    sidePanel.setPosition(WIN_W - 340.f, 12.f); sidePanel.setFillColor(sf::Color(18,18,18,220));
    sf::Text controls("", font, 16); controls.setPosition(sidePanel.getPosition().x + 10.f, sidePanel.getPosition().y + 12.f); controls.setFillColor(sf::Color::White);

    AppUser current;
    int screen = 1; // 1 home, 2 list, 3 detail, 4 cart
    size_t selRestaurant = 0;
    size_t selMenuItem = 0;

    auto makeControls = [&]() -> std::vector<std::string> {
    std::vector<std::string> out;
    out.push_back("Esc: Quit");
    out.push_back("L: Login (on-screen)");
    out.push_back("O: Logout");
    out.push_back("1: Home");
    out.push_back("2: Restaurants List");
    if (screen == 2) out.push_back("Press number key (1..9) to open that restaurant");
    // show these controls for both restaurant detail (3) and cart view (4)
    if (screen == 3 || screen == 4) {
        out.push_back("Left/Right: select menu item");
        out.push_back("A: Add to cart (customer)");
        out.push_back("R: Remove from cart (customer)");
        out.push_back("V: View cart");
        out.push_back("C: Checkout (customer)");
        out.push_back("P: Pay (customer)");
        out.push_back("U: Owner Edit (owner)");
    }
    out.push_back("9: Play welcome voice");
    return out;
    };


    auto updateControls = [&]() {
        controls.setString(joinLines(makeControls()));
    };
    updateControls();

    auto restaurantListString = [&]() {
        std::ostringstream oss;
        oss << "Restaurants (" << restaurants.size() << "):\n";
        for (size_t i = 0; i < restaurants.size(); ++i) {
            oss << (i+1) << ". " << restaurants[i].name << " (Owner: " << restaurants[i].ownerId << ")\n";
        }
        oss << "\nSelect by pressing its number.\n";
        return oss.str();
    };

    auto restaurantDetailString = [&](size_t ridx) {
        std::ostringstream oss;
        if (ridx >= restaurants.size()) return std::string("Invalid restaurant\n");
        const auto &r = restaurants[ridx];
        oss << "Restaurant: " << r.name << " (id=" << r.id << ", owner=" << r.ownerId << ")\n";
        oss << r.address.line1 << ", " << r.address.city << "\n\n";
        oss << "Menu:\n";
        for (size_t i = 0; i < r.menu.size(); ++i) {
            if (i == selMenuItem) oss << " > ";
            else oss << "   ";
            oss << r.menu[i].name << " (" << r.menu[i].price << " PKR)";
            if (!r.menu[i].available) oss << " [X]";
            oss << "\n";
        }
        return oss.str();
    };

    // Use liveCustomer pointer when logged-in as customer
    if (liveCustomer) { current.cust = liveCustomer; }

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); break; }
            if (ev.type == sf::Event::KeyPressed) {
                auto kc = ev.key.code;
                if (kc == sf::Keyboard::Escape) { window.close(); break; }
                if (kc == sf::Keyboard::L) {
                    // on-screen login
                    performOnScreenLogin(current, customers, owners, vm, window, font);
                    // if logged in as customer, ensure current.cust is live and persists loyalty/cart
                    if (current.role == Role::CustomerRole && !current.cust) {
                        // try to find customer data and create shared_ptr
                        for (const auto &c : customers) if (c.id == current.userId) { current.cust = std::make_shared<Customer>(c); break; }
                    }
                    updateControls();
                }
                else if (kc == sf::Keyboard::O) {
                    // logout
                    current = AppUser();
                    updateControls();
                    vm.play("welcome");
                }
                else if (kc == sf::Keyboard::Num1) { screen = 1; updateControls(); }
                else if (kc == sf::Keyboard::Num2) { screen = 2; updateControls(); }
                else if (screen == 2 && kc >= sf::Keyboard::Num1 && kc <= sf::Keyboard::Num9) {
                    int idx = (int)kc - (int)sf::Keyboard::Num1;
                    if (idx >= 0 && (size_t)idx < restaurants.size()) {
                        selRestaurant = idx;
                        selMenuItem = 0;
                        screen = 3;
                        updateControls();
                    }
                }
                else if (screen == 3 && kc == sf::Keyboard::Left) {
                    if (selMenuItem > 0) selMenuItem--;
                }
                else if (screen == 3 && kc == sf::Keyboard::Right) {
                    if (selMenuItem + 1 < restaurants[selRestaurant].menu.size()) selMenuItem++;
                }
                else if (screen == 3 && kc == sf::Keyboard::A) {
                    // add to cart: require customer
                    if (current.role != Role::CustomerRole || !current.cust) {
                        showMessage(window, font, "You must login as customer to add to cart. Press L to login.");
                        vm.play("error");
                    } else {
                        const auto &mi = restaurants[selRestaurant].menu[selMenuItem];
                        current.cust->addToCart(mi, 1);
                        showMessage(window, font, std::string("Added to cart: ") + mi.name);
                        Persistence::saveCustomer(*current.cust); // append state
                        vm.play("item_added");
                    }
                }
                else if (screen == 3 && kc == sf::Keyboard::R) {
                    if (current.role != Role::CustomerRole || !current.cust) {
                        showMessage(window, font, "Login as customer to remove from cart."); vm.play("error");
                    } else {
                        int menuId = restaurants[selRestaurant].menu[selMenuItem].id;
                        if (current.cust->cart) current.cust->cart->removeItem(menuId);
                        showMessage(window, font, std::string("Remove command executed for menuId ") + std::to_string(menuId));
                        vm.play("item_removed");
                    }
                }
                else if (screen == 3 && kc == sf::Keyboard::V) {
                    if (current.role == Role::CustomerRole && current.cust) screen = 4;
                    else { showMessage(window, font, "Login as customer to view cart."); vm.play("error"); }
                    updateControls();
                }
                else if (screen == 3 && kc == sf::Keyboard::U) {
                    // owner edit (on-screen)
                    if (current.role == Role::OwnerRole && current.ownerId == restaurants[selRestaurant].ownerId) {
                        performOwnerEdit(restaurants, selRestaurant, vm, window, font);
                    } else {
                        showMessage(window, font, "Not owner of this restaurant."); vm.play("error");
                    }
                }
                else if ((screen == 3 || screen == 4) && kc == sf::Keyboard::C) {
                    // checkout only if customer
                    if (current.role != Role::CustomerRole || !current.cust) { showMessage(window, font, "Login as customer to checkout."); vm.play("error"); }
                    else {
                        std::shared_ptr<Order> outOrder;
                        if (performCheckoutConfirm(current.cust, outOrder, restaurants[selRestaurant].id, vm, window, font)) {
                            // persist order
                            static int nextOrderId = 500;
                            outOrder->id = nextOrderId++;
                            outOrder->customerId = current.cust->id;
                            Persistence::saveOrder(*outOrder);
                        }
                    }
                }
                else if ((screen == 3 || screen == 4) && kc == sf::Keyboard::P) {
                    if (current.role != Role::CustomerRole || !current.cust) { showMessage(window, font, "Login as customer to pay."); vm.play("error"); }
                    else {
                        bool pay = showYesNo(window, font, "Confirm payment now?");
                        if (pay) { showMessage(window, font, "Payment succeeded (demo)."); vm.play("payment_success"); }
                        else { showMessage(window, font, "Payment cancelled."); vm.play("payment_fail"); }
                    }
                }
                else if (kc == sf::Keyboard::Num9) vm.play("welcome");
            } // key pressed
        } // pollEvent

        // render
        window.clear(sf::Color(28,28,28));
        window.draw(header);

        std::ostringstream oss;
        if (screen == 1) {
            oss << "Home - Welcome to SustiEats demo.\n";
            oss << "Press 2 to list restaurants.\n";
        } else if (screen == 2) {
            oss << restaurantListString();
        } else if (screen == 3) {
            oss << restaurantDetailString(selRestaurant);
            if (current.role == Role::OwnerRole && current.ownerId == restaurants[selRestaurant].ownerId) oss << "\nYou are the owner. Press U to edit.\n";
        } else if (screen == 4) {
            // cart view
            if (current.cust && current.cust->cart) {
                oss << "Cart contents:\n";
                for (const auto &ci : current.cust->cart->items) {
                    oss << "- " << ci.item.name << " x" << ci.qty << " = " << ci.subtotal() << "\n";
                }
                oss << "Total: " << current.cust->cart->getTotal() << " PKR\n";
            } else oss << "No cart.\n";
        }

        body.setString(oss.str());
        window.draw(body);

        updateControls();
        window.draw(sidePanel);
        window.draw(controls);

        window.display();
        sf::sleep(sf::milliseconds(12));
    } // main while

    vm.stopAll();
    std::cout << "Exiting.\n";
    return 0;
}
