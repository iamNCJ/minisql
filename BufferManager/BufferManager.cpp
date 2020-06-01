#include "BufferManager.h"
#include <sys/stat.h>

using namespace std;

BufferManager::BufferManager() {
    blockBuffer.resize(MaxBlocks);
    maxLRU = 0;
    int id = 0;
    for (auto &block : blockBuffer) {
        block.id = id++;
    }
}

int BufferManager::getTailBlock(string filename){
    struct stat st;
    if (!stat(filename.c_str(), &st)) {
        return st.st_size / BlockSize - 1;
    }
    cerr << "Failed to get number of blocks in file" + filename << endl;
}

void BufferManager::setDirty(const string &filename, unsigned int blockID){
    Block &block = findPair(filename, blockID);
    block.dirty = true;
}

char *BufferManager::getBlock(string filename, unsigned int offset, bool allocate = false){
    for (auto &blk: blockBuffer) {
        if (blk.filename == filename && blk.blockID == offset) {
            setBusy(blk.id);
            blockMap.insert(TYPE_BLOCK_MAP::value_type(make_pair(filename, offset), blk));
            return blk.content;
        }
    }
    
    fstream fp;
    fp.open(filename, ios::in | ios::out | ios:: binary);
    if (!fp.good()) cerr << "Fail to open:" << filename;
    fp.seekg(ios_base::end);
    int blockOffset = getTailBlock(filename) + 1;
    if (offset >= blockOffset) {
        if (!allocate) {return nullptr;}
        if (blockOffset != offset) {
            cerr << "offset too large";
            return nullptr;
        }
    }
    Block &block = getFreeBlock();
    block.bind(filename, offset);
    blockMap.insert(TYPE_BLOCK_MAP::value_type(make_pair(filename, offset), block));
    
    fp.seekg(offset * BlockSize, ios::beg);
    fp.read(block.content, BlockSize);
    fp.close();
    setBusy(block.id);
    block.flush();
    return block.content;
}

void BufferManager::flushAll(){
    for (auto &blk: blockBuffer) {
        if (blk.dirty) {
            blk.flush();
        }
        blk.reset();
    }
}

void BufferManager::createFile(string filename){
    ofstream f1(filename);
}

void BufferManager::removeFile(string filename){
    for (auto &blk: blockBuffer) {
        if (blk.filename == filename) {
            blk.reset();
        }
    }
    if (remove(filename.c_str())) cerr << "Failed to remove: " << filename;
}

void BufferManager::setBusy(int id) {
    Block &blk = blockBuffer[id];
    blk.busy = true;
    blk.LRUCnt = ++maxLRU;
}

Block &BufferManager::getLRU() {
    int max = maxLRU;
    Block *probe = nullptr;
    for (auto const &blk: blockBuffer) {
        if (blk.busy) continue;
        if (blk.LRUCnt <= max) {
            max = blk.LRUCnt;
            probe = const_cast<Block *>(&blk);
        }
    }
    if (probe) return *probe;
    else cerr << "No LRU Found";
}

Block &BufferManager::getFreeBlock() {
    for (auto &blk: blockBuffer) {
        if (!blk.dirty && !blk.busy) {
            blk.reset();
            setBusy(blk.id);
            return blk;
        }
    }

    Block &bb = getLRU();
    bb.flush().reset();
    setBusy(bb.id);

    return bb;
}

Block &BufferManager::findPair(string filename, unsigned int blockID) const {
    auto prt = blockMap.find(make_pair(filename, blockID));
    if (prt == blockMap.end()) {
        cerr << "block not found";
    }
    return prt->second;
}

void BufferManager::setFree(string filename, unsigned int blockID) {
    Block &block = findPair(filename, blockID);
    block.busy = false;
    blockMap.erase(make_pair(block.filename, block.blockID));
}