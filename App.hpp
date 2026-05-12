#ifndef BUDGETING_APP_HPP
#define BUDGETING_APP_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "Budget.hpp"
#include "Expense.hpp"
#include "Storage.hpp"
#include "json.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;
using json = nlohmann::json;

class App {
private:
    Budget* budget;
    bool hasBudget;
    std::string message;

    int page = 0;
    std::string budgetInput;
    std::string expenseNameInput;
    std::string expenseAmountInput;
    int selectedType = 0;

    json allBudgets;
    std::vector<std::string> budgetLabels;
    int selectedBudget = 0;

public:
    App() {
        budget = loadBudget();
        hasBudget = budget != nullptr;
        message = hasBudget ? "Loaded existing budget." : "No budget loaded.";
    }

    ~App() {
        delete budget;
    }

    void run() {
        auto screen = ScreenInteractive::Fullscreen();
        screen.TrackMouse(true);

        std::vector<std::string> menuEntries = {
                "Create Monthly Budget",
                "Add Expense",
                "View All Budgets",
                "Generate HTML Charts",
                "Delete Budget",
                "Exit"
        };

        std::vector<std::string> typeEntries = {
                "Housing",
                "Food",
                "Transportation",
                "Utilities",
                "Health",
                "Entertainment",
                "Shopping",
                "Savings",
                "Debt",
                "Other"
        };

        int selectedMenu = 0;

        auto menu = Menu(&menuEntries, &selectedMenu);
        auto budgetField = Input(&budgetInput, "2500");
        auto expenseNameField = Input(&expenseNameInput, "Taco Bell");
        auto expenseAmountField = Input(&expenseAmountInput, "12.50");
        auto typeMenu = Menu(&typeEntries, &selectedType);
        auto budgetHistoryMenu = Menu(&budgetLabels, &selectedBudget);

        auto createButton = Button("Create Budget", [&] {
            try {
                double amount = std::stod(budgetInput);

                std::vector<Expense> expenses;
                delete budget;
                budget = new Budget(expenses, amount);

                hasBudget = true;
                saveBudget(*budget);

                budgetInput = "";
                message = "Budget created successfully.";
                page = 0;
            } catch (...) {
                message = "Invalid budget amount.";
            }
        });

        auto addExpenseButton = Button("Save Expense", [&] {
            if (!hasBudget) {
                message = "Create a budget first.";
                page = 0;
                return;
            }

            try {
                double amount = std::stod(expenseAmountInput);

                Expense expense(
                        expenseNameInput,
                        amount,
                        static_cast<Type>(selectedType)
                );

                budget->addExpense(expense);
                saveBudget(*budget);

                expenseNameInput = "";
                expenseAmountInput = "";
                selectedType = 0;

                message = "Expense added successfully.";
                page = 0;
            } catch (...) {
                message = "Invalid expense amount.";
            }
        });

        auto backButton = Button("Back", [&] {
            page = 0;
        });

        auto dashboardContainer = Container::Vertical({
                                                              menu
                                                      });

        auto createBudgetContainer = Container::Vertical({
                                                                 budgetField,
                                                                 createButton,
                                                                 backButton
                                                         });

        auto addExpenseContainer = Container::Vertical({
                                                               expenseNameField,
                                                               expenseAmountField,
                                                               typeMenu,
                                                               addExpenseButton,
                                                               backButton
                                                       });

        auto historyContainer = Container::Vertical({
                                                            budgetHistoryMenu,
                                                            backButton
                                                    });

        auto mainContainer = Container::Tab({
                                                    dashboardContainer,
                                                    createBudgetContainer,
                                                    addExpenseContainer,
                                                    historyContainer
                                            }, &page);

        auto renderer = Renderer(mainContainer, [&] {
            if (page == 1) {
                return renderCreateBudget(budgetField, createButton, backButton);
            }

            if (page == 2) {
                return renderAddExpense(
                        expenseNameField,
                        expenseAmountField,
                        typeMenu,
                        addExpenseButton,
                        backButton
                );
            }

            if (page == 3) {
                return renderAllBudgets(budgetHistoryMenu, backButton);
            }

            return renderDashboard(menu);
        });

        renderer |= CatchEvent([&](Event event) {
            if (event == Event::Return && page == 0) {
                if (selectedMenu == 0) {
                    if (hasBudget) {
                        message = "Budget already exists. Delete it first.";
                    } else {
                        page = 1;
                    }
                }
                else if (selectedMenu == 1) {
                    if (!hasBudget) {
                        message = "Create a budget first.";
                    } else {
                        page = 2;
                    }
                }
                else if (selectedMenu == 2) {
                    loadBudgetLabels();
                    page = 3;
                }
                else if (selectedMenu == 3) {
                    if (!hasBudget) {
                        message = "Create a budget first.";
                    } else {
                        generateHtmlReport(*budget);
                        openHtmlReport();
                        message = "HTML report opened.";
                    }
                }
                else if (selectedMenu == 4) {
                    deleteBudget();
                }
                else if (selectedMenu == 5) {
                    if (hasBudget) {
                        saveBudget(*budget);
                    }

                    screen.ExitLoopClosure()();
                }

                return true;
            }

            return false;
        });

        screen.Loop(renderer);
    }

private:
    Element shell(const std::string& title, Element body) {
        return vbox({
                            text("BUDGET MANAGER") | bold | center | color(Color::Cyan),
                            text("Track spending • Review history • Generate reports") | center | dim,
                            separator() | color(Color::Cyan),
                            text(title) | bold | center | color(Color::Yellow),
                            separator(),
                            body
                    }) | borderDouble | color(Color::Cyan);
    }

