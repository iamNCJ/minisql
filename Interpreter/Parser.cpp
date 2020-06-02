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

    if (getLower(args.at(0)) == "select") { // Select
        execSelect(args);
    } else if (getLower(args.at(0)) == "delete") { // Delete
        execDelete(args);
    } else if (getLower(args.at(0)) == "drop") { // Drop
        execDrop(args);
    } else if (getLower(args.at(0)) == "create") { // Create
        if (args.at(1) == "index") { // create index
            execCreateIndex(args);
        } else if (args.at(1) == "table") { // create table
            execCreateTable(args);
        }
    } else {
        throw std::runtime_error("SYNTAX ERROR: You have an error in your SQL syntax");
    }
}

/**
 * Execute select operation
 * @param args arguments
 */
void Parser::execSelect(const std::vector<std::string> &args) {
    std::vector<MiniSqlBasic::Condition> conditions;

    auto _cm = API::getCatalogManager();

    // everything in the world should be select * from !!!
    auto it = std::find(args.begin(), args.end(), "from");
    int distance = it - args.begin();
    std::string tableName = args[distance + 1];

    // check and get table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table " << tableName << " not found!" << std::endl;
        return;
    }
    auto &table = _cm->GetTable(tableName);

    for (int i = distance + 2, len = args.size(), j = 0; i < len; j++) {
        std::string attr = args.at(i++);
        MiniSqlBasic::Operator op;
        if (args.at(i) == "<" && args.at(i + 1) == "=") {
            op = MiniSqlBasic::Operator::LE_OP;
            i++;
        } else if (args.at(i) == "<" && args.at(i + 1) == ">") {
            op = MiniSqlBasic::Operator::NE_OP;
            i++;
        } else if (args.at(i) == ">" && args.at(i + 1) == "=") {
            op = MiniSqlBasic::Operator::GE_OP;
            i++;
        } else if (args.at(i) == "<") {
            op = MiniSqlBasic::Operator::LT_OP;
        } else if (args.at(i) == ">") {
            op = MiniSqlBasic::Operator::GT_OP;
        } else if (args.at(i) == "=") {
            op = MiniSqlBasic::Operator::EQ_OP;
        } else {
            std::cout << "You have an error in your SQL syntax" << std::endl;
            return; // TODO make it more elegent
        }
        i++;

        SqlValue val;
        try {
            // Prepare val
            SqlValueType type = table.attrType.at(j);
            val.type = type;
            switch (type.type) {
                case SqlValueTypeBase::Integer:
                    val.i = std::stoi(args.at(i));
                    break;
                case SqlValueTypeBase::Float:
                    val.r = std::stof(args.at(i));
                    break;
                case SqlValueTypeBase::String:
                    val.str = args.at(i).substr(1, args.at(i).length() - 2); // remove 2 "'"
                    break;
            }
        } catch (...) {
            std::cout << "You have an error in your SQL syntax" << std::endl;
            return; // TODO make it more elegent
        }

        Condition condition;
        condition.name = attr;
        condition.op = op;
        condition.val = val;
        conditions.push_back(condition);

        i += 2; // pass "and"
    }

    std::cout << std::endl;
    API::select(tableName, conditions);
}

/**
 * Execute delete operation
 * @param args arguments
 */
