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
#include "LoyaltyManager.hpp" 

// --- THEME COLORS ---
const sf::Color COL_BG(30, 32, 36);          
const sf::Color COL_PANEL(40, 44, 52);       
const sf::Color COL_HEADER(230, 81, 0);      
const sf::Color COL_TEXT_MAIN(236, 240, 241);
const sf::Color COL_TEXT_SEC(149, 165, 166);
const sf::Color COL_ACCENT(241, 196, 15);    
const sf::Color COL_CARD(50, 55, 65);        
const sf::Color COL_BTN_GREEN(39, 174, 96);
const sf::Color COL_BTN_RED(192, 57, 43);
const sf::Color COL_BTN_HOVER(255, 255, 255, 50); 

enum class Role { Guest, CustomerRole, OwnerRole, AdminRole };

struct AppUser {
    Role role = Role::Guest;
    int userId = -1;
    std::shared_ptr<Customer> cust;
    int ownerId = -1;
};

static std::string joinLines(const std::vector<std::string>& lines) {
    std::ostringstream oss;
    for (const auto &l : lines) oss << l << "\n";
    return oss.str();
}

// ---------- UI Helpers ----------

static std::string showTextInput(sf::RenderWindow &window, sf::Font &font, const std::string &prompt, const std::string &initial = "") {
    std::string input = initial;
    sf::Text promptText(prompt, font, 20); 
    promptText.setFillColor(COL_ACCENT);
    sf::Text inputText("", font, 24);
    inputText.setFillColor(sf::Color::White);
    
    sf::RectangleShape modal(sf::Vector2f(window.getSize().x * 0.6f, 200.f));
    modal.setFillColor(sf::Color(40, 40, 40, 245));
    modal.setOutlineThickness(2.f);
    modal.setOutlineColor(COL_HEADER);
    modal.setPosition(window.getSize().x * 0.2f, window.getSize().y * 0.35f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return ""; }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Enter) return input;
                if (ev.key.code == sf::Keyboard::Escape) return "";
                if (ev.key.code == sf::Keyboard::BackSpace && !input.empty()) input.pop_back();
            } else if (ev.type == sf::Event::TextEntered) {
                if (ev.text.unicode >= 32 && ev.text.unicode < 128) input.push_back(static_cast<char>(ev.text.unicode));
            }
        }
        window.clear(COL_BG); 
        
        promptText.setPosition(modal.getPosition().x + 20.f, modal.getPosition().y + 20.f);
        inputText.setString(input + ((int)(std::floor((sf::Clock().getElapsedTime().asMilliseconds() / 500.0))) % 2 ? "|" : ""));
        inputText.setPosition(modal.getPosition().x + 20.f, modal.getPosition().y + 80.f);
        
        window.draw(modal);
        window.draw(promptText);
        window.draw(inputText);
        window.display();
        sf::sleep(sf::milliseconds(12));
    }
    return "";
}

static void showMessage(sf::RenderWindow &window, sf::Font &font, const std::string &msg) {
    sf::Text text(msg, font, 20);
    text.setFillColor(sf::Color::White);
    sf::RectangleShape modal(sf::Vector2f(window.getSize().x * 0.6f, 150.f));
    modal.setFillColor(sf::Color(40, 40, 40, 245));
    modal.setOutlineThickness(2.f);
    modal.setOutlineColor(COL_ACCENT);
    modal.setPosition(window.getSize().x * 0.2f, window.getSize().y * 0.4f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return; }
            if (ev.type == sf::Event::KeyPressed || ev.type == sf::Event::MouseButtonPressed) return;
        }
        text.setPosition(modal.getPosition().x + 20.f, modal.getPosition().y + 50.f);
        window.draw(modal);
        window.draw(text);
        window.display();
        sf::sleep(sf::milliseconds(12));
    }
}

static bool showYesNo(sf::RenderWindow &window, sf::Font &font, const std::string &prompt) {
    sf::Text promptText(prompt + "\n\n(Press Y for Yes, N for No)", font, 20);
    promptText.setFillColor(sf::Color::White);
    sf::RectangleShape modal(sf::Vector2f(window.getSize().x * 0.6f, 200.f));
    modal.setFillColor(sf::Color(40, 40, 40, 245));
    modal.setOutlineThickness(2.f);
    modal.setOutlineColor(COL_ACCENT);
    modal.setPosition(window.getSize().x * 0.2f, window.getSize().y * 0.35f);

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); return false; }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Y) return true;
                if (ev.key.code == sf::Keyboard::N || ev.key.code == sf::Keyboard::Escape) return false;
            }
            if (ev.type == sf::Event::MouseButtonPressed) return false;
        }
        promptText.setPosition(modal.getPosition().x + 20.f, modal.getPosition().y + 20.f);
        window.draw(modal);
        window.draw(promptText);
        window.display();
        sf::sleep(sf::milliseconds(12));
    }
    return false;
}

