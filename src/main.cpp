#include "Application.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        Application application;
        application.run();
    } catch (const std::exception& error) {
        std::cerr << "AStarLab error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

