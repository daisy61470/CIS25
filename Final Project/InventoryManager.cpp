// ============================================================
//  CIS 25 - C++ Programming  |  Final Project
//  Assignment 5: Build Your Own Application
//
//  PRODUCT INVENTORY MANAGER v1.0
//  Author : Daisy Schall
//  Date   : July 2026
//
//  A menu-driven console application for a small business.
//  Products live in a vector while the program runs and are
//  saved to inventory.txt, so nothing is lost when the
//  program closes. Every change is written to
//  transactions.txt with a timestamp, and every sale is
//  recorded in sales.txt.
//
//  Course concepts used:
//    - Class with constructor / destructor    (Module 10)
//    - Getters, setters, encapsulation        (Module 10)
//    - Pointers for record lookup             (Module 11)
//    - Vectors, loops, functions              (Modules 4-9)
//    - File streams for long-term storage     (fstream)
//    - Input validation with stringstream
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

using namespace std;

// ---------- constants ----------
const string INVENTORY_FILE  = "inventory.txt";
const string HISTORY_FILE    = "transactions.txt";
const string SALES_FILE      = "sales.txt";
const int    LOW_STOCK_LEVEL = 5;     // at or below this = *LOW* flag
const int    MAX_NAME_LENGTH = 20;

// ---------- small text helpers ----------

// remove spaces and tabs from both ends of a string
string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r");
    size_t end   = text.find_last_not_of(" \t\r");
    if (start == string::npos) {
        return "";
    }
    return text.substr(start, end - start + 1);
}

// lowercase copy of a string (used for case-insensitive search/sort)
string toLowerCase(const string& text) {
    string result = text;
    for (size_t i = 0; i < result.length(); i++) {
        result[i] = (char) tolower((unsigned char) result[i]);
    }
    return result;
}

// convert text to an int; true only if the WHOLE text is a number
bool textToInt(const string& text, int& value) {
    stringstream ss(trim(text));
    char extra;
    if (ss >> value && !(ss >> extra)) {
        return true;
    }
    return false;
}

// convert text to a double; true only if the WHOLE text is a number
bool textToDouble(const string& text, double& value) {
    stringstream ss(trim(text));
    char extra;
    if (ss >> value && !(ss >> extra)) {
        return true;
    }
    return false;
}

// current date and time as text, e.g. 2026-07-25 14:30:59
string currentTimestamp() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return string(buffer);
}

// ============================================================
//  Product class  (Module 10: classes, constructors, destructors)
// ============================================================
class Product {
private:
    int    id;
    string name;
    int    quantity;
    double price;

public:
    // default constructor
    Product() {
        id = 0;
        name = "";
        quantity = 0;
        price = 0.0;
    }

    // constructor with values
    Product(int newId, string newName, int newQuantity, double newPrice) {
        id = newId;
        name = newName;
        quantity = newQuantity;
        price = newPrice;
    }

    // destructor (nothing to clean up, included to show the concept)
    ~Product() {
    }

    // getters
    int    getId() const       { return id; }
    string getName() const     { return name; }
    int    getQuantity() const { return quantity; }
    double getPrice() const    { return price; }

    // setters
    void setQuantity(int newQuantity) { quantity = newQuantity; }
    void setPrice(double newPrice)    { price = newPrice; }

    // total value of this product's stock (quantity x price)
    double getValue() const { return quantity * price; }

    // print this product as one formatted table row
    void printRow() const {
        cout << left  << setw(6)  << id
             << setw(22) << name
             << right << setw(8)  << quantity
             << setw(11) << fixed << setprecision(2) << price
             << setw(14) << getValue();
        if (quantity <= LOW_STOCK_LEVEL) {
            cout << "   *LOW*";
        }
        cout << endl;
    }
};

// Undo memory: a snapshot of the whole product list taken
// right before the most recent change, so it can be restored.
struct UndoState {
    bool available = false;
    string description = "";
    vector<Product> snapshot;
};

// ---------- validated keyboard input ----------

