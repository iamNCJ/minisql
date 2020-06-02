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

    static void execDrop(const std::vector<std::string> &args);

    static void execCreateIndex(const std::vector<std::string> &args);

    static void execCreateTable(const std::vector<std::string> &args);

    static void execInsert(const std::vector<std::string> &args);

    static void execFile(const std::string &fileName);

public:
    Parser() = default;

    bool inputLine(std::string line);

    void flushBuffer();
};

#endif //MINISQL_PARSER_H
