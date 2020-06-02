#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <string>
#include "Parser.h"
#include "API.h"
#include "../DataStructure.h"

/**
 * get lower case
 * @param str input string
 * @param pos
 * @return
 */
std::string Parser::getLower(std::string str) {
    for (int index = 0;; index++) {
        if (str[index] == ' ' || str[index] == '\0')
            break;
        else if (str[index] >= 'A' && str[index] <= 'Z')
            str[index] += 32;
    }
    return str;
}

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
            if (args.empty()) continue;
            exec(args);
        } else break;
    }

    // quit can work without ";"
    if (!buffer.empty() && buffer.front() == "quit") {
        std::cout << "Bye!" << std::endl;
        exit(0);
    }

    return buffer.empty();
}

/**
 * WriteToFile readline buffer
 */
void Parser::flushBuffer() {
    buffer.clear();
}

/**
 * execute a line
 * @param args commands
 */
void Parser::exec(const std::vector<std::string> &args) {
    // Quit
    if (args.front() == "quit") {
        std::cout << "Bye!" << std::endl;
        exit(0);
    }

    if (getLower(args[0]) == "select") { // Select
        execSelect(args);
    } else {
        throw std::runtime_error("You have an error in your SQL syntax");
    }
}

/**
 * Execute select operation
 * @param args arguments
 */
void Parser::execSelect(const std::vector<std::string> &args) {
    std::string table;
    std::vector<std::string> attrs;
    std::vector<MiniSqlBasic::Condition> conditions;

//    auto rm = API::getRecordManager();
//    auto im = API::getIndexManager();
//    auto cm = API::getCatalogManager();

    // everything in the world should be select * !!!
    auto it = std::find(args.begin(), args.end(), "from");
    int distance = it - args.begin();

    // fuck stupid sql, damn it
//    int i = 1, len = args.size();
//    try {
//        for (; i < len; i += 2) {
//            if (args.at(i) == "*") { ; // TODO get all attrs
//            } else if (args.at(i) == ",") {
//                throw std::runtime_error("You have an error in your SQL syntax");
//            } else {
//                attrs.push_back(args.at(i));
//            }
//
//            if (getLower(args.at(i + 1)) == "from") {
//                break;
//            } else if (args.at(i + 1) == ",") {
//                continue;
//            } else {
//                throw std::runtime_error("You have an error in your SQL syntax");
//            }
//        }
//        table = args.at(i + 2);
//        i += 3; // skip table name
//    } catch (std::out_of_range &out_of_range) {
//        throw std::runtime_error("You have an error in your SQL syntax");
//    }
//    if (i <= len - 1) {
//        if (getLower(args.at(i)) != "where") throw std::runtime_error("You have an error in your SQL syntax");
//        else i++;
//        for (; i < len;) {
//            std::string attr = args[i++];
//            MiniSqlBasic::Operator op;
//            if (args.at(i) == "<" && args.at(i + 1) == "=") {
//                op = MiniSqlBasic::Operator::LE_OP;
//                i++;
//            } else if (args.at(i) == "<" && args.at(i + 1) == ">") {
//                op = MiniSqlBasic::Operator::NE_OP;
//                i++;
//            } else if (args.at(i) == ">" && args.at(i + 1) == "=") {
//                op = MiniSqlBasic::Operator::GE_OP;
//                i++;
//            } else if (args.at(i) == "<") {
//                op = MiniSqlBasic::Operator::LT_OP;
//            } else if (args.at(i) == ">") {
//                op = MiniSqlBasic::Operator::GT_OP;
//            } else if (args.at(i) == "=") {
//                op = MiniSqlBasic::Operator::EQ_OP;
//            }
//            i++;
//            // check type from catalog manager
//        }
//    }
    std::cout << std::endl;
//    API::select(table, conditions, attrs);
}
