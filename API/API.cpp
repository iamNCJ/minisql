#include "API.h"
#include "../DataStructure.h"
#include <algorithm>

/**
 * Insert values
 * @param tableName table
 * @param valueList values
 * @return items modified
 */
int API::insert(const std::string &tableName, std::vector<MiniSqlBasic::SqlValue> &valueList) {
    auto _cm = API::getCatalogManager();

    // Check table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "No such table!" << std::endl;
        return 0;
    }

    // Get table
    auto &table = _cm->GetTable(tableName);
    if (table.attrNames.size() != valueList.size()) {
        std::cerr << "Number of values not fit, expected " << table.attrNames.size() << ", got " << valueList.size()
                  << std::endl;
        return 0;
    }

    // Prepare value
    for (int i = 0, len = valueList.size(); i < len; i++) {
        if (valueList.at(i).type.type != table.attrType.at(i).type) {
            if (valueList.at(i).type.type == SqlValueTypeBase::Integer &&
                table.attrType.at(i).type == SqlValueTypeBase::Float) {
                valueList.at(i).type.type = SqlValueTypeBase::Float;
                valueList.at(i).r = float(valueList.at(i).i);
            } else {
                std::cerr << "Type Error!" << std::endl;
                return 0;
            }
        }
        valueList.at(i).type.attrName = table.attrNames.at(i);
        if (valueList.at(i).type.type == SqlValueTypeBase::String) {
            if (valueList.at(i).str.length() > table.attrType.at(i).charSize) {
                std::cerr << "String too long! Max " << table.attrType.at(i).charSize << ", got "
                          << valueList.at(i).str.length();
                return 0;
            }
            valueList.at(i).type.charSize = table.attrType.at(i).charSize;
        }
    }

    // Assert unique
    IndexHint uniqueTest;
    std::vector<Cond> conditions;
    conditions.emplace_back();
    Cond &condition = conditions[0];
    condition.cond = MINISQL_COND_EQUAL;

    for (auto &index : table.index) {
        for (auto &value : valueList) {
            if (value.type.attrName != index.second) { continue; }
            condition.value = value;
            condition.attr = value.type.attrName;
            uniqueTest.attrName = condition.attr;
            uniqueTest.cond = condition;
            uniqueTest.attrType = condition.value.type.M();
            if (rm->selectRecord(table, table.attrNames, conditions, uniqueTest, false)) {
                std::cerr << "Insert failed. Duplicate key!" << std::endl;
                return 0;
            }
        }
    }

    Tuple t;
    t.element = valueList;
    auto offset = rm->insertRecord(table, t);

    for (const auto &idx: table.index) {
        auto it = std::find(table.attrNames.begin(), table.attrNames.end(), idx.first);
        im->insert(indexFile(table.Name, idx.first),
                   valueList[it - table.attrNames.begin()], offset);
    }

    ++table.recordCnt;
    std::cout << "Insert finished. 1 row affected." << std::endl;
    return 1;
}

/**
 * Delete
 * @param tableName table
 * @param conditionList conditions
 * @return isSuc
 */
bool API::deleteOp(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList) {
    auto _rm = API::getRecordManager();
    auto _cm = API::getCatalogManager();

    // check and get table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table not found!" << std::endl;
        return false;
    }
    auto &table = _cm->GetTable(tableName);

    // handle conditions
    for (const auto &cond: conditionList) {
        // find attr
        auto it = std::find(table.attrNames.begin(), table.attrNames.end(), cond.name);
        if (it == table.attrNames.end()) {
            std::cerr << "Attribute in conditions mismatch!" << std::endl;
            return false;
        }
        // check type
        auto type = table.attrType[it - table.attrNames.begin()];
        if (type.type != cond.val.type.type) {
            std::cerr << "Type in conditions mismatch!" << std::endl;
            return false;
        }
    }

    // prepare conditions
    auto condList = std::vector<Cond>();
    for (const auto &it: conditionList) {
        condList.push_back(it);
    }

    // call record manager
    auto r = _rm->deleteRecord(table, condList);
    if (r) {
        std::cout << "Delete success" << std::endl;
    } else {
        std::cerr << "Delete failed!" << std::endl;
    }
    return r;
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
    auto _rm = API::getRecordManager();
    auto _cm = API::getCatalogManager();

    // find and get table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table not found!" << std::endl;
        return false;
    }
    auto &table = _cm->GetTable(tableName);

    for (auto &index: table.index) {
        auto ifn = index.first;
        _rm->dropIndex(table, ifn);
    }

    // call managers to drop table
    _cm->RemoveTable(table);
    _cm->WriteToFile();
    std::cout << "Table " << tableName << " dropped." << std::endl;
    return _rm->dropTable(tableName);
}

/**
 * Drop Index
 * @param indexName index name
 * @return isSuc
 */
bool API::dropIndex(const std::string &indexName) {
    std::string tableName;
    auto _rm = API::getRecordManager();
    auto _cm = API::getCatalogManager();

    // find and get index
    bool index = _cm->ExistIndex(indexName);
    if (!index) {
        std::cerr << "Index not found!" << std::endl;
        return false;
    }
    auto &table = _cm->GetIndex(indexName);

    // remove index
    for (auto &idx: table.index) {
        if (idx.second == indexName) {
            _rm->dropIndex(table, idx.first);
            table.index.erase(std::find_if(table.index.begin(), table.index.end(),
                                           [&indexName](const std::pair<std::string, std::string> &item) {
                                               return item.second == indexName;
                                           }));
            std::cout << "Index " << indexName << " dropped." << std::endl;
            _cm->WriteToFile();
            return true;
        }
    }
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