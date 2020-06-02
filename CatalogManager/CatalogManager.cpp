#include <fstream>
#include <algorithm>
#include "CatalogManager.h"
#include "API.h"

/**
 * Constructor
 */
CatalogManager::CatalogManager() : tables(std::vector<MiniSqlBasic::Table>()) {
    LoadFromFile();
}

/**
 * Deconstruct
 */
CatalogManager::~CatalogManager() {
    WriteToFile();
}

/**
 * Write to file
 */
void CatalogManager::WriteToFile() const {
    std::ofstream fileOut(metaFileName);
    fileOut << tables.size() << std::endl;

    for (const auto &table: tables) {
        fileOut << table.Name << std::endl;
        fileOut << table.recordCnt << std::endl;

        std::ofstream catalogFileOut(table.Name + ".catalog");
        catalogFileOut << table.attrNames.size() << std::endl;
        int i = 0;
        for (const auto &attrName: table.attrNames) {
            // attr name
            catalogFileOut << attrName << std::endl;
            // attr type
            const auto &attr = table.attrType[i];
            switch (attr.type) {
                case MiniSqlBasic::SqlValueTypeBase::Integer:
                    catalogFileOut << "int" << std::endl;
                    break;
                case MiniSqlBasic::SqlValueTypeBase::Float:
                    catalogFileOut << "float" << std::endl;
                    break;
                case MiniSqlBasic::SqlValueTypeBase::String:
                    catalogFileOut << "char" << std::endl;
                    break;
            }

            // check if is string
            if (attr.type == MiniSqlBasic::SqlValueTypeBase::String) {
                catalogFileOut << attr.charSize << std::endl;
            } else {
                catalogFileOut << 0 << std::endl;
            }
            // attr primary / unique
            catalogFileOut << (attr.primary ? 1 : 0) << std::endl;
            catalogFileOut << (attr.unique ? 1 : 0) << std::endl;
            auto index = std::find_if(table.index.begin(), table.index.end(),
                                      [&attrName](const std::pair<std::string, std::string> &p) {
                                          return p.first == attrName;
                                      });
            if (index != table.index.end()) {
                catalogFileOut << 1 << std::endl << index->second << std::endl;
            } else {
                catalogFileOut << 0 << std::endl << "-" << std::endl;
            }
            ++i;
        }
        catalogFileOut.close();
    }
    fileOut.close();
}

/**
 * Read File
 */
void CatalogManager::LoadFromFile() {
    std::ifstream fileIn(metaFileName);
    if (!fileIn.is_open()) {
        std::ofstream touch(metaFileName); // create file is not exists
        return;
    }

    int tables_count;
    fileIn >> tables_count;

    auto rm = API::getRecordManager();

    // read each table from file
    std::string tableName;
    for (auto i = 0; i < tables_count; ++i) {
        fileIn >> tableName;
        auto fileName = tableName + ".catalog";
        std::ifstream attrFileIn(fileName);

        MiniSqlBasic::Table table;
        std::vector<MiniSqlBasic::SqlValueType> indexVector;
        fileIn >> table.recordCnt;

        table.Name = tableName;

        int countsCheck = 0;
        int recordLength = 0;
        int attrCounts;
        attrFileIn >> attrCounts;
        for (auto ci = 0; ci < attrCounts; ++ci) {
            std::string attrName, typeName, indexName;
            int isPrimary, isUnique, isIndex, size;
            MiniSqlBasic::SqlValueType type;

            attrFileIn >> attrName >> typeName >> size >> isPrimary >> isUnique >> isIndex >> indexName;
            countsCheck++; // check if attr cnt correct
            table.attrNames.push_back(attrName);

            // set type
            if (typeName == "int") {
                type.type = MiniSqlBasic::SqlValueTypeBase::Integer;
            } else if (typeName == "char") {
                type.type = MiniSqlBasic::SqlValueTypeBase::String;
                type.charSize = size;
            } else if (typeName == "float") {
                type.type = MiniSqlBasic::SqlValueTypeBase::Float;
            } else {
                ; // TODO corupted file
            }
            recordLength += type.getSize();
            type.primary = isPrimary != 0;
            type.unique = isUnique != 0;
            type.attrName = attrName;
            table.attrType.push_back(type);

            // if is index
            if (isIndex) {
                auto index = std::make_pair(attrName, indexName);
                table.index.push_back(index);
                indexVector.push_back(type);
            }
        }
        table.attrCount = countsCheck;
        table.recordLength = recordLength;
        tables.push_back(table);

        // record manager
        for (auto &it: indexVector) {
            rm->createIndex(table, it);
        }
    }
}

