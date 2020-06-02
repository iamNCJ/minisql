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
 * Select *
 * @param tableName table name string
 * @param conditionList conditions
 * @return isSuc
 */
bool API::select(const std::string &tableName, const std::vector<MiniSqlBasic::Condition> &conditionList) {
    auto _cm = API::getCatalogManager();

    // check and get table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table not found!" << std::endl;
        return false;
    }
    auto &table = _cm->GetTable(tableName);

    // precheck each condition
    for (const auto &condition: conditionList) {
        auto it = std::find(table.attrNames.begin(), table.attrNames.end(), condition.name);
        if (it == table.attrNames.end()) {
            std::cerr << "Attribute in conditions mismatch!" << std::endl;
            return false;
        }
        auto type = table.attrType[it - table.attrNames.begin()];
        if (type.type != condition.val.type.type) {
            std::cerr << "Type in conditions mismatch!" << std::endl;
            return false;
        }
    }

    // call normal select
    return select(tableName, conditionList, table.attrNames);
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
    auto _rm = API::getRecordManager();
    auto _cm = API::getCatalogManager();

    // check and get table
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table not found!" << std::endl;
        return false;
    }
    auto &table = _cm->GetTable(tableName);

    // condition to cond // TODO check correctness
    auto condList = std::vector<Cond>();
    for (const auto &condition: conditionList) {
        condList.push_back(condition);
    }

    // check attr list valid
    for (auto &attr: attrList) {
        if (std::find(table.attrNames.begin(), table.attrNames.end(), attr) == table.attrNames.end()) {
            std::cerr << "Attribute mismatch!" << std::endl;
            return false;
        }
    }

    // call managers to select
    if (table.index.size() == 0 || condList.size() == 0) {
        // no index to acc || select all
        return _rm->selectRecord(table, attrList, condList);
    } else {
        for (const auto &index: table.index) {
            for (const auto &cond: condList) {
                if (index.first == cond.attr) { // if this is the index on the attr
                    IndexHint hint;
                    hint.attrName = cond.attr;
                    hint.cond = cond;
                    hint.attrType = cond.value.type.M();
                    return _rm->selectRecord(table, attrList, condList, hint); // use index to acc
                }
            }
        }
        // no acc, standard select
        return _rm->selectRecord(table, attrList, condList);
    }
}

/**
 * Create Table
 * @param tableName table
 * @param attrList attributes
 * @param primaryKey primary key
 * @return isSuc
 */
bool API::createTable(const std::string &tableName,
                      const std::vector<std::pair<std::string, MiniSqlBasic::SqlValueType>> &attrList,
                      const std::string &primaryKey) {
    auto _rm = API::getRecordManager();
    auto _cm = API::getCatalogManager();

    // check if exists
    if (_cm->ExistTable(tableName)) {
        std::cerr << "Table" << tableName << " already exists!" << std::endl;
        return false;
    }

    // check char(n) range 1~255
    for (auto &attr: attrList) {
        if (attr.second.type == SqlValueTypeBase::String) {
            if (attr.second.charSize < 1 || attr.second.charSize > 255) {
                std::cerr << "Char count out of range" << std::endl;
                return false;
            }
        }
    }

    // find primary key
    bool isPrimaryIndex = false;
    SqlValueType primaryKeyType;
    std::string indexName;
    if (!primaryKey.empty()) {
        bool primaryKeyFoundFlag = false;
        for (auto &attr: attrList) {
            if (attr.first == primaryKey) {
                primaryKeyFoundFlag = true;
                primaryKeyType = attr.second;
                primaryKeyType.unique = true;
                primaryKeyType.charSize = attr.second.getSize();
                primaryKeyType.attrName = attr.first;
                indexName = "pri_" + tableName + "_" + attr.first;
                isPrimaryIndex = true;
            }
        }
        if (!primaryKeyFoundFlag) {
            std::cerr << "Primary key not found!" << std::endl;
            return false;
        }
    }

    // call managers to create
    auto res = _rm->createTable(tableName);
    _cm->CreateTable(tableName, attrList, primaryKey);
    auto &tb = _cm->GetTable(tableName);
    if (isPrimaryIndex) {
        _rm->createIndex(tb, primaryKeyType);
        tb.index.push_back(std::make_pair(primaryKey, indexName));
    }
    std::cout << "Create table " << tableName << " success." << std::endl;
    _cm->WriteToFile();

    return res;
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
    auto _rm = API::getRecordManager();
    auto _cm = API::getCatalogManager();

    // check if index not exists
    if (_cm->ExistIndex(indexName)) {
        std::cerr << "Index name" << indexName << " exists!" << std::endl;
        return false;
    }

    // check if table exists
    if (!_cm->ExistTable(tableName)) {
        std::cerr << "Table" << tableName << " not found!" << std::endl;
        return false;
    }
    auto &table = _cm->GetTable(tableName);

    // check if there already has an index
    for (auto &index: table.index) {
        if (index.first == attrName) {
            // auto gen, not error
            if (index.second.find("autoIndex_") == 0) {
                index.second = indexName;
                std::cout << "Create index" << indexName << " success" << std::endl;
                return true;
            }
            // manually gen index, error
            std::cerr << "Index on the attribute " << attrName << " exists!" << std::endl;
            return false;
        }
    }

    // find and check attr
    SqlValueType type;
    for (int i = 0; i < table.attrNames.size(); ++i) {
        if (table.attrNames[i] == attrName) {
            if (table.attrType[i].unique) {
                type = table.attrType[i];
                type.attrName = table.attrNames[i];
                break;
            } else { // only unique attr can have index
                std::cout << "Not a unique attribute!!" << std::endl;
                return false;
            }
        }
    }

    // call managers to create index
    auto res = _rm->createIndex(table, type);
    table.index.emplace_back(attrName, indexName);
    _cm->WriteToFile();
    if (res) {
        std::cout << "Create index success" << std::endl;
        return true;
    } else {
        std::cerr << "Unknown failure!" << std::endl;
        return false;
    }
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

void API::flushAll() {
    auto bm = API::getBufferManager();
    bm->flushAll();
}