// read one full line; if input closes, leave politely
string readRawLine() {
    string line;
    if (!getline(cin, line)) {
        cout << "\n[!] Input ended. Goodbye!" << endl;
        exit(0);
    }
    return line;
}

// keep asking until the user types a whole number in range
int readInt(const string& prompt, int minValue, int maxValue) {
    while (true) {
        cout << prompt;
        string input = readRawLine();
        int value;
        if (!textToInt(input, value)) {
            cout << "  [!] \"" << trim(input)
                 << "\" is not a whole number. Try again." << endl;
        } else if (value < minValue || value > maxValue) {
            cout << "  [!] Please enter a number between " << minValue
                 << " and " << maxValue << "." << endl;
        } else {
            return value;
        }
    }
}

// keep asking until the user types a valid number in range
double readDouble(const string& prompt, double minValue, double maxValue) {
    while (true) {
        cout << prompt;
        string input = readRawLine();
        double value;
        if (!textToDouble(input, value)) {
            cout << "  [!] \"" << trim(input)
                 << "\" is not a number. Try again." << endl;
        } else if (value < minValue || value > maxValue) {
            cout << "  [!] Please enter an amount between $" << fixed
                 << setprecision(2) << minValue << " and $" << maxValue
                 << "." << endl;
        } else {
            return value;
        }
    }
}

// keep asking until a usable product name is typed
string readProductName() {
    while (true) {
        cout << "Product name: ";
        string name = trim(readRawLine());
        if (name == "") {
            cout << "  [!] The name cannot be empty." << endl;
        } else if ((int) name.length() > MAX_NAME_LENGTH) {
            cout << "  [!] Please keep the name to " << MAX_NAME_LENGTH
                 << " characters or fewer." << endl;
        } else if (name.find('|') != string::npos) {
            cout << "  [!] The name cannot contain the | character." << endl;
        } else {
            return name;
        }
    }
}

// simple yes/no confirmation
bool readYesNo(const string& prompt) {
    while (true) {
        cout << prompt << " (y/n): ";
        string answer = toLowerCase(trim(readRawLine()));
        if (answer == "y" || answer == "yes") {
            return true;
        }
        if (answer == "n" || answer == "no") {
            return false;
        }
        cout << "  [!] Please answer y or n." << endl;
    }
}

// ---------- file storage (long-term data) ----------

// write every product to inventory.txt, one line each: id|name|qty|price
void saveInventory(const vector<Product>& items) {
    ofstream file(INVENTORY_FILE);
    if (!file) {
        cout << "[!] Could not open " << INVENTORY_FILE << " for writing." << endl;
        return;
    }
    for (size_t i = 0; i < items.size(); i++) {
        file << items[i].getId() << "|"
             << items[i].getName() << "|"
             << items[i].getQuantity() << "|"
             << fixed << setprecision(2) << items[i].getPrice() << "\n";
    }
}

// read inventory.txt back into the vector; returns how many loaded
int loadInventory(vector<Product>& items) {
    ifstream file(INVENTORY_FILE);
    if (!file) {
        return 0;                       // first run: no file yet, not an error
    }
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (trim(line) == "") {
            continue;                   // skip blank lines
        }
        stringstream ss(line);
        string idText, name, quantityText, priceText;
        if (getline(ss, idText, '|') && getline(ss, name, '|') &&
            getline(ss, quantityText, '|') && getline(ss, priceText)) {
            int id, quantity;
            double price;
            if (textToInt(idText, id) && textToInt(quantityText, quantity) &&
                textToDouble(priceText, price)) {
                items.push_back(Product(id, name, quantity, price));
                count++;
            }
        }
    }
    return count;
}

// append one timestamped line to transactions.txt
void logTransaction(const string& action, const string& details) {
    ofstream file(HISTORY_FILE, ios::app);
    if (file) {
        file << currentTimestamp() << " | " << left << setw(6) << action
             << " | " << details << "\n";
    }
}

