
#include "Interpreter.h"
#include <iostream>

/**
 * main loop for interpreter, interpret each command and call corresponding APIs
 */
void Interpreter::main_loop() {
    std::string inputCmd;
    static bool inputState = true;
    while (true) {
        std::cout << (inputState ? "MiniSQL>" : "       >");
        std::getline(std::cin, inputCmd);
        try {
            inputState = parser.inputLine(inputCmd);
        } catch (std::runtime_error &error) {
            std::cout << "[Error] " << error.what() << std::endl;
            parser.flushBuffer();
            inputState = true;
        }
    }
}
