# Product Inventory Manager

**CIS 25 - C++ Programming | Final Project (Assignment 5: Build Your Own Application)**
**Author: Daisy Schall**

A menu-driven C++ console application that manages a small business's product
inventory. Products are kept in long-term storage, so everything is still there
the next time the program runs.

## Features

- Add, view, search, update, and delete products (full CRUD)
- **Long-term storage**: the inventory auto-saves to `inventory.txt` after every
  change and loads back on startup
- **Sell mode**: sells stock, prints a receipt on screen, and records revenue in
  `sales.txt`
- **Transaction history**: every add, update, delete, sale, and undo is
  timestamped in `transactions.txt`
- **Low-stock alerts**: products at or below the threshold are flagged `*LOW*`,
  with a restock report that suggests how many to reorder
- **Inventory value report**: value of every product plus the grand total
- **Sales summary**: every sale with total revenue earned
- **Stock level chart**: ASCII bar chart of stock right in the console
- **Undo**: reverses the most recent change
- **Sorting**: by name, price (both directions), or quantity
- **Input validation everywhere**: rejects letters where numbers belong, negative
  prices, duplicate IDs, empty names, and out-of-range amounts

## How to compile and run

```bash
g++ -Wall -Wextra -std=c++17 -o inventory InventoryManager.cpp
./inventory
```

Works with any C++17 compiler (g++, clang++, OnlineGDB).

## Files

| File | What it is |
|------|------------|
| `InventoryManager.cpp` | Full source code |
| `inventory.txt` | Saved inventory (sample data included), format: `id\|name\|qty\|price` |
| `transactions.txt` | Timestamped log of every change |
| `sales.txt` | Sales record, format: `timestamp\|id\|name\|qty\|total` |
| `samplerun.txt` | Transcript of a real session showing the features |

## Course concepts used

- Class with constructor / destructor, getters and setters (Module 10)
- Pointers for record lookup (Module 11)
- Vectors, functions, loops (Modules 4-9)
- File streams (`ifstream` / `ofstream`) for persistent storage
- Input validation with `stringstream`
- `std::sort` with custom comparison functions
