
#include "Interpreter.h"
#include <iostream>

/**
 * main loop for interpreter, interpret each command and call corresponding APIs
 */
void Interpreter::main_loop() {
    std::string inputCmd;
    while (true) {
        std::cout << "MiniSQL> ";
        std::getline(std::cin, inputCmd);
        parse(inputCmd);
    }
}

/**
 * Parser for each command
 * @param cmd the command to parse
 */
void Interpreter::parse(std::string &cmd) {
    if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        std::cout << "Bye!" << std::endl;
        exit(0);
    }
    std::cout << cmd << std::endl;
}
