//
// Created by Will Clore on 5/11/26.
//

#ifndef BUDGETING_EXPENSE_HPP
#define BUDGETING_EXPENSE_HPP

#include <string>
#include <ctime>

enum Type {
    HOUSING,
    FOOD,
    TRANSPORTATION,
    UTILITIES,
    HEALTH,
    ENTERTAINMENT,
    SHOPPING,
    SAVINGS,
    DEBT,
    OTHER
};

class Expense {
private:
    std::string name;
    double amount;
    Type type;
    std::time_t timestamp;

public:
    Expense(const std::string& name, double amount, Type type)
            : name(name),
              amount(amount),
              type(type),
              timestamp(std::time(nullptr)) {}

    std::string getName() const {
        return name;
    }

    double getAmount() const {
        return amount;
    }

    Type getType() const {
        return type;
    }

    std::time_t getTimestamp() const {
        return timestamp;
    }
};

#endif //BUDGETING_EXPENSE_HPP