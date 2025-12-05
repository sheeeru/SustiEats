# SustiEats C++ Food Ordering System 

to run this code run CMakeList.txt first and then run this cmd :-

g++  -Iinclude -I "C:\msys64\ucrt64\SFML-2.6.2\include"  src/*.cpp main.cpp -L "C:\msys64\ucrt64\SFML-2.6.2\lib"  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o proj.exe 


Author: Shaheer Qureshi , Arqish Zaria

Language: C++17

Libraries: SFML (Graphics & Audio)

📝 Project Overview

SustiEats is a console and window-based food ordering application. It simulates a real-world system where Customers can buy food, Owners can manage menus, and Admins can ban users. It features a graphical interface, voice feedback, and file-based database storage.

 Object-Oriented Programming (OOP) Concepts Used

This project relies heavily on OOP to keep code organized and secure.

1. Inheritance 👨‍👦

Inheritance allows classes to share common logic.

User Class (Base): Holds common data like id, name, password, and isActive.

Customer, Owner, Admin (Derived): These classes inherit from User.

Example: A Customer gets the login() function automatically because it inherits from User.

2. Polymorphism 🎭

Polymorphism allows different objects to be treated as the same type (User).

Virtual Functions: The User class has virtual methods like displayDashboard().

Usage: Although we mostly cast to specific shared pointers (like shared_ptr<Customer>), the base structure supports treating everyone as a generic User.

3. Encapsulation 💊

Encapsulation wraps data and logic together to protect it.

Logic Hiding: The complex math for Loyalty Points is hidden inside LoyaltyManager. The main code just asks "process checkout," and the manager handles the details internally.

Data Protection: Order status logic (dispatch, cancel) is protected. You cannot just set status = "Random"; you must use the specific functions that enforce rules (e.g., you can't cancel a dispatched order).

4. Memory Management (Smart Pointers) 🧠

unique_ptr: Used in Customer for the Cart. This ensures a cart belongs to only one customer and cannot be accidentally copied (which prevents bugs).

shared_ptr: Used in main.cpp to track the currently logged-in user.

📂 Class & Function Dictionary

Here is a simple guide to every function in the project.

🧱 Core Classes

User (Base Class)

login(password): Checks if the password matches.

logout(): Placeholder for logging out logic.

displayDashboard(): Virtual function to show user-specific data.

Customer (Inherits User)

addToCart(item, qty, ...): Adds a food item to the customer's unique cart.

checkout(): Converts Cart items into Orders and returns them. Clears the cart afterwards.

applyPointsToOrder(...) / consumePoints(...): Logic for using loyalty points.

Copy Constructor: Custom logic to safely copy a customer's unique cart.

Owner (Inherits User)

addMenuItem(...): Adds a new food item to a restaurant menu.

updateOrderStatus(...): Used to change an order from "Placed" to "Dispatched".

Restaurant

addMenuItem(item): Pushes a new item to the menu vector.

removeMenuItem(id): Finds an item by ID and deletes it from the menu vector.

Order

place(): Finalizes the order and calculates the total price.

dispatch(): Changes status to Dispatched (only allowed if currently Placed).

cancel(): Changes status to Cancelled (only allowed if currently Placed).

Cart

addItem(...): Adds an item or increases quantity if it already exists.

removeItem(id): Removes an item from the shopping list.

getTotal(): Loops through items to calculate the final bill.

clear(): Empties the cart.

⚙️ Managers & Utilities

Persistence (The Database)

verifyAdmin(id, pass): Checks admin.txt to see if credentials match.

saveCustomer / loadAllCustomers: Reads/Writes to customers.txt.

saveOwner / loadAllOwners: Reads/Writes to owners.txt.

saveRestaurant / loadAllRestaurants: Reads/Writes to restaurants.txt.

saveOrder / loadAllOrders: Reads/Writes to orders.txt.

saveAll...: Specialized functions that overwrite the file (used for updating statuses or fixing corruption) instead of appending.

getNextId(filename): Scans a file to find the highest ID and returns highest + 1.

LoyaltyManager (Logic)

isEligibleForDiscount(customer): Returns true if points >= 1000.

processCheckout(...): The Master Function. It:

Generates a unique Order ID.

Applies discounts (10% off) if requested.

Saves the orders to text files.

Adds +10 points to the customer.

Updates the customer file.

VoiceManager (Audio)

loadVoice(key, path): Loads an .ogg file into memory.

play(key): Plays the sound effect associated with the key name.

🖥️ Interface (Main.cpp)

The main file handles the visual interface using SFML.

showTextInput(...): Pops up a graphical box to type (e.g., ID, Password).

showMessage(...): Shows a popup alert with a message.

showYesNo(...): Asks a Yes/No question and returns true/false.

performOnScreenLogin(...): The login flow. Checks ID/Pass and isActive status.

performOwnerEdit(...): Allows owners to add/remove menu items (Restricted to their own restaurants).

performCheckoutConfirm(...): Handles the checkout flow, asks for Loyalty usage, and calls LoyaltyManager.

main(): The infinite loop that draws the screens (Home, List, Detail, Cart, Dashboard).


