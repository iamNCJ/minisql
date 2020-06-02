#ifndef MINISQL_API_H
#define MINISQL_API_H

#include <string>
#include <vector>
#include <CatalogManager.h>

#include "../DataStructure.h"
#include "../RecordManager/RecordManager.h"

namespace API {
    int insert(const std::string &tableName, std::vector<MiniSqlBasic::SqlValue> &valueList);

    bool deleteOp(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList);

    bool select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList);

    bool select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList,
                const std::vector<std::string> &attrList);

    bool createTable(const std::string &tableName,
                     const std::vector<std::pair<std::string, MiniSqlBasic::SqlValueType>> &attrList,
                     const std::string &primaryKey);

    bool createIndex(const std::string &tableName, const std::string &attrName, const std::string &indexName,
                     bool manual = true);

    bool dropTable(const std::string &tableName);

    bool dropIndex(const std::string &item);

    RecordManager *getRecordManager();

    IndexManager *getIndexManager();

    BufferManager *getBufferManager();

    CatalogManager *getCatalogManager();

    static RecordManager *rm;
    static IndexManager *im;
    static BufferManager *bm;
    static CatalogManager *cm;
};


#endif //MINISQL_API_H