// append one sale to sales.txt: timestamp|id|name|qty|total
void logSale(const Product& item, int quantitySold, double total) {
    ofstream file(SALES_FILE, ios::app);
    if (file) {
        file << currentTimestamp() << "|" << item.getId() << "|"
             << item.getName() << "|" << quantitySold << "|"
             << fixed << setprecision(2) << total << "\n";
    }
}

// ---------- finding products ----------

// Module 11: find a product by ID and return a POINTER to it.
// nullptr means "not found".
Product* findProductById(vector<Product>& items, int id) {
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].getId() == id) {
            return &items[i];
        }
    }
    return nullptr;
}

// position of a product in the vector (-1 = not found), used for delete
int findIndexById(const vector<Product>& items, int id) {
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].getId() == id) {
            return (int) i;
        }
    }
    return -1;
}

// ---------- shared table printing ----------

void printTableHeader() {
    cout << left  << setw(6)  << "ID"
         << setw(22) << "Name"
         << right << setw(8)  << "Qty"
         << setw(11) << "Price"
         << setw(14) << "Value" << endl;
    cout << string(61, '-') << endl;
}

// take a snapshot of the list so Undo can restore it later
void rememberState(UndoState& undo, const vector<Product>& items,
                   const string& description) {
    undo.available = true;
    undo.description = description;
    undo.snapshot = items;
}

// ============================================================
//  Menu option 1: add a product
// ============================================================
void addProduct(vector<Product>& items, UndoState& undo) {
    cout << "\n--- ADD A PRODUCT ---" << endl;
    int id = readInt("Product ID (1-99999): ", 1, 99999);
    while (findProductById(items, id) != nullptr) {
        cout << "  [!] ID " << id << " is already used by \""
             << findProductById(items, id)->getName()
             << "\". Pick a different ID." << endl;
        id = readInt("Product ID (1-99999): ", 1, 99999);
    }
    string name    = readProductName();
    int quantity   = readInt("Quantity in stock (0-100000): ", 0, 100000);
    double price   = readDouble("Price per unit (0-100000): ", 0.0, 100000.0);

    rememberState(undo, items, "added \"" + name + "\"");
    items.push_back(Product(id, name, quantity, price));
    saveInventory(items);

    stringstream details;
    details << name << " (ID " << id << ") qty " << quantity
            << " @ $" << fixed << setprecision(2) << price;
    logTransaction("ADD", details.str());
    cout << "[OK] Added and saved: " << details.str() << endl;
}

// ============================================================
//  Menu option 2: view every product as a table
// ============================================================
void viewInventory(const vector<Product>& items) {
    cout << "\n--- CURRENT INVENTORY ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty. Add your first product with option 1!" << endl;
        return;
    }
    printTableHeader();
    double totalValue = 0;
    for (size_t i = 0; i < items.size(); i++) {
        items[i].printRow();
        totalValue += items[i].getValue();
    }
    cout << string(61, '-') << endl;
    cout << items.size() << " product(s) | Total inventory value: $"
         << fixed << setprecision(2) << totalValue << endl;
    cout << "(*LOW* = " << LOW_STOCK_LEVEL << " or fewer in stock)" << endl;
}

// ============================================================
//  Menu option 3: search by ID (exact) or name (partial)
// ============================================================
void searchProducts(vector<Product>& items) {
    cout << "\n--- SEARCH PRODUCTS ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    cout << " 1. Search by ID" << endl;
    cout << " 2. Search by name (partial matches count)" << endl;
    int choice = readInt("Search how? (1-2): ", 1, 2);

    if (choice == 1) {
        int id = readInt("ID to look for: ", 1, 99999);
        Product* found = findProductById(items, id);    // pointer lookup
        if (found == nullptr) {
            cout << "No product has ID " << id << "." << endl;
        } else {
            printTableHeader();
            found->printRow();                          // access through the pointer
        }
    } else {
        cout << "Name (or part of it): ";
        string searchText = toLowerCase(trim(readRawLine()));
        if (searchText == "") {
            cout << "  [!] Nothing typed, search cancelled." << endl;
            return;
        }
        int matches = 0;
        for (size_t i = 0; i < items.size(); i++) {
            if (toLowerCase(items[i].getName()).find(searchText) != string::npos) {
                if (matches == 0) {
                    printTableHeader();
                }
                items[i].printRow();
                matches++;
            }
        }
        if (matches == 0) {
            cout << "No product names contain \"" << searchText << "\"." << endl;
        } else {
            cout << matches << " match(es) found." << endl;
        }
    }
}

