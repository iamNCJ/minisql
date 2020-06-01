#include "API.h"

/**
 * Insert values
 * @param tableName table
 * @param valueList values
 * @return items modified
 */
int API::insert(const std::string &tableName, const std::vector<MiniSqlBasic::SqlValue> &valueList) {
    return 0;
}

/**
 * Delete
 * @param tableName table
 * @param conditionList conditions
 * @return isSuc
 */
bool API::deleteOp(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList) {
    return false;
}

/**
 * Select all
 * @param tableName table
 * @param conditionList conditions
 * @return isSuc
 */
bool API::select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList) {
    return false;
}

/**
 * Select some attr
 * @param tableName table
 * @param conditionList conditions
 * @param attrList attributes
 * @return isSuc
 */
bool API::select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList, const std::vector<std::string> &attrList) {
    return false;
}

/**
 * Update
 * @param tableName table
 * @param attr attribute
 * @param value value
 * @param conditionList conditions
 * @return isSuc
 */
bool API::update(const std::string &tableName, const std::string &attr, const MiniSqlBasic::SqlValue &value, const std::vector<MiniSqlBasic::Condition> &conditionList) {
    return false;
}

/**
 * Create Table
 * @param tableName table
 * @param attrList attributes
 * @param primaryKey primary key
 * @return isSuc
 */
bool API::createTable(const std::string &tableName, const std::vector<std::pair<std::string, MiniSqlBasic::SqlValue>> &attrList, const std::string &primaryKey) {
    return false;
}

/**
 * Create Index
 * @param tableName table
 * @param attrName attribute
 * @param indexName index name
 * @param manual is user call or is default (Primary Key)
 * @return isSuc
 */
bool API::createIndex(const std::string &tableName, const std::string &attrName, const std::string &indexName, bool manual = true) {
    return false;
}

/**
 * Drop Table
 * @param tableName table
 * @return isSuc
 */
bool API::dropTable(const std::string &tableName) {
    return false;
}

/**
 * Drop Index
 * @param indexName index name
 * @return isSuc
 */
bool API::dropIndex(const std::string &indexName) {
    return false;
}