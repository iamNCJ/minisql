#ifndef MINISQL_RECORDMANAGER_H
#define MINISQL_RECORDMANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include "../DataStructure.h"
#include "../BufferManager/BufferManager.h"


using namespace std;
using namespace MiniSqlBasic;

typedef int IndexManager; //TODO implement IM

class RecordManager {
public:
    RecordManager(BufferManager *bm, IndexManager *im) : bm(bm), im(im) {}

    ~RecordManager() = default;

    void createTable(const string &table);

    void dropTable(const string &table);

    void createIndex(const Table &table, const SqlValueType &index);

    void dropIndex(const Table &table, const string &index);

    int insertRecord(const Table &table, const Tuple &record);

    int selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond);

    int selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond, const IndexHint &indexHint, bool printResult = true);

    bool deleteRecord(const Table &table, const vector<Cond> &cond);

private:
    BufferManager *bm;
    IndexManager *im;

    void dumpResult(const Result &res) const;

    bool condsTest(const std::vector<Cond> &conds, const Tuple &tup, const std::vector<std::string> &attr);

    void convertToTuple(const char *blockBuffer, int offset, const std::vector<SqlValueType> &attrType, Tuple &tup);
};


#endif //MINISQL_RECORDMANAGER_H