    Element renderDashboard(Component menu) {
        Element body = hbox({
                                    dashboard() | flex,

                                    separator(),

                                    vbox({
                                                 text("Navigation") | bold | color(Color::Yellow),
                                                 separator(),
                                                 menu->Render(),
                                                 filler(),
                                                 separator(),
                                                 text(message) | dim | color(Color::Green)
                                         }) | border | size(WIDTH, GREATER_THAN, 30)
                            });

        return shell("Dashboard", body);
    }

    Element renderCreateBudget(Component input, Component createButton, Component backButton) {
        Element body = vbox({
                                    text("Set the spending limit for this month.") | dim,
                                    separator(),

                                    vbox({
                                                 text("Monthly Budget") | bold | color(Color::Cyan),
                                                 input->Render() | border | color(Color::White)
                                         }) | border | color(Color::Blue),

                                    separator(),

                                    hbox({
                                                 createButton->Render() | size(WIDTH, GREATER_THAN, 22),
                                                 text("  "),
                                                 backButton->Render() | size(WIDTH, GREATER_THAN, 14)
                                         }),

                                    filler(),
                                    separator(),
                                    text(message) | dim | color(Color::Green)
                            });

        return shell("Create Monthly Budget", body);
    }

    Element renderAddExpense(
            Component nameInput,
            Component amountInput,
            Component typeMenu,
            Component addButton,
            Component backButton
    ) {
        Element body = hbox({
                                    vbox({
                                                 text("Expense Details") | bold | color(Color::Yellow),
                                                 separator(),

                                                 text("Name") | bold,
                                                 nameInput->Render() | border,

                                                 text("Amount") | bold,
                                                 amountInput->Render() | border,

                                                 separator(),

                                                 hbox({
                                                              addButton->Render() | size(WIDTH, GREATER_THAN, 18),
                                                              text("  "),
                                                              backButton->Render() | size(WIDTH, GREATER_THAN, 12)
                                                      }),

                                                 filler(),
                                                 separator(),
                                                 text(message) | dim | color(Color::Green)
                                         }) | border | flex,

                                    separator(),

                                    vbox({
                                                 text("Category") | bold | color(Color::Cyan),
                                                 separator(),
                                                 typeMenu->Render()
                                         }) | border | size(WIDTH, GREATER_THAN, 28)
                            });

        return shell("Add Expense", body);
    }

