#ifndef BUDGETING_STORAGE_HPP
#define BUDGETING_STORAGE_HPP

#include "Budget.hpp"

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include "json.hpp"

using json = nlohmann::json;

const std::string SAVE_PATH =
        "/Users/willclore/Documents/budget_data.json";

const std::string REPORT_PATH =
        "/Users/willclore/Documents/budget_report.html";

int getCurrentYear() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    return localTime->tm_year + 1900;
}

json budgetToJson(const Budget& budget) {
    json budgetJson;

    budgetJson["month"] = budget.getMonth();
    budgetJson["year"] = getCurrentYear();
    budgetJson["expectedCost"] = budget.getExpectedCost();
    budgetJson["totalCost"] = budget.getTotalCost();
    budgetJson["expenses"] = json::array();

    for (const auto& expense : budget.getExpenses()) {
        json expenseJson;

        expenseJson["name"] = expense.getName();
        expenseJson["amount"] = expense.getAmount();
        expenseJson["type"] = expense.getType();
        expenseJson["timestamp"] = expense.getTimestamp();

        budgetJson["expenses"].push_back(expenseJson);
    }

    return budgetJson;
}

void saveBudget(const Budget& budget) {
    json root;

    std::ifstream inFile(SAVE_PATH);

    if (inFile.is_open()) {
        inFile >> root;
        inFile.close();
    }

    if (!root.contains("budgets")) {
        root["budgets"] = json::array();
    }

    json budgetJson = budgetToJson(budget);

    bool replaced = false;

    for (auto& savedBudget : root["budgets"]) {
        if (savedBudget["month"] == budget.getMonth() &&
            savedBudget["year"] == getCurrentYear()) {
            savedBudget = budgetJson;
            replaced = true;
            break;
        }
    }

    if (!replaced) {
        root["budgets"].push_back(budgetJson);
    }

    std::ofstream outFile(SAVE_PATH);
    outFile << root.dump(4);
}

Budget* loadBudget() {
    std::ifstream file(SAVE_PATH);

    if (!file.is_open()) {
        return nullptr;
    }

    json root;
    file >> root;

    if (!root.contains("budgets")) {
        return nullptr;
    }

    std::string currentMonth = getCurrentMonth();
    int currentYear = getCurrentYear();

    for (const auto& budgetJson : root["budgets"]) {
        if (budgetJson["month"] == currentMonth &&
            budgetJson["year"] == currentYear) {

            std::vector<Expense> expenses;

            if (budgetJson.contains("expenses")) {
                for (const auto& expenseJson : budgetJson["expenses"]) {
                    Expense expense(
                            expenseJson["name"],
                            expenseJson["amount"],
                            static_cast<Type>(expenseJson["type"])
                    );

                    expenses.push_back(expense);
                }
            }

            return new Budget(
                    expenses,
                    budgetJson["expectedCost"],
                    budgetJson["month"]
            );
        }
    }

    return nullptr;
}

void deleteBudgetFile() {
    std::ifstream inFile(SAVE_PATH);

    if (!inFile.is_open()) {
        return;
    }

    json root;
    inFile >> root;
    inFile.close();

    if (!root.contains("budgets")) {
        return;
    }

    std::string currentMonth = getCurrentMonth();
    int currentYear = getCurrentYear();

    json newBudgets = json::array();

    for (const auto& savedBudget : root["budgets"]) {
        if (!(savedBudget["month"] == currentMonth &&
              savedBudget["year"] == currentYear)) {
            newBudgets.push_back(savedBudget);
        }
    }

    root["budgets"] = newBudgets;

    std::ofstream outFile(SAVE_PATH);
    outFile << root.dump(4);
}