void Parser::execDelete(const std::vector<std::string> &args) {
    std::vector<MiniSqlBasic::Condition> conditions;

    auto _cm = API::getCatalogManager();

    // everything in the world should be delete from !!!
    auto it = std::find(args.begin(), args.end(), "from");
    int distance = it - args.begin();
    std::string tableName = args[distance + 1];

    // check and get table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table not " << tableName << " found!" << std::endl;
        return;
    }
    auto &table = _cm->GetTable(tableName);

    for (int i = distance + 2, len = args.size(), j = 0; i < len; j++) {
        std::string attr = args.at(i++);
        MiniSqlBasic::Operator op;
        if (args.at(i) == "<" && args.at(i + 1) == "=") {
            op = MiniSqlBasic::Operator::LE_OP;
            i++;
        } else if (args.at(i) == "<" && args.at(i + 1) == ">") {
            op = MiniSqlBasic::Operator::NE_OP;
            i++;
        } else if (args.at(i) == ">" && args.at(i + 1) == "=") {
            op = MiniSqlBasic::Operator::GE_OP;
            i++;
        } else if (args.at(i) == "<") {
            op = MiniSqlBasic::Operator::LT_OP;
        } else if (args.at(i) == ">") {
            op = MiniSqlBasic::Operator::GT_OP;
        } else if (args.at(i) == "=") {
            op = MiniSqlBasic::Operator::EQ_OP;
        } else {
            std::cout << "You have an error in your SQL syntax" << std::endl;
            return; // TODO make it more elegent
        }
        i++;

        SqlValue val;
        try {
            // Prepare val
            SqlValueType type = table.attrType.at(j);
            val.type = type;
            switch (type.type) {
                case SqlValueTypeBase::Integer:
                    val.i = std::stoi(args.at(i));
                    break;
                case SqlValueTypeBase::Float:
                    val.r = std::stof(args.at(i));
                    break;
                case SqlValueTypeBase::String:
                    val.str = args.at(i).substr(1, args.at(i).length() - 2); // remove 2 "'"
                    break;
            }
        } catch (...) {
            std::cout << "You have an error in your SQL syntax" << std::endl;
            return; // TODO make it more elegent
        }

        Condition condition;
        condition.name = attr;
        condition.op = op;
        condition.val = val;
        conditions.push_back(condition);

        i += 2; // pass "and"
    }

    std::cout << std::endl;
    API::deleteOp(tableName, conditions);
}

/**
 * Execute drop operation
 * @param args arguments
 */
void Parser::execDrop(const std::vector<std::string> &args) {
    try {
        if (args.at(1) == "table") {
            API::dropTable(args.at(2));
        } else if (args.at(1) == "index") {
            API::dropIndex(args.at(2));
        } else {
            throw std::runtime_error("SYNTAX ERROR: You have an error in your SQL syntax");
            return;
        }
    } catch (std::out_of_range) {
        throw std::runtime_error("SYNTAX ERROR: You have an error in your SQL syntax");
    }
}

/**
 * Execute create index operation
 * @param args arguments
 */
void Parser::execCreateIndex(const vector<std::string> &args) {
    try {
        std::string indexName = args.at(2);
        std::string tableName = args.at(4);
        std::string attrName = args.at(6);

        // call API
        API::createIndex(tableName, attrName, indexName, true);
    } catch (std::out_of_range) {
        throw std::runtime_error("SYNTAX ERROR: You have an error in your SQL syntax");
    }
}

/**
 * Exectute create table operation
 * @param args arguments
 */
void Parser::execCreateTable(const vector<std::string> &args) {
    try {
        std::string tableName = args.at(2);
        std::vector<std::pair<std::string, MiniSqlBasic::SqlValueType>> attrList;
        std::string primaryKey;
        bool hasPrimaryKey = false;

        // primary key
        if (args.at(args.size() - 6) == "primary" && args.at(args.size() - 5) == "key") {
            primaryKey = args.at(args.size() - 3);
        } else {
            throw std::runtime_error("SYNTAX ERROR: You must have primary key when create table!");
        }

        for (int i = 4, len = args.size(); i < len - 7;) {
            SqlValueType type;
            type.attrName = args.at(i++);
            if (args.at(i) == "int") { // Integer
                type.type = MiniSqlBasic::SqlValueTypeBase::Integer;
                i++;
            } else if (args.at(i) == "float") { // Float
                type.type = MiniSqlBasic::SqlValueTypeBase::Float;
                i++;
            } else if (args.at(i) == "char") { // Char
                type.type = MiniSqlBasic::SqlValueTypeBase::String;
                i += 2;
                type.charSize = std::stoi(args.at(i++)); // i++ to pass ")"
            }

            if (type.attrName == primaryKey) { // is primary
                hasPrimaryKey = true;
                type.primary = true;
            }
            if (args.at(i) == "unique") { // is unique
                type.unique = true;
                i++;
            }
            i++; // pass ","

            attrList.push_back(std::pair<std::string, MiniSqlBasic::SqlValueType>(type.attrName, type)); // add to attr list
        }

        if (!hasPrimaryKey) {
            throw std::runtime_error("SYNTAX ERROR: Primary key not inside attributes!");
        }
        // call API
        API::createTable(tableName, attrList, primaryKey);
    } catch (std::out_of_range) {
        throw std::runtime_error("SYNTAX ERROR: You have an error in your SQL syntax!");
    }
}

/**
 * Exectute insert operation
 * @param args arguments
 */
void Parser::execInsert(const vector<std::string> &args) {

}
