
#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H

#include <string>
#include "Parser.h"

/**
 * Class for interpreter
 */
class Interpreter {
public:
    Interpreter() = default;
    void main_loop [[noreturn]]();

private:
    Parser parser;
};


#endif //MINISQL_INTERPRETER_H