    Element renderAllBudgets(Component budgetHistoryMenu, Component backButton) {
        Element body = vbox({
                                    hbox({
                                                 vbox({
                                                              text("Budget History") | bold | color(Color::Yellow),
                                                              separator(),
                                                              budgetHistoryMenu->Render()
                                                      }) | border | flex,

                                                 separator(),

                                                 renderSelectedBudgetDetails() | border | flex
                                         }),

                                    separator(),

                                    hbox({
                                                 backButton->Render() | size(WIDTH, GREATER_THAN, 12),
                                                 filler(),
                                                 text("Select a month to view details") | dim
                                         }),

                                    separator(),
                                    text(message) | dim | color(Color::Green)
                            });

        return shell("All Budgets", body);
    }

    Element dashboard() {
        if (!hasBudget) {
            return vbox({
                                text("No budget created yet.") | bold | color(Color::Red),
                                text("Choose 'Create Monthly Budget' to begin.") | dim
                        }) | border;
        }

        double expected = budget->getExpectedCost();
        double spent = budget->getTotalCost();
        double remaining = expected - spent;

        float percent = 0.0;
        if (expected > 0) {
            percent = spent / expected;
        }

        Color statusColor = spent > expected ? Color::Red : Color::Green;
        std::string statusText = spent > expected ? "OVER BUDGET" : "ON TRACK";

        return vbox({
                            hbox({
                                         statCard("Month", budget->getMonth(), Color::Cyan),
                                         statCard("Budget", "$" + money(expected), Color::Green),
                                         statCard("Spent", "$" + money(spent), Color::Yellow),
                                         statCard("Remaining", "$" + money(remaining), statusColor)
                                 }),

                            separator(),

                            hbox({
                                         text("Status: ") | bold,
                                         text(statusText) | bold | color(statusColor)
                                 }),

                            text("Progress") | bold | color(Color::Yellow),
                            gauge(percent) | color(statusColor),

                            separator(),

                            text("Expenses") | bold | color(Color::Cyan),
                            expenseList() | border,

                            separator(),

                            text("Monthly Report") | bold | color(Color::Magenta),
                            monthlyReport() | border
                    });
    }

    Element statCard(const std::string& label, const std::string& value, Color colorValue) {
        return vbox({
                            text(label) | dim | center,
                            text(value) | bold | center | color(colorValue)
                    }) | border | flex;
    }

    Element monthlyReport() {
        if (!hasBudget) {
            return text("No report available.");
        }

        double expected = budget->getExpectedCost();
        double spent = budget->getTotalCost();
        double remaining = expected - spent;

        double percentUsed = 0;
        if (expected > 0) {
            percentUsed = (spent / expected) * 100;
        }

        return vbox({
                            hbox({
                                         text("Budget used: ") | bold,
                                         text(money(percentUsed) + "%") |
                                         color(percentUsed > 100 ? Color::Red : Color::Green)
                                 }),
                            hbox({
                                         text("Total expenses: ") | bold,
                                         text(std::to_string(budget->getExpenses().size())) | color(Color::Cyan)
                                 }),
                            hbox({
                                         text("Remaining money: $") | bold,
                                         text(money(remaining)) |
                                         color(remaining < 0 ? Color::Red : Color::Green)
                                 }),
                            hbox({
                                         text("Status: ") | bold,
                                         text(spent > expected ? "Over budget" : "On track")
                                         | bold
                                         | color(spent > expected ? Color::Red : Color::Green)
                                 })
                    });
    }

    Element expenseList() {
        if (!hasBudget || budget->getExpenses().empty()) {
            return text("No expenses yet.") | dim;
        }

        Elements rows;

        rows.push_back(
                hbox({
                             text("Name") | bold | size(WIDTH, EQUAL, 20),
                             text("Amount") | bold | size(WIDTH, EQUAL, 12),
                             text("Category") | bold
                     }) | color(Color::Yellow)
        );

        rows.push_back(separator());

        for (const auto& expense : budget->getExpenses()) {
            rows.push_back(
                    hbox({
                                 text(expense.getName()) | size(WIDTH, EQUAL, 20),
                                 text("$" + money(expense.getAmount()))
                                 | size(WIDTH, EQUAL, 12)
                                 | color(Color::Green),
                                 text(typeToString(expense.getType())) | color(Color::Cyan)
                         })
            );
        }

        return vbox(rows);
    }

