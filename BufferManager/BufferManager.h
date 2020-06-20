#ifndef MINISQL_BUFFERMANAGER_H
#define MINISQL_BUFFERMANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <cstdio>
#include <utility>
#include "../DataStructure.h"

using namespace std;
using namespace MiniSqlBasic;

struct Block {
    string filename;
    unsigned int blockID;
    int id;
    bool dirty;
    bool busy;
    char content[BlockSize];
    int LRUCnt;

    Block() { reset(); }

    void reset() {
        dirty = false;
        busy = false;
        memset(content, 0, BlockSize);
    }

    void bind(string _filename, unsigned int _blockID) {
        filename = _filename;
        blockID = _blockID;
    }

    Block &write() {
        fstream fp;
        fp.open(filename, ios::in | ios::out | ios::binary);
        fp.seekg(blockID * BlockSize, ios::beg);
        fp.write(content, BlockSize);
        fp.close();
        return *this;
    }
};


class BufferManager {
    typedef map<pair<string, unsigned int>, Block &> TYPE_BLOCK_MAP;
    TYPE_BLOCK_MAP blockMap;
    vector<Block> blockBuffer;

    void setBusy(int id);

    Block &getFreeBlock();

    Block &fetchBlock(string filename, unsigned int blockID) const;

    int maxLRU;
public:
    BufferManager();

    ~BufferManager() = default;

    int getTailBlock(string filename);

    void setDirty(const string &filename, unsigned int blockID);

    char *getBlock(string filename, unsigned int blockID, bool allocate = false);

    void writeAll();

    void createFile(string filename);

    Block &getLRU();

    void removeFile(string filename);

    void setFree(string filename, unsigned int blockID);
};


#endif //MINISQL_BUFFERMANAGER_H