// ============================================================
//  Menu option 4: update quantity and/or price
// ============================================================
void updateProduct(vector<Product>& items, UndoState& undo) {
    cout << "\n--- UPDATE A PRODUCT ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    int id = readInt("ID of the product to update: ", 1, 99999);
    Product* product = findProductById(items, id);
    if (product == nullptr) {
        cout << "No product has ID " << id << "." << endl;
        return;
    }

    cout << "Current details:" << endl;
    printTableHeader();
    product->printRow();

    cout << " 1. Change quantity" << endl;
    cout << " 2. Change price" << endl;
    cout << " 3. Change both" << endl;
    int choice = readInt("What would you like to change? (1-3): ", 1, 3);

    rememberState(undo, items, "updated \"" + product->getName() + "\"");
    stringstream details;
    details << product->getName() << " (ID " << id << ")";

    if (choice == 1 || choice == 3) {
        int newQuantity = readInt("New quantity (0-100000): ", 0, 100000);
        details << " qty " << product->getQuantity() << " -> " << newQuantity;
        product->setQuantity(newQuantity);
    }
    if (choice == 2 || choice == 3) {
        double newPrice = readDouble("New price (0-100000): ", 0.0, 100000.0);
        details << " price $" << fixed << setprecision(2)
                << product->getPrice() << " -> $" << newPrice;
        product->setPrice(newPrice);
    }
    saveInventory(items);
    logTransaction("UPDATE", details.str());
    cout << "[OK] Updated and saved." << endl;
}

// ============================================================
//  Menu option 5: delete a product (with confirmation)
// ============================================================
void deleteProduct(vector<Product>& items, UndoState& undo) {
    cout << "\n--- DELETE A PRODUCT ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    int id = readInt("ID of the product to delete: ", 1, 99999);
    int position = findIndexById(items, id);
    if (position == -1) {
        cout << "No product has ID " << id << "." << endl;
        return;
    }

    printTableHeader();
    items[position].printRow();
    if (!readYesNo("Really delete this product?")) {
        cout << "Delete cancelled - nothing changed." << endl;
        return;
    }
    string name = items[position].getName();
    rememberState(undo, items, "deleted \"" + name + "\"");
    items.erase(items.begin() + position);
    saveInventory(items);
    logTransaction("DELETE", name + " (ID " + to_string(id) + ")");
    cout << "[OK] \"" << name << "\" deleted. "
         << items.size() << " product(s) left." << endl;
}

