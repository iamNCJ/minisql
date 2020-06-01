#ifndef MINISQL_API_H
#define MINISQL_API_H

#include <string>
#include <vector>

namespace API {
    int insert(const std::string &tableName, std::vector<int> &valueList); // TODO SQL Value
};


#endif //MINISQL_API_H
