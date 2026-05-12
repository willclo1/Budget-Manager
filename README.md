# Budget Manager

Budget Manager is a C++ terminal-based budgeting app with a full TUI interface, persistent JSON storage, monthly budget tracking, expense categories, budget history, and HTML chart reports.

## Features

- Create a monthly budget
- Add expenses by name, amount, and category
- View current month spending
- Track remaining budget
- View all saved monthly budgets
- Drill down into past budget details
- Generate an HTML report with charts
- Package as a macOS `.app`
- Custom app icon support

## Tech Stack

- C++
- CMake
- FTXUI
- nlohmann/json
- Chart.js for generated HTML reports

## Build and Run

```bash
cmake -B build
cmake --build build
./build/Budgeting
