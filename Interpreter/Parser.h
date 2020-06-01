#ifndef MINISQL_PARSER_H
#define MINISQL_PARSER_H


#include <string>
#include <deque>
#include <vector>

class Parser {
private:
    std::deque<std::string> buffer;

    static void exec(const std::vector<std::string>& args);
public:
    void inputLine(std::string line);
};


#endif //MINISQL_PARSER_H