void generateHtmlReport(const Budget& budget) {
    std::ifstream inFile(SAVE_PATH);

    if (!inFile.is_open()) {
        return;
    }

    json root;
    inFile >> root;
    inFile.close();

    if (!root.contains("budgets")) {
        return;
    }

    std::ofstream file(REPORT_PATH);

    file << "<!DOCTYPE html>\n";
    file << "<html>\n<head>\n";
    file << "<title>Budget Report</title>\n";
    file << "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n";
    file << "<style>\n";
    file << "body{font-family:Arial,sans-serif;background:#f5f7fb;margin:0;padding:32px;color:#111827;}\n";
    file << ".wrap{max-width:1100px;margin:auto;background:white;border-radius:18px;padding:28px;box-shadow:0 8px 30px rgba(0,0,0,.08);}\n";
    file << ".cards{display:grid;grid-template-columns:repeat(4,1fr);gap:16px;margin:20px 0;}\n";
    file << ".card{background:#f9fafb;border:1px solid #e5e7eb;border-radius:14px;padding:16px;}\n";
    file << ".label{color:#6b7280;font-size:13px;}\n";
    file << ".value{font-size:24px;font-weight:700;margin-top:6px;}\n";
    file << "table{width:100%;border-collapse:collapse;margin-top:24px;}\n";
    file << "th,td{text-align:left;padding:10px;border-bottom:1px solid #e5e7eb;}\n";
    file << "th{background:#f9fafb;}\n";
    file << ".over{color:#dc2626;font-weight:700;}\n";
    file << ".ok{color:#16a34a;font-weight:700;}\n";
    file << "</style>\n</head>\n<body>\n";
    file << "<div class=\"wrap\">\n";
    file << "<h1>Budget Report</h1>\n";
    file << "<p>Month-by-month spending trend and budget performance.</p>\n";

    file << "<div class=\"cards\">\n";
    file << "<div class=\"card\"><div class=\"label\">Total Budgeted</div><div class=\"value\" id=\"totalBudgeted\"></div></div>\n";
    file << "<div class=\"card\"><div class=\"label\">Total Spent</div><div class=\"value\" id=\"totalSpent\"></div></div>\n";
    file << "<div class=\"card\"><div class=\"label\">Total Remaining</div><div class=\"value\" id=\"totalRemaining\"></div></div>\n";
    file << "<div class=\"card\"><div class=\"label\">Average Monthly Spend</div><div class=\"value\" id=\"avgSpent\"></div></div>\n";
    file << "</div>\n";

    file << "<canvas id=\"trendChart\" height=\"120\"></canvas>\n";

    file << "<h2>Monthly Breakdown</h2>\n";
    file << "<table>\n";
    file << "<thead><tr><th>Month</th><th>Budget</th><th>Spent</th><th>Remaining</th><th>Used</th><th>Status</th></tr></thead>\n";
    file << "<tbody id=\"tableBody\"></tbody>\n";
    file << "</table>\n";

    file << "<script>\n";

    file << "const months=[";
    bool first = true;
    for (const auto& b : root["budgets"]) {
        if (!first) file << ",";
        file << "\"" << b["month"].get<std::string>() << " " << b["year"].get<int>() << "\"";
        first = false;
    }
    file << "];\n";

    file << "const budget=[";
    first = true;
    for (const auto& b : root["budgets"]) {
        if (!first) file << ",";
        file << b["expectedCost"].get<double>();
        first = false;
    }
    file << "];\n";

    file << "const spent=[";
    first = true;
    for (const auto& b : root["budgets"]) {
        if (!first) file << ",";
        file << b["totalCost"].get<double>();
        first = false;
    }
    file << "];\n";

    file << "const remaining=budget.map((b,i)=>b-spent[i]);\n";
    file << "const used=budget.map((b,i)=>b>0?(spent[i]/b)*100:0);\n";
    file << "const money=n=>'$'+n.toLocaleString(undefined,{minimumFractionDigits:2,maximumFractionDigits:2});\n";

    file << "document.getElementById('totalBudgeted').textContent=money(budget.reduce((a,b)=>a+b,0));\n";
    file << "document.getElementById('totalSpent').textContent=money(spent.reduce((a,b)=>a+b,0));\n";
    file << "document.getElementById('totalRemaining').textContent=money(remaining.reduce((a,b)=>a+b,0));\n";
    file << "document.getElementById('avgSpent').textContent=money(spent.reduce((a,b)=>a+b,0)/spent.length);\n";

    file << "new Chart(document.getElementById('trendChart'),{\n";
    file << "type:'line',\n";
    file << "data:{labels:months,datasets:[\n";
    file << "{label:'Spent',data:spent,borderWidth:3,tension:.3},\n";
    file << "{label:'Budget',data:budget,borderWidth:2,borderDash:[6,6],tension:.3},\n";
    file << "{label:'Remaining',data:remaining,borderWidth:3,tension:.3}\n";
    file << "]},\n";
    file << "options:{responsive:true,interaction:{mode:'index',intersect:false},plugins:{tooltip:{callbacks:{label:c=>c.dataset.label+': '+money(c.raw)}}},scales:{y:{beginAtZero:true,ticks:{callback:v=>money(v)}}}}\n";
    file << "});\n";

    file << "const body=document.getElementById('tableBody');\n";
    file << "months.forEach((m,i)=>{\n";
    file << "const over=remaining[i]<0;\n";
    file << "body.innerHTML+=`<tr><td>${m}</td><td>${money(budget[i])}</td><td>${money(spent[i])}</td><td>${money(remaining[i])}</td><td>${used[i].toFixed(1)}%</td><td class='${over?'over':'ok'}'>${over?'Over Budget':'On Track'}</td></tr>`;\n";
    file << "});\n";

    file << "</script>\n</div>\n</body>\n</html>\n";
}

void openHtmlReport() {
    std::string command = "open \"" + REPORT_PATH + "\"";
    system(command.c_str());
}
json loadAllBudgetsJson() {
    std::ifstream file(SAVE_PATH);

    if (!file.is_open()) {
        return json::array();
    }

    json root;
    file >> root;

    if (!root.contains("budgets")) {
        return json::array();
    }

    return root["budgets"];
}



#endif