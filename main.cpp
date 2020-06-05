#include <iostream>
#include <cstring>

#include "Interpreter.h"

int main() {
    std::cout << "Welcome to miniSQL!" << std::endl;
    Interpreter interpreter;
    char *p;
    bool singleMode = false;
    p = getenv("MINISQL_MODE");
    if(p && strcmp(p, "SINGLE") == 0) {
        singleMode = true;
    }
    interpreter.main_loop(singleMode);
}