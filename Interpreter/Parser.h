#ifndef MINISQL_PARSER_H
#define MINISQL_PARSER_H


#include <string>
#include <deque>
#include <vector>

/**
 * Grammar Parser inside Interpreter
 */
class Parser {
private:
    std::deque<std::string> buffer;

    static void exec(const std::vector<std::string> &args);

    static std::string getLower(std::string str);

    static void execSelect(const std::vector<std::string> &args);

    static void execDelete(const std::vector<std::string> &args);


public:
    Parser() = default;

    bool inputLine(std::string line);

    void flushBuffer(void);
};

#endif //MINISQL_PARSER_H
