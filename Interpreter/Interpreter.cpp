
#include "Interpreter.h"
#include "Parser.h"
#include <iostream>
#include <sstream>

/**
 * main loop for interpreter, interpret each command and call corresponding APIs
 */
void Interpreter::main_loop() {
    std::string inputCmd;
    while (true) {
        std::cout << "MiniSQL>>";
        std::getline(std::cin, inputCmd);
        parser.inputLine(inputCmd);
    }
}
