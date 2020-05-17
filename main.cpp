#include <iostream>

#include "Interpreter.h"

int main() {
    std::cout << "Welcome to miniSQL!" << std::endl;
    Interpreter interpreter;
    interpreter.main_loop();
}