#include <iostream>
#include <cstring>

#include "Interpreter.h"

int main() {
    Interpreter interpreter;
    char *p;
    bool singleMode = false;
    p = getenv("MINISQL_MODE");
    if(p && strcmp(p, "SINGLE") == 0) {
        singleMode = true;
    }
    if (!singleMode)
        std::cout << "Welcome to miniSQL!" << std::endl;
    interpreter.main_loop(singleMode);
}