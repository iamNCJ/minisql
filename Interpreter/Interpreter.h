//
// Created by NCJ on 5/8/2020.
//

#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H

#include <string>

/**
 * Class for interpreter
 */
class Interpreter {
public:
    void main_loop [[noreturn]]();

private:
    static void parse(std::string &cmd);
};


#endif //MINISQL_INTERPRETER_H
