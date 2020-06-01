#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <string>
#include "Parser.h"

/**
 * send a line into parser
 * @param line a line of cmd
 * @return is this line finished
 */
bool Parser::inputLine(std::string line) {
    // add space after */=/,/()/<>/;
    for (int pos = 0; pos < line.length(); pos++) {
        if (line[pos] == '*' || line[pos] == '=' || line[pos] == ',' || line[pos] == '(' || line[pos] == ')' ||
            line[pos] == '<' || line[pos] == '>' || line[pos] == ';') {
            if (line[pos - 1] != ' ')
                line.insert(pos++, " ");
            if (line[pos + 1] != ' ')
                line.insert(++pos, " ");
        }
    }

    // string.split()
    std::istringstream iss(line);
    std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
                                     std::istream_iterator<std::string>());

    // insert into buffer
    buffer.insert(buffer.end(), results.begin(), results.end());

    // check for ";"
    while (true) {
        auto it = std::find(buffer.begin(), buffer.end(), ";");
        if (it != buffer.end()) {
            int distance = it - buffer.begin();
            std::vector<std::string> args;
            for (int i = 0; i < distance; i++) {
                args.push_back(buffer.front());
                buffer.pop_front();
            }
            buffer.pop_front(); // pop ";"
            exec(args);
        } else break;
    }

    // quit can work without ";"
    if (buffer.front() == "quit") {
        std::cout << "Bye!" << std::endl;
        exit(0);
    }

    return buffer.empty();
}

/**
 * execute a line
 * @param args commands
 */
void Parser::exec(const std::vector<std::string> &args) {
    // quit
    if (args.front() == "quit") {
        std::cout << "Bye!" << std::endl;
        exit(0);
    }

    for (auto &arg : args) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
}
