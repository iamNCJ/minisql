#ifndef MINISQL_CATALOGMANAGER_H
#define MINISQL_CATALOGMANAGER_H

#include "../DataStructure.h"

#include <string>
#include <vector>
#include <map>

class CatalogManager {
public:
    CatalogManager();

    ~CatalogManager();

    void CreateTable(const std::string &TableName,
                     const std::vector<std::pair<std::string, MiniSqlBasic::SqlValueType>> &schemaList,
                     const std::string &PrimaryKey);

    bool ExistTable(const std::string &table) const;

    MiniSqlBasic::Table &GetTable(const std::string &table);

    bool ExistIndex(const std::string &IndexName) const;

    MiniSqlBasic::Table &GetIndex(const std::string &IndexName);

    bool RemoveTable(const MiniSqlBasic::Table &_table);

    void WriteToFile() const;

private:
    void LoadFromFile();

private:
    std::vector<MiniSqlBasic::Table> tables;

    static constexpr auto metaFileName = "tables.meta";
};


#endif //MINISQL_CATALOGMANAGER_H