    void loadBudgetLabels() {
        allBudgets = loadAllBudgetsJson();
        budgetLabels.clear();

        for (const auto& b : allBudgets) {
            std::string label =
                    b["month"].get<std::string>() + " " +
                    std::to_string(b["year"].get<int>()) +
                    "   $" + money(b["totalCost"].get<double>()) +
                    " / $" + money(b["expectedCost"].get<double>());

            budgetLabels.push_back(label);
        }

        if (budgetLabels.empty()) {
            budgetLabels.push_back("No budgets found.");
        }

        selectedBudget = 0;
    }

    Element renderSelectedBudgetDetails() {
        if (allBudgets.empty() || selectedBudget >= allBudgets.size()) {
            return text("No budget selected.") | dim;
        }

        const auto& b = allBudgets[selectedBudget];

        if (!b.contains("expectedCost")) {
            return text("No budget selected.") | dim;
        }

        double expected = b["expectedCost"].get<double>();
        double spent = b["totalCost"].get<double>();
        double remaining = expected - spent;

        double percentUsed = 0;
        if (expected > 0) {
            percentUsed = (spent / expected) * 100;
        }

        Color statusColor = spent > expected ? Color::Red : Color::Green;

        Elements expenseRows;

        if (b.contains("expenses") && !b["expenses"].empty()) {
            expenseRows.push_back(
                    hbox({
                                 text("Name") | bold | size(WIDTH, EQUAL, 18),
                                 text("Amount") | bold | size(WIDTH, EQUAL, 12),
                                 text("Category") | bold
                         }) | color(Color::Yellow)
            );

            expenseRows.push_back(separator());

            for (const auto& e : b["expenses"]) {
                expenseRows.push_back(
                        hbox({
                                     text(e["name"].get<std::string>()) | size(WIDTH, EQUAL, 18),
                                     text("$" + money(e["amount"].get<double>()))
                                     | size(WIDTH, EQUAL, 12)
                                     | color(Color::Green),
                                     text(typeToString(static_cast<Type>(e["type"].get<int>())))
                                     | color(Color::Cyan)
                             })
                );
            }
        } else {
            expenseRows.push_back(text("No expenses.") | dim);
        }

        return vbox({
                            text("Details") | bold | color(Color::Cyan),
                            separator(),

                            hbox({
                                         statCard("Month",
                                                  b["month"].get<std::string>() + " " + std::to_string(b["year"].get<int>()),
                                                  Color::Cyan),
                                         statCard("Budget", "$" + money(expected), Color::Green),
                                         statCard("Spent", "$" + money(spent), Color::Yellow)
                                 }),

                            hbox({
                                         statCard("Remaining", "$" + money(remaining), statusColor),
                                         statCard("Used", money(percentUsed) + "%", statusColor),
                                         statCard("Status", spent > expected ? "Over" : "Good", statusColor)
                                 }),

                            separator(),
                            text("Expenses") | bold | color(Color::Magenta),
                            vbox(expenseRows)
                    });
    }

    void deleteBudget() {
        if (!hasBudget) {
            message = "No budget exists to delete.";
            return;
        }

        delete budget;
        budget = nullptr;
        hasBudget = false;

        deleteBudgetFile();

        message = "Budget deleted.";
    }

    std::string money(double amount) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << amount;
        return out.str();
    }

    std::string typeToString(Type type) {
        switch (type) {
            case HOUSING: return "Housing";
            case FOOD: return "Food";
            case TRANSPORTATION: return "Transportation";
            case UTILITIES: return "Utilities";
            case HEALTH: return "Health";
            case ENTERTAINMENT: return "Entertainment";
            case SHOPPING: return "Shopping";
            case SAVINGS: return "Savings";
            case DEBT: return "Debt";
            case OTHER: return "Other";
            default: return "Unknown";
        }
    }
};

#endif