// ============================================================
//  Menu option 6: sell a product and print a receipt
// ============================================================
void sellProduct(vector<Product>& items, UndoState& undo) {
    cout << "\n--- SELL A PRODUCT ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    int id = readInt("ID of the product being sold: ", 1, 99999);
    Product* product = findProductById(items, id);
    if (product == nullptr) {
        cout << "No product has ID " << id << "." << endl;
        return;
    }
    if (product->getQuantity() == 0) {
        cout << "\"" << product->getName()
             << "\" is OUT OF STOCK. Restock it before selling." << endl;
        return;
    }
    cout << "\"" << product->getName() << "\" - " << product->getQuantity()
         << " in stock at $" << fixed << setprecision(2)
         << product->getPrice() << " each." << endl;
    int quantitySold = readInt("How many are being sold? ", 1,
                               product->getQuantity());

    rememberState(undo, items, "sold " + to_string(quantitySold) +
                  " x \"" + product->getName() + "\"");
    double total = quantitySold * product->getPrice();
    product->setQuantity(product->getQuantity() - quantitySold);
    saveInventory(items);
    logSale(*product, quantitySold, total);

    stringstream details;
    details << quantitySold << " x " << product->getName()
            << " (ID " << id << ") = $" << fixed << setprecision(2) << total;
    logTransaction("SALE", details.str());

    // the receipt
    cout << "\n+----------------------------------+" << endl;
    cout << "|           SALE RECEIPT           |" << endl;
    cout << "+----------------------------------+" << endl;
    cout << "  " << currentTimestamp() << endl;
    cout << "  Item : " << product->getName() << " (ID " << id << ")" << endl;
    cout << "  Qty  : " << quantitySold << " @ $" << fixed
         << setprecision(2) << product->getPrice() << " each" << endl;
    cout << "  TOTAL: $" << total << endl;
    cout << "  Stock remaining: " << product->getQuantity() << endl;
    if (product->getQuantity() <= LOW_STOCK_LEVEL) {
        cout << "  (!) Stock is now LOW - see the restock report." << endl;
    }
    cout << "+----------------------------------+" << endl;
    cout << "     Thank you for your purchase!" << endl;
}

// ---------- comparison functions used by sort ----------
bool compareByName(const Product& a, const Product& b) {
    return toLowerCase(a.getName()) < toLowerCase(b.getName());
}
bool compareByPriceLowHigh(const Product& a, const Product& b) {
    return a.getPrice() < b.getPrice();
}
bool compareByPriceHighLow(const Product& a, const Product& b) {
    return a.getPrice() > b.getPrice();
}
bool compareByQuantity(const Product& a, const Product& b) {
    return a.getQuantity() < b.getQuantity();
}

// ============================================================
//  Menu option 7: sort the inventory
// ============================================================
void sortProducts(vector<Product>& items) {
    cout << "\n--- SORT PRODUCTS ---" << endl;
    if (items.size() < 2) {
        cout << "Nothing to sort yet - add more products first." << endl;
        return;
    }
    cout << " 1. By name (A-Z)" << endl;
    cout << " 2. By price (low to high)" << endl;
    cout << " 3. By price (high to low)" << endl;
    cout << " 4. By quantity (low to high)" << endl;
    int choice = readInt("Sort how? (1-4): ", 1, 4);

    if (choice == 1) {
        sort(items.begin(), items.end(), compareByName);
    } else if (choice == 2) {
        sort(items.begin(), items.end(), compareByPriceLowHigh);
    } else if (choice == 3) {
        sort(items.begin(), items.end(), compareByPriceHighLow);
    } else {
        sort(items.begin(), items.end(), compareByQuantity);
    }
    saveInventory(items);        // keep the new order in the file too
    viewInventory(items);
}

// ---------- reports ----------

void lowStockReport(const vector<Product>& items) {
    cout << "\n--- LOW STOCK & RESTOCK LIST ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    int threshold = readInt("Flag products at or below what stock level? (suggested 5): ",
                            0, 100000);
    int found = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].getQuantity() <= threshold) {
            if (found == 0) {
                cout << left  << setw(6)  << "ID"
                     << setw(22) << "Name"
                     << right << setw(8)  << "Stock"
                     << setw(18) << "Suggested order" << endl;
                cout << string(54, '-') << endl;
            }
            int suggested = threshold * 2 - items[i].getQuantity();
            if (suggested < 1) {
                suggested = 1;
            }
            cout << left  << setw(6)  << items[i].getId()
                 << setw(22) << items[i].getName()
                 << right << setw(8)  << items[i].getQuantity()
                 << setw(18) << suggested << endl;
            found++;
        }
    }
    if (found == 0) {
        cout << "No products are at or below " << threshold
             << ". Stock looks healthy!" << endl;
    } else {
        cout << found << " product(s) need restocking (suggestion brings each "
             << "back to " << threshold * 2 << ")." << endl;
    }
}

