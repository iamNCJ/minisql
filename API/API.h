//
// Created by NCJ on 5/8/2020.
//

#ifndef MINISQL_API_H
#define MINISQL_API_H

#include <string>
#include <vector>

namespace API {
    bool useDatabase(const std::string &dbName);
    int insert(const std::string &tableName, std::vector<int> &valueList); // TODO SQL Value
};


#endif //MINISQL_API_H
