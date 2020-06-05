
#include "Interpreter.h"
#include <iostream>

/**
 * main loop for interpreter, interpret each command and call corresponding APIs
 */
void Interpreter::main_loop(bool singleMode) {
    std::string inputCmd;
    static bool inputState = true;
    while (true) {
        if (!singleMode)
            std::cout << (inputState ? "MiniSQL> " : "       > ");
        std::getline(std::cin, inputCmd);
        try {
            inputState = parser.inputLine(inputCmd);
        } catch (std::runtime_error &error) {
            std::cout << "[Error] " << error.what() << std::endl;
            parser.flushBuffer();
            inputState = true;
        }
        if (singleMode) {
            exit(0);
        }
    }
}