void valueReport(const vector<Product>& items) {
    cout << "\n--- INVENTORY VALUE REPORT ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    printTableHeader();
    double totalValue = 0;
    size_t mostValuable = 0;
    for (size_t i = 0; i < items.size(); i++) {
        items[i].printRow();
        totalValue += items[i].getValue();
        if (items[i].getValue() > items[mostValuable].getValue()) {
            mostValuable = i;
        }
    }
    cout << string(61, '-') << endl;
    cout << "TOTAL INVENTORY VALUE: $" << fixed << setprecision(2)
         << totalValue << endl;
    cout << "Most valuable stock: \"" << items[mostValuable].getName()
         << "\" at $" << items[mostValuable].getValue() << endl;
}

void salesSummary() {
    cout << "\n--- SALES SUMMARY ---" << endl;
    ifstream file(SALES_FILE);
    if (!file) {
        cout << "No sales recorded yet. Sell something with option 6!" << endl;
        return;
    }
    cout << left  << setw(21) << "Date/Time"
         << setw(22) << "Product"
         << right << setw(6)  << "Qty"
         << setw(12) << "Total" << endl;
    cout << string(61, '-') << endl;
    string line;
    int count = 0;
    double revenue = 0;
    while (getline(file, line)) {
        if (trim(line) == "") {
            continue;
        }
        stringstream ss(line);
        string when, idText, name, qtyText, totalText;
        if (getline(ss, when, '|') && getline(ss, idText, '|') &&
            getline(ss, name, '|') && getline(ss, qtyText, '|') &&
            getline(ss, totalText)) {
            int qty;
            double total;
            if (textToInt(qtyText, qty) && textToDouble(totalText, total)) {
                cout << left  << setw(21) << when
                     << setw(22) << name
                     << right << setw(6)  << qty
                     << setw(12) << fixed << setprecision(2) << total << endl;
                count++;
                revenue += total;
            }
        }
    }
    if (count == 0) {
        cout << "No sales recorded yet. Sell something with option 6!" << endl;
    } else {
        cout << string(61, '-') << endl;
        cout << count << " sale(s) | TOTAL REVENUE: $" << fixed
             << setprecision(2) << revenue << endl;
    }
}

void stockChart(const vector<Product>& items) {
    cout << "\n--- STOCK LEVEL CHART ---" << endl;
    if (items.empty()) {
        cout << "The inventory is empty." << endl;
        return;
    }
    int maxQuantity = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].getQuantity() > maxQuantity) {
            maxQuantity = items[i].getQuantity();
        }
    }
    int unitsPerBlock = 1;
    if (maxQuantity > 40) {
        unitsPerBlock = (maxQuantity + 39) / 40;   // keep bars on screen
    }
    cout << "(each # = " << unitsPerBlock << " unit(s) in stock)" << endl << endl;
    for (size_t i = 0; i < items.size(); i++) {
        cout << left << setw(22) << items[i].getName() << "|";
        int blocks = items[i].getQuantity() / unitsPerBlock;
        for (int b = 0; b < blocks; b++) {
            cout << "#";
        }
        cout << " " << items[i].getQuantity();
        if (items[i].getQuantity() <= LOW_STOCK_LEVEL) {
            cout << "  *LOW*";
        }
        cout << endl;
    }
}

// ============================================================
//  Menu option 8: reports sub-menu
// ============================================================
void reportsMenu(const vector<Product>& items) {
    cout << "\n--- REPORTS & CHART ---" << endl;
    cout << " 1. Low stock & restock list" << endl;
    cout << " 2. Inventory value report" << endl;
    cout << " 3. Sales summary" << endl;
    cout << " 4. Stock level chart" << endl;
    int choice = readInt("Which report? (1-4): ", 1, 4);
    if (choice == 1) {
        lowStockReport(items);
    } else if (choice == 2) {
        valueReport(items);
    } else if (choice == 3) {
        salesSummary();
    } else {
        stockChart(items);
    }
}

