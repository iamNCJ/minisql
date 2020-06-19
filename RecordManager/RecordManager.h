#ifndef MINISQL_RECORDMANAGER_H
#define MINISQL_RECORDMANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include "../DataStructure.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"


using namespace std;
using namespace MiniSqlBasic;

class RecordManager {
private:
    BufferManager *bm;
    IndexManager *im;

    bool validCheck(const std::vector<Cond> &conds, const Tuple &tu, const std::vector<std::string> &attr);

    void readBlock(const char *blockBuffer, int offset, const std::vector<SqlValueType> &attrType, Tuple &tup);

    void print(const Result &res) const;

public:
    RecordManager(BufferManager *bm, IndexManager *im) : bm(bm), im(im) {}

    ~RecordManager() = default;

    bool createTableFile(const string &table);

    bool createIndex(const Table &table, const SqlValueType &index);

    bool dropIndex(const Table &table, const string &index);

    bool deleteRecord(const Table &table, const vector<Cond> &cond);

    bool dropTableFile(const string &table);

    int insertRecord(const Table &table, const Tuple &record);

    int selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond);

    int selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond, const IndexedAttrList &indexHint,
                     bool printResult = true);
};


#endif //MINISQL_RECORDMANAGER_H
