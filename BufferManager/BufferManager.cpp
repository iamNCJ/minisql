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

/**
 * Get the last block index in a certain file
 * @param filename
 * @return
 */
int BufferManager::getTailBlock(string filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) == 0) {
        return st.st_size / BlockSize - 1;
    }
    cerr << "Failed to get number of blocks in file " + filename << endl;
}

/**
 * set the block to dirty
 * @param filename
 * @param blockID
 */
void BufferManager::setDirty(const string &filename, unsigned int blockID) {
    Block &block = fetchBlock(filename, blockID);
    block.dirty = true;
}

/**
 * Get certain block in a file
 * @param filename
 * @param offset the offset of the block in the file
 * @param allocate if true, then append new space in the file
 * @return
 */
char *BufferManager::getBlock(string filename, unsigned int offset, bool allocate) {
    for (auto &blk: blockBuffer) {
        if (blk.filename == filename && blk.blockID == offset) {
            setBusy(blk.id);
            blockMap.insert(TYPE_BLOCK_MAP::value_type(make_pair(filename, offset), blk));
            return blk.content;
        }
    }

    fstream fp;
    fp.open(filename, ios::in | ios::out | ios::binary);
    if (!fp.good()) cerr << "Fail to open: " << filename << endl;
    fp.seekg(ios_base::end);
    int blockOffset = getTailBlock(filename) + 1;
    if (offset >= blockOffset) {
        if (!allocate) { return nullptr; }
        if (blockOffset != offset) {
            cerr << "offset too large";
            return nullptr;
        }
    }

    // Create a new block and update the file info
    Block &block = getFreeBlock();
    block.bind(filename, offset);
    blockMap.insert(TYPE_BLOCK_MAP::value_type(make_pair(filename, offset), block));

    fp.seekg(offset * BlockSize, ios::beg);
    fp.read(block.content, BlockSize);
    fp.close();
    setBusy(block.id);
    block.write();
    return block.content;
}

/**
 * Write all in memory blocks to file
 */
void BufferManager::writeAll() {
    for (auto &blk: blockBuffer) {
        if (blk.dirty) {
            blk.write();
        }
        blk.reset();
    }
}

/**
 * Create a new file
 * @param filename
 */
void BufferManager::createFile(string filename) {
    ofstream f1(filename);
}

/**
 * Remove a file
 * @param filename
 */
void BufferManager::removeFile(string filename) {
    for (auto &blk: blockBuffer) {
        if (blk.filename == filename) {
            blk.reset();
        }
    }
    if (remove(filename.c_str())) cerr << "Failed to remove: " << filename;
}

/**
 * Set the block to busy
 * @param id
 */
void BufferManager::setBusy(int id) {
    Block &blk = blockBuffer[id];
    blk.busy = true;
    blk.LRUCnt = ++maxLRU;
}

/**
 * Get the least used unit
 * @return
 */
Block &BufferManager::getLRU() {
    int max = blockBuffer[0].LRUCnt;
    Block *probe = nullptr;
    for (auto & blk : blockBuffer) {
        if (blk.busy) continue;
        if (blk.LRUCnt <= max) {
            max = blk.LRUCnt;
            probe = const_cast<Block *>(&blk);
        }
    }
    if (probe) return *probe;
    else cerr << "No LRU Found";
}

/**
 * Init an new empty block
 * @return
 */
Block &BufferManager::getFreeBlock() {
    for (auto &blk: blockBuffer) {
        if (!blk.dirty && !blk.busy) {
            blk.reset();
            setBusy(blk.id);
            return blk;
        }
    }

    Block &newBlock = getLRU();
    newBlock.write().reset();
    setBusy(newBlock.id);

    return newBlock;
}

/**
 * Find the certain block in file
 * @param filename
 * @param blockID
 * @return
 */
Block &BufferManager::fetchBlock(string filename, unsigned int blockID) const {
    auto prt = blockMap.find(make_pair(filename, blockID));
    if (prt == blockMap.end()) {
        cerr << "block not found";
    }
    return prt->second;
}

/**
 * Set a block to empty
 * @param filename
 * @param blockID
 */
void BufferManager::setFree(string filename, unsigned int blockID) {
    Block &block = fetchBlock(filename, blockID);
    block.busy = false;
    blockMap.erase(make_pair(block.filename, block.blockID));
}