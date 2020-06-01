#ifndef MINISQL_API_H
#define MINISQL_API_H

#include <string>
#include <vector>

#include "../DataStructure.h"

namespace API {
    int insert(const std::string &tableName, const std::vector<MiniSqlBasic::SqlValue> &valueList);

    bool deleteOp(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList);

    bool select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList, const std::vector<std::string> &attrList);

    bool update(const std::string &tableName, const std::string &attr, const MiniSqlBasic::SqlValue &value, const std::vector<MiniSqlBasic::Condition> &conditionList);

    bool createTable(const std::string &tableName, const std::vector<std::pair<std::string, MiniSqlBasic::SqlValue>> &attrList, const std::string &primaryKey);

    bool createIndex(const std::string &tableName, const std::string &attrName, const std::string &indexName, bool manual = true);

    bool dropTable(const std::string &tableName);

    bool dropIndex(const std::string &indexName);
};


#endif //MINISQL_API_H
