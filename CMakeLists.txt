cmake_minimum_required(VERSION 3.20)
project(Budgeting)

set(CMAKE_CXX_STANDARD 17)

add_executable(Budgeting
        main.cpp
        App.hpp
        Budget.hpp
        Expense.hpp
        Storage.hpp
        json.hpp
        App.hpp)

include(FetchContent)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v6.1.9
        )

FetchContent_MakeAvailable(ftxui)

target_link_libraries(Budgeting
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
        )