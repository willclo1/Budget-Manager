//
// Created by Will Clore on 5/11/26.
//


#ifndef BUDGETING_BUDGET_HPP
#define BUDGETING_BUDGET_HPP

#include "Expense.hpp"
#include <iostream>
#include <vector>

#include <ctime>
#include <string>

std::string getCurrentMonth() {

    std::time_t now = std::time(nullptr);

    std::tm* localTime = std::localtime(&now);

    char buffer[20];

    std::strftime(buffer, sizeof(buffer), "%B", localTime);

    return std::string(buffer);

}

class Budget{
    std::vector<Expense> expenses;
    double totalCost = 0;
    std::string month;
    double expectedCost;

public:
    Budget(std::vector<Expense> expenses, double expectedCost) {
        this->expenses = expenses;
        this->expectedCost = expectedCost;
        this->month = getCurrentMonth();

        for (auto expense : expenses) {
            this->totalCost += expense.getAmount();
        }
    }
    Budget(std::vector<Expense> expenses, double expectedCost, std::string month) {
        this->expenses = expenses;
        this->expectedCost = expectedCost;
        this->month = month;

        for (auto expense : expenses) {
            this->totalCost += expense.getAmount();
        }
    }

    const std::vector<Expense> &getExpenses() const {
        return expenses;
    }

    void setExpenses(const std::vector<Expense> &expenses) {
        Budget::expenses = expenses;
    }

    double getTotalCost() const {
        return totalCost;
    }

    void setTotalCost(double totalCost) {
        Budget::totalCost = totalCost;
    }

    const std::string &getMonth() const {
        return month;
    }

    void setMonth(const std::string &month) {
        Budget::month = month;
    }

    double getExpectedCost() const {
        return expectedCost;
    }

    void setExpectedCost(double expectedCost) {
        Budget::expectedCost = expectedCost;
    }

    void addExpense(const Expense& expense) {
        expenses.push_back(expense);
        totalCost += expense.getAmount();
    }






};


#endif //BUDGETING_BUDGET_HPP