// ---------- Logic Helpers ----------

static void performOnScreenLogin(AppUser &current, const std::vector<Customer> &customers, const std::vector<Owner> &owners, VoiceManager &vm, sf::RenderWindow &window, sf::Font &font) {
    std::string r = showTextInput(window, font, "Login role (c=cust, o=owner, a=admin).");
    if (r.empty()) return;
    char roleChar = std::tolower(r[0]);

    if (roleChar == 'c') {
        std::string sid = showTextInput(window, font, "Customer ID:");
        std::string pw = showTextInput(window, font, "Password:");
        if (sid.empty() || pw.empty()) return;
        int id = -1;
        try { id = std::stoi(sid); } catch(...) { showMessage(window, font, "Invalid ID"); return; }
        for (const auto &c : customers) {
            if (c.id == id && c.password == pw) {
                if (!c.isActive) {
                    vm.play("error");
                    showMessage(window, font, "Account Disabled by Admin.");
                    return;
                }
                current.role = Role::CustomerRole; current.userId = id;
                current.cust = std::make_shared<Customer>(c);
                showMessage(window, font, "Welcome " + c.name); vm.play("welcome");
                return;
            }
        }
        vm.play("error");
        showMessage(window, font, "Invalid credentials."); 
    }
    else if (roleChar == 'o') {
        std::string sid = showTextInput(window, font, "Owner ID:");
        std::string pw = showTextInput(window, font, "Password:");
        if (sid.empty() || pw.empty()) return;
        int id = -1;
        try { id = std::stoi(sid); } catch(...) { showMessage(window, font, "Invalid ID"); return; }
        for (const auto &o : owners) {
            if (o.id == id && o.password == pw) {
                if (!o.isActive) {
                    vm.play("error");
                    showMessage(window, font, "Account Disabled by Admin.");
                    return;
                }
                current.role = Role::OwnerRole; current.ownerId = id;
                showMessage(window, font, "Welcome Owner " + o.name); vm.play("welcome");
                return;
            }
        }
        vm.play("error");
        showMessage(window, font, "Invalid credentials."); 
    }
    else if (roleChar == 'a') {
        std::string sid = showTextInput(window, font, "Admin ID:");
        std::string pw = showTextInput(window, font, "Password:");
        if (sid.empty() || pw.empty()) return;
        int id = -1;
        try { id = std::stoi(sid); } catch(...) { showMessage(window, font, "Invalid ID"); return; }
        
        if (Persistence::verifyAdmin(id, pw)) {
            current.role = Role::AdminRole; current.userId = id;
            showMessage(window, font, "Welcome Admin"); vm.play("welcome");
        } else { 
            vm.play("error"); showMessage(window, font, "Invalid admin credentials.");  
        }
    }
}

static void performOwnerEdit(int ownerId, std::vector<Restaurant> &restaurants, size_t selRestaurant, VoiceManager &vm, sf::RenderWindow &window, sf::Font &font) {
    if (selRestaurant >= restaurants.size()) return;
    Restaurant &r = restaurants[selRestaurant];

    if (r.ownerId != ownerId) {
        vm.play("error");
        showMessage(window, font, "Permission Denied.\nYou do not own this restaurant.");
        return;
    }

    std::string action = showTextInput(window, font, "Owner Edit: (A)dd item, (D)elete item?");
    if (action.empty()) return;
    char ch = std::tolower(action[0]);
    if (ch == 'a') {
        std::string sid = showTextInput(window, font, "New ID:");
        std::string name = showTextInput(window, font, "Name:");
        std::string sprice = showTextInput(window, font, "Price:");
        try {
            MenuItem mi; mi.id = std::stoi(sid); mi.name = name; mi.price = std::stod(sprice);
            mi.available = true;
            r.addMenuItem(mi);
            Persistence::saveAllRestaurants(restaurants);
            vm.play("order_success");
            showMessage(window, font, "Item Added."); 
        } catch(...) { showMessage(window, font, "Invalid input."); }
    } else if (ch == 'd') {
        std::string sid = showTextInput(window, font, "ID to remove:");
        try {
            r.removeMenuItem(std::stoi(sid));
            Persistence::saveAllRestaurants(restaurants);
            
            showMessage(window, font, "Item Removed."); 
        } catch(...) { showMessage(window, font, "Invalid input."); }
    }
}