// ============================================================
//  Menu option 9: show the transaction history file
// ============================================================
void viewHistory() {
    cout << "\n--- TRANSACTION HISTORY ---" << endl;
    ifstream file(HISTORY_FILE);
    if (!file) {
        cout << "No history yet. Adds, updates, deletes, sales and undos "
             << "all get recorded here." << endl;
        return;
    }
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (trim(line) != "") {
            cout << line << endl;
            count++;
        }
    }
    if (count == 0) {
        cout << "No history yet." << endl;
    } else {
        cout << string(40, '-') << endl;
        cout << count << " event(s) recorded in " << HISTORY_FILE << endl;
    }
}

// ============================================================
//  Menu option 10: undo the most recent change
// ============================================================
void undoLastChange(vector<Product>& items, UndoState& undo) {
    cout << "\n--- UNDO LAST CHANGE ---" << endl;
    if (!undo.available) {
        cout << "Nothing to undo right now. (One change can be undone, "
             << "and only from this session.)" << endl;
        return;
    }
    cout << "Last change: you " << undo.description << "." << endl;
    if (!readYesNo("Undo it?")) {
        cout << "Undo cancelled." << endl;
        return;
    }
    items = undo.snapshot;
    saveInventory(items);
    logTransaction("UNDO", "reversed the last change (" + undo.description + ")");
    undo.available = false;
    cout << "[OK] The inventory is back to how it was before you "
         << undo.description << "." << endl;
    if (undo.description.rfind("sold", 0) == 0) {
        cout << "Note: the receipt already issued stays in the sales summary." << endl;
    }
}

// ---------- main menu ----------
void showMainMenu(int productCount) {
    cout << "\n==================================================" << endl;
    cout << "           PRODUCT INVENTORY MANAGER" << endl;
    cout << "           (" << productCount << " product(s) on file)" << endl;
    cout << "==================================================" << endl;
    cout << "  1. Add a product" << endl;
    cout << "  2. View all products" << endl;
    cout << "  3. Search products" << endl;
    cout << "  4. Update a product" << endl;
    cout << "  5. Delete a product" << endl;
    cout << "  6. Sell a product (prints a receipt)" << endl;
    cout << "  7. Sort products" << endl;
    cout << "  8. Reports & stock chart" << endl;
    cout << "  9. Transaction history" << endl;
    cout << " 10. Undo last change" << endl;
    cout << " 11. Save and exit" << endl;
    cout << "==================================================" << endl;
}

int main() {
    vector<Product> inventory;
    UndoState undo;

    cout << "==================================================" << endl;
    cout << "      PRODUCT INVENTORY MANAGER  v1.0" << endl;
    cout << "      CIS 25 Final Project  |  Daisy Schall" << endl;
    cout << "==================================================" << endl;

    int loaded = loadInventory(inventory);
    if (loaded > 0) {
        cout << "Loaded " << loaded << " product(s) from "
             << INVENTORY_FILE << "." << endl;
    } else {
        cout << "No saved inventory found - starting a brand new one." << endl;
    }

    bool running = true;
    while (running) {
        showMainMenu((int) inventory.size());
        int choice = readInt("Choose an option (1-11): ", 1, 11);

        if (choice == 1) {
            addProduct(inventory, undo);
        } else if (choice == 2) {
            viewInventory(inventory);
        } else if (choice == 3) {
            searchProducts(inventory);
        } else if (choice == 4) {
            updateProduct(inventory, undo);
        } else if (choice == 5) {
            deleteProduct(inventory, undo);
        } else if (choice == 6) {
            sellProduct(inventory, undo);
        } else if (choice == 7) {
            sortProducts(inventory);
        } else if (choice == 8) {
            reportsMenu(inventory);
        } else if (choice == 9) {
            viewHistory();
        } else if (choice == 10) {
            undoLastChange(inventory, undo);
        } else {
            saveInventory(inventory);
            cout << "\nEverything is saved in " << INVENTORY_FILE
                 << ". See you next time!" << endl;
            running = false;
        }
    }
    return 0;
}