/**
 * Create table
 * @param TableName table name string
 * @param schemaList schemas vector
 * @param PrimaryKey primary key string
 */
void CatalogManager::CreateTable(const std::string &TableName,
                                 const std::vector<std::pair<std::string, MiniSqlBasic::SqlValueType>> &schemaList,
                                 const std::string &PrimaryKey) {
    MiniSqlBasic::Table table;
    table.Name = TableName;
    table.attrCount = (int) schemaList.size();
    int len = 0;
    char autoIndex = '1';

    auto rm = API::getRecordManager();

    // Make attribute
    for (const auto &schema: schemaList) {
        len += schema.second.getSize();
        table.attrNames.push_back(schema.first);
        auto t = schema.second;
        t.attrName = schema.first;
        table.attrType.push_back(t);
        if (schema.first == PrimaryKey) {
            (table.attrType.end() - 1)->primary = true;
            (table.attrType.end() - 1)->unique = true;
        }
    }
    table.recordLength = len;
    table.recordCnt = 0;

    // Setup index
    for (auto &type: table.attrType) {
        if (type.unique && !type.primary) {
            table.index.emplace_back(std::make_pair(type.attrName, std::string("autoIndex_") + (autoIndex++)));
            rm->createIndex(table, type);
        }
    }

    // Add table
    tables.push_back(table);
}

/**
 * Check if table exists
 * @param TableName table name string
 * @return is exists
 */
bool CatalogManager::ExistTable(const std::string &TableName) const {
    return std::find_if(tables.begin(), tables.end(), [&TableName](const MiniSqlBasic::Table &table) {
        return (table.Name == TableName);
    }) != tables.end();
}

/**
 * Get a table
 * @param TableName table name string
 * @return the ref of the table
 */
MiniSqlBasic::Table &CatalogManager::GetTable(const std::string &TableName) {
    return *std::find_if(tables.begin(), tables.end(), [&TableName](const MiniSqlBasic::Table &table) {
        return (table.Name == TableName);
    });
}

/**
 * Check if index name exists
 * @param IndexName index name string
 * @return if exists
 */
bool CatalogManager::ExistIndex(const std::string &IndexName) const {
    for (const auto &table: tables) {
        for (auto &index: table.index) {
            if (index.second == IndexName) return true;
        }
    }
    return false;
}

/**
 * Get index
 * @param IndexName
 * @return table containing that index
 */
MiniSqlBasic::Table &CatalogManager::GetIndex(const std::string &IndexName) {
    for (auto &table: tables) {
        for (const auto &index: table.index) {
            if (index.second == IndexName) return table;
        }
    }
}

/**
 * Remove table
 * @param table table name string
 * @return isSuc
 */
bool CatalogManager::RemoveTable(const MiniSqlBasic::Table &table) {
    if (std::find_if(tables.begin(), tables.end(),
                     [&table](const MiniSqlBasic::Table &_table) { return _table.Name == table.Name; }) == tables.end())
        return false;
    tables.erase(std::find_if(tables.begin(), tables.end(),
                              [&table](const MiniSqlBasic::Table &_table) { return _table.Name == table.Name; }));
    return true;
}
