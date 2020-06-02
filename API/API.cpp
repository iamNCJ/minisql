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
 * Select some attr
 * @param tableName table
 * @param conditionList conditions
 * @param attrList attributes
 * @return isSuc
 */
bool API::select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList,
                 const std::vector<std::string> &attrList) {
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
bool API::update(const std::string &tableName, const std::string &attr, const MiniSqlBasic::SqlValue &value,
                 const std::vector<MiniSqlBasic::Condition> &conditionList) {
    return false;
}

/**
 * Create Table
 * @param tableName table
 * @param attrList attributes
 * @param primaryKey primary key
 * @return isSuc
 */
bool API::createTable(const std::string &tableName,
                      const std::vector<std::pair<std::string, MiniSqlBasic::SqlValue>> &attrList,
                      const std::string &primaryKey) {
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
bool
API::createIndex(const std::string &tableName, const std::string &attrName, const std::string &indexName, bool manual) {
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

/**
 * Get record manager
 * @return pointer to record manager
 */
RecordManager *API::getRecordManager() {
    if (rm == nullptr) {
        auto _im = getIndexManager();
        auto _bm = getBufferManager();
        rm = new RecordManager(_bm, _im);
    }
    return rm;
}

/**
 * Get index manager
 * @return pointer to index manager
 */
IndexManager *API::getIndexManager() {
    return (im == nullptr) ? im = new IndexManager() : im;
}

/**
 * get buffer manager
 * @return pointer to buffer manager
 */
BufferManager *API::getBufferManager() {
    return (bm == nullptr) ? bm = new BufferManager() : bm;
}

/**
 * get catalog manager
 * @return catalog manager
 */
CatalogManager *API::getCatalogManager() {
    return (cm == nullptr) ? cm = new CatalogManager() : cm;
}