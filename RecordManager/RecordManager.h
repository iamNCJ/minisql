#ifndef MINISQL_RECORDMANAGER_H
#define MINISQL_RECORDMANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include "../DataStructure.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"


using namespace std;
using namespace MiniSqlBasic;

class RecordManager {
public:
    RecordManager(BufferManager *bm, IndexManager *im) : bm(bm), im(im) {}

    ~RecordManager() = default;

    bool createTable(const string &table);

    bool dropTable(const string &table);

    bool createIndex(const Table &table, const SqlValueType &index);

    bool dropIndex(const Table &table, const string &index);

    int insertRecord(const Table &table, const Tuple &record);

    int selectRecord(const Table &table, const vector<string> &attr, const vector<Condition> &cond);

    int selectRecord(const Table &table, const vector<string> &attr, const vector<Condition> &cond, const IndexHint &indexHint, bool printResult = true);

    bool deleteRecord(const Table &table, const vector<Condition> &cond);

private:
    BufferManager *bm;
    IndexManager *im;

    void dumpResult(const Result &res) const;

    bool condsTest(const std::vector<Condition> &conds, const Tuple &tup, const std::vector<std::string> &attr);

    void convertToTuple(const char *blockBuffer, int offset, const std::vector<SqlValueType> &attrType, Tuple &tup);
};


#endif //MINISQL_RECORDMANAGER_H
