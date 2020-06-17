#ifndef MINISQL_INDEXMANAGER_H
#define MINISQL_INDEXMANAGER_H

#include <string>
#include <map>
#include "../DataStructure.h"
#include "../BPTree.h"

using namespace std;
using namespace MiniSqlBasic;

class IndexManager {
public:
    IndexManager() = default;
    // fixme!!! destructor needs to be implemented to release memory of B+Tree instances
    ~IndexManager();

    bool create(const string &filename, const SqlValueType &type);
    bool drop(const string &filename, const SqlValueType &type);
    int search(const string &filename, const Element &e);
    int searchHead(const string &filename, int attrType);
    int searchNext(const string &filename, int attrType);
    bool finishSearch(const string &filename, int attrType);
    bool insert(const string &filename, const Element &e, int offset);
    bool removeKey(const string &filename, const Element &e);

private:
    map<string, BPTree<int> *> intIndexMap;
    map<string, BPTree<float> *> floatIndexMap;
    map<string, BPTree<string> *> charIndexMap;
    map<string, NodeSearchParse<int>> intOffsetMap;
    map<string, NodeSearchParse<float>> floatOffsetMap;
    map<string, NodeSearchParse<string>> charOffsetMap;
};


#endif //MINISQL_INDEXMANAGER_H
