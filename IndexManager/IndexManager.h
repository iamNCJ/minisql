#ifndef MINISQL_INDEXMANAGER_H
#define MINISQL_INDEXMANAGER_H

#include <string>
#include <map>
#include <memory>
#include "../DataStructure.h"
#include "../BPTree.h"

using namespace std;
using namespace MiniSqlBasic;

class IndexManager {
public:
    IndexManager() = default;
    // with shared_ptr, B+ trees are freed automatically, upon destruction of maps.

    void create(const string &filename, const SqlValueType &type);
    void drop(const string &filename, const SqlValueType &type);
    int search(const string &filename, const Element &e);
    int searchHead(const string &filename, int attrType);
    int searchNext(const string &filename, int attrType);

    bool insert(const string &filename, const Element &e, int offset);
    bool removeKey(const string &filename, const Element &e);

private:
    map<string, std::shared_ptr<BPTree<int>>> intIndexMap;
    map<string, std::shared_ptr<BPTree<float>>> floatIndexMap;
    map<string, std::shared_ptr<BPTree<string>>> charIndexMap;
    map<string, NodeSearchParse<int>> intOffsetMap;
    map<string, NodeSearchParse<float>> floatOffsetMap;
    map<string, NodeSearchParse<string>> charOffsetMap;
};


#endif //MINISQL_INDEXMANAGER_H