static bool performCheckoutConfirm(std::shared_ptr<Customer> cust, VoiceManager &vm, sf::RenderWindow &window, sf::Font &font) {
    if (!cust || !cust->cart || cust->cart->items.empty()) { showMessage(window, font, "Cart empty."); return false; }
    
    std::ostringstream oss; oss << "Total: " << cust->cart->getTotal() << " PKR\nConfirm?";
    if (!showYesNo(window, font, oss.str())) return false;
    
    bool useDiscount = false;
    if (cust->loyaltyPoints >= 1000) {
        if(showYesNo(window, font, "Use 1000 points for 10% off?")) { useDiscount = true; vm.play("loyalty"); }
    }

    auto outOrders = cust->checkout();
    if (outOrders.empty()) return false;
    
    std::vector<Order> orderValues;
    for (auto &ptr : outOrders) orderValues.push_back(*ptr);
    LoyaltyManager::processCheckout(*cust, orderValues, useDiscount);
    vm.play("order_success");
    showMessage(window, font, "Checkout Success!\n Loyalty Points: +10 Points.");

    return true;
}

// ---------- Main Loop ----------
int main() {
    auto restaurants = Persistence::loadAllRestaurants();
    auto owners = Persistence::loadAllOwners();
    auto customers = Persistence::loadAllCustomers();
    auto allOrders = Persistence::loadAllOrders();

    // AUTO-REPAIR Logic: If empty or corrupted, create defaults and overwrite file.
    if (owners.empty()) {
        Owner o1; o1.id=200; o1.name="DemoOwnerA"; o1.email="a@d"; o1.password="owner";
        Owner o2; o2.id=201; o2.name="DemoOwnerB"; o2.email="b@d"; o2.password="ownerb";
        std::vector<Owner> initOwners = {o1, o2};
        Persistence::saveAllOwners(initOwners); // Overwrite corrupt file
        owners = Persistence::loadAllOwners();
    } else {
        // Force save to ensure clean format if it loaded but had skipped lines
        Persistence::saveAllOwners(owners);
    }

    if (restaurants.empty()) {
        Restaurant r1; r1.id=1; r1.name="Demo Deli"; r1.ownerId=200; r1.address.line1="Loc1";
        MenuItem mi1; mi1.id = 1; mi1.name = "Falafel"; mi1.price = 120.0; mi1.available = true; 
        r1.addMenuItem(mi1);
        
        Restaurant r2; r2.id=2; r2.name="Campus Grill"; r2.ownerId=201; r2.address.line1="Loc2";
        MenuItem mi2; mi2.id = 2; mi2.name = "Burger"; mi2.price = 250.0; mi2.available = true;
        r2.addMenuItem(mi2);

        restaurants.push_back(r1); 
        restaurants.push_back(r2);
        Persistence::saveAllRestaurants(restaurants); // Overwrite corrupt file
    }

    if (customers.empty()) {
        Customer c; c.id=100; c.name="Shaheer"; c.password="pass"; 
        Persistence::saveCustomer(c); customers = Persistence::loadAllCustomers();
    }

    std::shared_ptr<Customer> liveCustomer = nullptr;
    if (!customers.empty()) liveCustomer = std::make_shared<Customer>(customers.front());

    VoiceManager vm;
    vm.loadVoice("welcome", "assets/audio/voice/welcome.ogg");
    vm.loadVoice("item_added", "assets/audio/voice/item_added.ogg");
    vm.loadVoice("item_removed", "assets/audio/voice/item_removed.ogg");
    vm.loadVoice("loyalty", "assets/audio/voice/loyalty.ogg");
    vm.loadVoice("order_success", "assets/audio/voice/order_success.ogg");
    vm.loadVoice("error", "assets/audio/voice/error.ogg");
    vm.loadVoice("order_dispatched", "assets/audio/voice/order_dispatched.ogg");
    vm.loadVoice("order_cancel", "assets/audio/voice/order_cancel.ogg");
    vm.play("welcome");

    sf::RenderWindow window(sf::VideoMode(1000, 640), "SustiEats Interactive");
    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) return -1;

    sf::RectangleShape headerRect(sf::Vector2f(1000.f, 60.f));
    headerRect.setFillColor(COL_HEADER);

    sf::Text header("SustiEats - Order Smart, Eat Well.", font, 24);
    header.setPosition(20.f, 15.f);
    header.setFillColor(sf::Color::White);
    
    sf::View contentView;
    float contentWidth = 660.f; float contentHeight = 580.f; 
    contentView.setViewport(sf::FloatRect(0.f, 60.f/640.f, (1000.f - 300.f)/1000.f, 580.f/640.f));
    contentView.setSize(contentWidth, contentHeight);
    contentView.setCenter(contentWidth/2.f, contentHeight/2.f);
    float currentScrollY = 0.0f;

    sf::Text body("", font, 20); 
    body.setPosition(20.f, 0.f); 
    body.setFillColor(COL_TEXT_MAIN);
    body.setLineSpacing(1.2f);

    sf::RectangleShape sidePanel(sf::Vector2f(300.f, 640.f)); 
    sidePanel.setPosition(1000.f - 300.f, 60.f); 
    sidePanel.setFillColor(COL_PANEL);
    
    sf::Text controls("", font, 18); 
    controls.setPosition(sidePanel.getPosition().x + 20.f, sidePanel.getPosition().y + 20.f); 
    controls.setFillColor(COL_TEXT_SEC);

    AppUser current;
    if (liveCustomer) current.cust = liveCustomer;

    int screen = 1; 
    size_t selRestaurant = 0; 
    size_t selMenuItem = 0;
    
    auto restaurantListString = [&]() {
        std::ostringstream oss;
        oss << "Select a Restaurant:\n\n";
        for (size_t i = 0; i < restaurants.size(); ++i) {
            oss << "[" << (i+1) << "] " << restaurants[i].name << "\n";
            oss << "    " << restaurants[i].address.line1 << "\n\n";
        }
        return oss.str();
    };

    auto restaurantDetailString = [&](size_t ridx) {
        std::ostringstream oss;
        if (ridx >= restaurants.size()) return std::string("Invalid restaurant\n");
        const auto &r = restaurants[ridx];
        oss << ">> " << r.name << " <<\n";
        oss << r.address.line1 << "\n\nMENU:\n";
        for (size_t i = 0; i < r.menu.size(); ++i) {
            if (i == selMenuItem) oss << " -> ";
            else oss << "    ";
            oss << r.menu[i].name << " ................. " << r.menu[i].price << " PKR\n";
        }
        return oss.str();
    };

    auto customerDashboardString = [&]() {
        if (current.role != Role::CustomerRole) return std::string("Not logged in as customer.");
        allOrders = Persistence::loadAllOrders(); // refresh
        std::ostringstream oss;
        oss << "Welcome back, " << current.cust->name << "!\n";
        oss << "Loyalty Points: " << current.cust->loyaltyPoints << "\n\nYOUR ORDER HISTORY:\n";
        int myOrdersCount = 0;
        for (const auto &o : allOrders) {
            if (o.customerId == current.userId) {
                oss << "Order #" << o.id << "  [" << o.status << "]  Total: " << o.total << "\n";
                myOrdersCount++;
            }
        }
        if (myOrdersCount == 0) oss << "No previous orders found.";
        return oss.str();
    };

    auto makeControls = [&]() -> std::vector<std::string> {
        std::vector<std::string> out;
        out.push_back("ESC : Quit App");
        out.push_back("");

        if (current.role == Role::Guest) {
            out.push_back("L : Login");
            out.push_back("1 : Home Screen");
            out.push_back("2 : Browse Restaurants");
            if (screen == 2) out.push_back("\n(Type Number to Select)");
            if (screen == 3) out.push_back("\nARROWS : Scroll Menu");
        }
        else if (current.role == Role::CustomerRole) {
            out.push_back("O : Logout");
            out.push_back("");
            out.push_back("1 : Home");
            out.push_back("2 : Restaurants");
            if (screen == 3 || screen == 4) {
                out.push_back("\n--- ACTIONS ---");
                if (screen == 3) {
                    out.push_back("ARROWS : Select");
                    out.push_back("A : Add to Cart");
                }
                out.push_back("V : View Cart");
                out.push_back("C : Checkout");
                out.push_back("P : Pay (Simulate)");
            }
        }
        else if (current.role == Role::OwnerRole) {
            out.push_back("O : Logout");
            out.push_back("1 : Home");
            out.push_back("2 : Restaurants");
            out.push_back("6 : Owner Dashboard");
            if (screen == 3) {
                out.push_back("\nARROWS : Navigate");
                out.push_back("U : Edit Menu");
            }
            if (screen == 6) out.push_back("\n(Mouse Click Buttons)");
        }
        else if (current.role == Role::AdminRole) {
            out.push_back("O : Logout");
            out.push_back("7 : Admin Dashboard");
            if (screen == 7) {
                out.push_back("\n(Click to Ban/Unban)");
            }
        }
        
        out.push_back("\nUP/DOWN : Scroll Page");
        return out;
    };

    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        bool mouseClicked = false;

        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            
            if (ev.type == sf::Event::MouseWheelScrolled) {
                if (ev.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    currentScrollY -= ev.mouseWheelScroll.delta * 30.0f;
                }
            }
            
            if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                mouseClicked = true;
            }

            if (ev.type == sf::Event::KeyPressed) {
                auto kc = ev.key.code;
                if (kc == sf::Keyboard::Escape) window.close();
                
                if (kc == sf::Keyboard::Up) currentScrollY -= 30.0f;
                if (kc == sf::Keyboard::Down) currentScrollY += 30.0f;

                if (kc == sf::Keyboard::L) {
                    performOnScreenLogin(current, customers, owners, vm, window, font);
                    if (current.role == Role::CustomerRole && !current.cust) {
                        for (const auto &c : customers) if (c.id == current.userId) { current.cust = std::make_shared<Customer>(c); break; }
                    }
                }
                else if (kc == sf::Keyboard::O) {
                    current = AppUser(); screen = 1; vm.play("welcome");
                }
                
                else if (screen == 2 && kc >= sf::Keyboard::Num1 && kc <= sf::Keyboard::Num9) {
                    int idx = (int)kc - (int)sf::Keyboard::Num1;
                    if (idx >= 0 && (size_t)idx < restaurants.size()) {
                        selRestaurant = idx;
                        selMenuItem = 0;
                        screen = 3;
                        currentScrollY = 0.f;
                    }
                }
                else if (kc == sf::Keyboard::Num1) { screen = 1; currentScrollY = 0.f; }
                else if (kc == sf::Keyboard::Num2) { screen = 2; currentScrollY = 0.f; }
                else if (kc == sf::Keyboard::Num5 && current.role == Role::CustomerRole) { screen = 5; currentScrollY = 0.f; }
                else if (kc == sf::Keyboard::Num6 && current.role == Role::OwnerRole) { screen = 6; currentScrollY = 0.f; }
                else if (kc == sf::Keyboard::Num7 && current.role == Role::AdminRole) { screen = 7; currentScrollY = 0.f; }
                
                if (screen == 3) {
                    if (kc == sf::Keyboard::Down) selMenuItem++; 
                    if (kc == sf::Keyboard::Up && selMenuItem > 0) selMenuItem--;
                    if (selMenuItem >= restaurants[selRestaurant].menu.size()) selMenuItem = restaurants[selRestaurant].menu.size() - 1;

                    if (kc == sf::Keyboard::A) {
                        if (current.role == Role::CustomerRole) {
                            const auto &r = restaurants[selRestaurant];
                            const auto &mi = r.menu[selMenuItem];
                            current.cust->addToCart(mi, 1, r.id, r.name);
                            showMessage(window, font, "Added " + mi.name);
                            vm.play("item_added");
                        } 
                    }
                    if (kc == sf::Keyboard::V && current.role == Role::CustomerRole) { screen = 4; currentScrollY = 0.f; }
                    
                    if (kc == sf::Keyboard::U && current.role == Role::OwnerRole) {
                        performOwnerEdit(current.ownerId, restaurants, selRestaurant, vm, window, font);
                    }
                }
                
                if ((screen == 3 || screen == 4) && kc == sf::Keyboard::C) {
                    if (current.role == Role::CustomerRole) performCheckoutConfirm(current.cust, vm, window, font);
                }
                
                if ((screen == 3 || screen == 4) && kc == sf::Keyboard::P) {
                     if (current.role == Role::CustomerRole) showMessage(window, font, "Payment Simulated."); 
                }
            }
        }

        window.clear(COL_BG);
        
        window.setView(window.getDefaultView());
        window.draw(headerRect);
        window.draw(header);

        float totalH = 0;
        
        // SCREEN 6: OWNER DASHBOARD
        if (screen == 6 && current.role == Role::OwnerRole) {
            window.setView(contentView);
            static int fc = 0; if (fc++ % 60 == 0) allOrders = Persistence::loadAllOrders("orders.txt");

            float yPos = 20.f;
            sf::Text rowText("", font, 18);
            std::vector<int> myRestIds;
            for(auto &r : restaurants) if(r.ownerId == current.ownerId) myRestIds.push_back(r.id);

            int count = 0;
            for (auto &o : allOrders) {
                // Filter: Only show orders for THIS owner's restaurants
                bool mine = false;
                for(int id : myRestIds) if(id == o.restaurantId) mine = true;
                if(!mine) continue;

                float cardHeight = 130.f + (o.items.size() * 20.f); 
                sf::RectangleShape card(sf::Vector2f(620.f, cardHeight));
                card.setPosition(20.f, yPos);
                card.setFillColor(COL_CARD);
                card.setOutlineThickness(1.f);
                card.setOutlineColor(sf::Color(70, 70, 70));
                window.draw(card);

                std::string statusStr = "[" + o.status + "]";
                std::string info = "Order #" + std::to_string(o.id) + "  " + statusStr + "  Total: " + std::to_string((int)o.total);
                
                rowText.setString(info);
                rowText.setPosition(35.f, yPos + 15.f);
                rowText.setFillColor(COL_ACCENT); 
                window.draw(rowText);

                sf::Text statusText(statusStr, font, 18);
                statusText.setPosition(rowText.findCharacterPos(info.find(statusStr))); 
                if (o.status == "Dispatched") statusText.setFillColor(COL_BTN_GREEN);
                else if (o.status == "Cancelled") statusText.setFillColor(COL_BTN_RED);
                else statusText.setFillColor(sf::Color::Yellow);

                std::string itemsStr = "";
                for(auto &it : o.items) itemsStr += "- " + it.itemSnapshot.name + " x" + std::to_string(it.qty) + "\n";
                
                sf::Text itemsText(itemsStr, font, 16);
                itemsText.setPosition(35.f, yPos + 45.f);
                itemsText.setFillColor(sf::Color::White);
                window.draw(itemsText);

                if (o.status == "Placed") {
                    float btnY = yPos + cardHeight - 40.f; // Bottom of card
                    float btnX = 35.f; 

                    sf::RectangleShape btnD(sf::Vector2f(100.f, 30.f));
                    btnD.setPosition(btnX, btnY);
                    btnD.setFillColor(COL_BTN_GREEN); 
                    sf::Text txtD("Dispatch", font, 14); txtD.setPosition(btnX + 15, btnY + 5);

                    sf::RectangleShape btnC(sf::Vector2f(100.f, 30.f));
                    btnC.setPosition(btnX + 120.f, btnY);
                    btnC.setFillColor(COL_BTN_RED);
                    sf::Text txtC("Cancel", font, 14); txtC.setPosition(btnX + 120.f + 25, btnY + 5);

                    sf::Vector2f worldMouse = window.mapPixelToCoords(mousePos, contentView);
                    
                    if (btnD.getGlobalBounds().contains(worldMouse)) {
                        btnD.setOutlineThickness(2.f); btnD.setOutlineColor(sf::Color::White);
                        if (mouseClicked) {
                            vm.play("order_dispatched"); 
                            o.dispatch(); Persistence::saveAllOrders(allOrders, "orders.txt"); 
                        }
                    }
                    if (btnC.getGlobalBounds().contains(worldMouse)) {
                        btnC.setOutlineThickness(2.f); btnC.setOutlineColor(sf::Color::White);
                        if (mouseClicked) {
                            o.cancel(); Persistence::saveAllOrders(allOrders, "orders.txt"); vm.play("order_cancel"); 
                        }
                    }
                    window.draw(btnD); window.draw(txtD); window.draw(btnC); window.draw(txtC);
                }
                
                yPos += cardHeight + 20.f;
                count++;
            }
            if (count == 0) {
                rowText.setString("No orders received yet."); rowText.setPosition(20, 20); 
                window.draw(rowText); totalH = 50;
            } else {
                totalH = yPos;
            }
        } 
        // --- SCREEN 7: ADMIN DASHBOARD ---
        else if (screen == 7 && current.role == Role::AdminRole) {
            window.setView(contentView);
            static int afc = 0; 
            if (afc++ % 60 == 0) {
                customers = Persistence::loadAllCustomers();
                owners = Persistence::loadAllOwners();
            }

            float yPos = 20.f;
            
            auto drawUserRow = [&](int id, std::string name, bool active, bool isOwner) {
                sf::RectangleShape card(sf::Vector2f(600.f, 50.f));
                card.setPosition(20.f, yPos);
                card.setFillColor(COL_CARD);
                window.draw(card);

                std::string info = (active ? "[ACTIVE] " : "[BANNED] ") + name + " (ID: " + std::to_string(id) + ")";
                sf::Text t(info, font, 18);
                t.setPosition(30.f, yPos + 12.f);
                t.setFillColor(active ? COL_BTN_GREEN : COL_BTN_RED);
                window.draw(t);

                sf::RectangleShape btn(sf::Vector2f(120.f, 30.f));
                btn.setPosition(480.f, yPos + 10.f);
                btn.setFillColor(active ? COL_BTN_RED : COL_BTN_GREEN);
                
                sf::Text bt(active ? "Deactivate" : "Activate", font, 14);
                bt.setPosition(480.f + 15.f, yPos + 7.f);
                window.draw(btn);
                window.draw(bt);

                sf::Vector2f wm = window.mapPixelToCoords(mousePos, contentView);
                if (btn.getGlobalBounds().contains(wm)) {
                    btn.setOutlineThickness(2.f); btn.setOutlineColor(sf::Color::White);
                    if (mouseClicked) {
                        if (isOwner) {
                            for(auto &o : owners) if(o.id == id) o.isActive = !o.isActive;
                            Persistence::saveAllOwners(owners);
                        } else {
                            for(auto &c : customers) if(c.id == id) c.isActive = !c.isActive;
                            Persistence::saveAllCustomers(customers);
                        }
                        vm.play(active ? "item_removed" : "item_added"); 
                    }
                }
            };

            sf::Text titleC("MANAGE CUSTOMERS", font, 22);
            titleC.setPosition(20.f, yPos); titleC.setFillColor(COL_ACCENT);
            window.draw(titleC);
            yPos += 40.f;

            for (const auto &c : customers) {
                drawUserRow(c.id, c.name, c.isActive, false);
                yPos += 60.f;
            }

            yPos += 30.f;

            sf::Text titleO("MANAGE OWNERS", font, 22);
            titleO.setPosition(20.f, yPos); titleO.setFillColor(COL_ACCENT);
            window.draw(titleO);
            yPos += 40.f;

            for (const auto &o : owners) {
                drawUserRow(o.id, o.name, o.isActive, true);
                yPos += 60.f;
            }

            totalH = yPos + 50.f;
        }
        // OTHER SCREENS
        else {
            std::ostringstream oss;
            if (screen == 1) oss << "Welcome to SustiEats.\n\nUse the sidebar to navigate.\nPress 'L' to Login.";
            else if (screen == 2) oss << restaurantListString();
            else if (screen == 3) oss << restaurantDetailString(selRestaurant);
            else if (screen == 4) {
                if (current.cust && current.cust->cart) {
                    oss << "YOUR CART:\n\n";
                    for (const auto &ci : current.cust->cart->items) 
                        oss << "- " << ci.item.name << " x" << ci.qty << "  (" << ci.subtotal() << " PKR)\n";
                    oss << "\nTotal: " << current.cust->cart->getTotal() << " PKR\n";
                } else oss << "Your cart is empty.\n";
            }
            else if (screen == 5) oss << customerDashboardString();
            
            body.setString(oss.str());
            totalH = body.getGlobalBounds().height + 40.f;
            
            window.setView(contentView);
            window.draw(body);
        }

        if (currentScrollY < 0.f) currentScrollY = 0.f;
        float maxScroll = std::max(0.f, totalH - contentHeight);
        if (currentScrollY > maxScroll) currentScrollY = maxScroll;
        contentView.setCenter(contentWidth/2.f, (contentHeight/2.f) + currentScrollY);

        window.setView(window.getDefaultView());
        window.draw(sidePanel);
        controls.setString(joinLines(makeControls()));
        window.draw(controls);

        window.display();
        sf::sleep(sf::milliseconds(16));
    }

    return 0;
}