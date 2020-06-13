#include "RecordManager.h"
/**
 * To create a file recording table info and data
 * @param table
 * @return
 */
bool RecordManager::createTableFile(const string &table) {
    bm->createFile(table + ".tb");
    return true;
}

/**
 * remove the table file
 * @param table
 * @return
 */
bool RecordManager::dropTableFile(const string &table) {
    bm->removeFile(table + ".tb");
    return true;
}

/**
 * Create a new index on an attr of very table
 * @param table
 * @param index
 * @return
 */
bool RecordManager::createIndex(const Table &table, const SqlValueType &index) {
    string indexFileStr = table.Name + "_" + index.attrName + ".ind";
    // Create corresponding files
    bm->createFile(indexFileStr);
    im->create(indexFileStr, index);

    //Compute offset of the indexed attr in an record
    Element attr;
    int offset = 1;
    for (auto const &attrType : table.attrType) {
        if (attrType.attrName == index.attrName) {
            attr.type = attrType;
            break;
        }
        offset += attrType.getSize();
    }

    //Initiate some important number
    int blockID = 0; // Indicate the index in the file
    int length = table.recordLength + 1; // Indicate length of a record
    int recordsPerBlock = BlockSize / length; // Records in a block

    char *block = bm->getBlock(table.Name + ".tb", blockID); // Scan through all blocks
    const char *dest;
    while (block) {
        for (int i = 0; i < recordsPerBlock; i++) {
            if (block[i * length] != Used) { continue; }
            dest = block + i * length + offset; // Move the pointer to right place
            switch (attr.M()) {
                case MINISQL_TYPE_INT:
                    memcpy(&attr.i, dest, attr.type.getSize());
                    break;
                case MINISQL_TYPE_FLOAT:
                    memcpy(&attr.r, dest, attr.type.getSize());
                    break;
                case MINISQL_TYPE_CHAR:
                    attr.str.replace(0, attr.type.getSize(), dest);
                    break;
                default:
                    cerr << "Undefined type in RM::createIndex." << endl;
            }
            // Call index manager
            im->insert(indexFileStr, attr, blockID * recordsPerBlock + i);
        }
        // Free the block approving further access
        bm->setFree(table.Name + ".tb", blockID);
        blockID++;
        block = bm->getBlock(table.Name + ".tb", blockID);
    }
    return true;
}

/**
 * drop index from a table
 * @param table
 * @param index
 * @return
 */
bool RecordManager::dropIndex(const Table &table, const string &index) {
    string indexFileStr = table.Name + "_" + index + ".ind";
    bm->removeFile(indexFileStr);

    bool foundAttr = false;
    for (auto &attr : table.attrType) {
        if (attr.attrName == index) {
            foundAttr = true;
            im->drop(indexFileStr, attr); // Call the index manager to delete corresponding files and info
            break;
        }
    }
    if (!foundAttr) {
        cerr << "Drop index on undefined attr!" << endl;
    }
}

/**
 * insert a record into a certain table
 * @param table
 * @param record
 * @return
 */
int RecordManager::insertRecord(const Table &table, const Tuple &record) {
    string tableName = table.Name + ".tb";
    int tailBlockID = bm->getTailBlock(tableName); // Get the last block index in the table file
    char *content; // Pointer to the content
    if (tailBlockID >= 0) {
        content = bm->getBlock(tableName, tailBlockID);
    } else {
        tailBlockID = 0;
        content = bm->getBlock(tableName, tailBlockID, true);
    }

    int len =  table.recordLength + 1; // Length of a record
    int rcdPerBlock = BlockSize / len; // Number of record in each block

    bool validWord = false;
    int rcdOffset = 0; // To compute the target offset of the newly inserted record
    while (rcdOffset < rcdPerBlock) {
        if (content[rcdOffset * len] == Used) {
            rcdOffset++; // Unused region found in the block
        } else {
            validWord = true;
            content += rcdOffset * len;
            break;
        }
    }
    // If the block is full then get the next block
    if (!validWord) {
        rcdOffset = 0;
        bm->setFree(tableName, tailBlockID);
        content = bm->getBlock(tableName, ++tailBlockID, true);
    }

    int offset = 1; // Storing the pointer inside one record
    string fixedStr;
    for (auto attr = record.element.begin(); attr < record.element.end(); attr++) {
        switch (attr->type.M()) {
            case MINISQL_TYPE_CHAR:
                fixedStr = attr->str;
                if (attr->type.charSize > fixedStr.length()) {
                    fixedStr += '\0';
                }
                memcpy(content + offset, fixedStr.c_str(), attr->type.charSize + 1);
                offset += attr->type.charSize + 1;
                break;
            case MINISQL_TYPE_INT:
                memcpy(content + offset, &attr->i, sizeof(int));
                offset += sizeof(int);
                break;
            case MINISQL_TYPE_FLOAT:
                memcpy(content + offset, &attr->r, sizeof(float));
                offset += sizeof(float);
                break;
            default:
                std::runtime_error("Invalid Type");
        }
    }
    content[0] = Used; // Mark this region as used
    bm->setDirty(tableName, tailBlockID); // Set the block to dirty
    bm->setFree(tableName, tailBlockID); // Free the block and approve further access
    return tailBlockID * rcdPerBlock + rcdOffset; // Return the offset of the record in the whole file
}

/**
 * select from a record with constraints on unindexed attr
 * @param table
 * @param attr
 * @param cond
 * @return
 */
int RecordManager::selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond) {
    int length = table.recordLength + 1;
    int rcdPerBlock = BlockSize / length;
    Tuple tup;
    Result res;

    int blockID = 0;
    char *block = bm->getBlock(table.Name + ".tb", blockID);
    Row row;
    //Scan all the records to find the matching ones
    while (block) {
        for (int i = 0; i < rcdPerBlock; i++) {
            if (block[i * length] != Used) { continue; } // Skip the unused regions
            readBlock(block, i * length, table.attrType, tup);
            if (validCheck(cond, tup, table.attrNames)) {
                row = tup.fetchRow(table.attrNames, attr);
                res.row.push_back(row);
            }
        }
        bm->setFree(table.Name + ".tb", blockID);
        blockID++;
        block = bm->getBlock(table.Name + ".tb", blockID);
    }

    print(res);
    return res.row.size();
}

/**
 * Select record with constraints on indexed attr
 * @param table
 * @param attr projected attrs
 * @param cond constrains list
 * @param indexHint corresponding index name
 * @param printResult if show the result
 * @return
 */
int RecordManager::selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond,
                                 const IndexHint &indexHint, bool printResult) {
    string tableFileName = table.Name + ".tb";
    string indexFileName = table.Name + "_" + indexHint.attrName + ".ind";
    int recordPos; // Compute a init start point
    if (indexHint.cond.cond == MINISQL_COND_LESS || indexHint.cond.cond == MINISQL_COND_LEQUAL || indexHint.cond.cond == MINISQL_COND_UEQUAL) {
        recordPos = im->searchHead(indexFileName, indexHint.attrType);
    } else {
        recordPos = im->search(indexFileName, indexHint.cond.value);
    }

    int length = table.recordLength + 1;
    int recordsPerBlock = BlockSize / length;
    char *block; // Pointer to block contents
    int blockID; // Corresponding block id
    Element e;
    bool degrade = false;
    int threshold = table.recordCnt / recordsPerBlock / 3;
    int cnt = 0;

    Row row;
    Tuple tup;
    Result res;
    while (recordPos != -1) {
        blockID = recordPos / recordsPerBlock;
        block = bm->getBlock(tableFileName, blockID) + recordPos % recordsPerBlock * length;
        readBlock(block, 0, table.attrType, tup); // Read block contend to tup for check
        if (validCheck(cond, tup, table.attrNames)) {
            row = tup.fetchRow(table.attrNames, attr);
            res.row.push_back(row);
        } else {
            // For a miss in selection
            e = tup.fetchElement(table.attrNames, indexHint.attrName);
            if (indexHint.cond.cond == MINISQL_COND_MORE) {
                // Selection done
                IndexHint tmp = indexHint;
                tmp.cond.cond = MINISQL_COND_GEQUAL;
                if (!tmp.cond.test(e)) {
                    bm->setFree(tableFileName, blockID);
                    break;
                }
            } else if (indexHint.cond.cond == MINISQL_COND_EQUAL && !indexHint.cond.test(e)) {
                bm->setFree(tableFileName, blockID);
                break;
            }
        }
        recordPos = im->searchNext(indexFileName, indexHint.attrType);
        cnt++;
        if (cnt > threshold) {
            degrade = true;
            // If most records are selected then use scan method to avoid overhead of b+ tree
            bm->setFree(tableFileName, blockID);
            break;
        }
        bm->setFree(tableFileName, blockID);
    }

    if (!degrade) {
        if (printResult) {
            print(res);
        }
        return cnt;
    } else {
        return selectRecord(table, attr, cond);
    }
}

/**
 * delete a record in the table
 * @param table
 * @param cond constraint
 * @return
 */
bool RecordManager::deleteRecord(const Table &table, const vector<Cond> &cond) {
    int blockOffset = 0;
    int length = table.recordLength + 1;
    int blocks = BlockSize / length;

    Tuple tup;
    char *block = bm->getBlock(table.Name + ".tb", blockOffset);
    // Scan all the block
    while (block) {
        for (int i = 0; i < blocks; i++) {
            if (block[i * length] != Used) { continue; }
            readBlock(block, i * length, table.attrType, tup);
            if (validCheck(cond, tup, table.attrNames)) {
                block[i * length] = UnUsed;
                // Delete indexed key in b+ tree
                for (auto &col: tup.element) {
                    for (auto &attr : table.index) {
                        if (col.type.attrName == attr.first) {
                            im->removeKey(table.Name + "_" + attr.first + ".ind", col);
                        }
                    }
                }
            }
        }
        bm->setDirty(table.Name + ".tb", blockOffset);
        bm->setFree(table.Name + ".tb", blockOffset);
        blockOffset++;
        block = bm->getBlock(table.Name + ".tb", blockOffset);
    }
    return true;
}

/**
 * print the result
 * @param res
 */
void RecordManager::print(const Result &res) const {
    char *p;
    bool singleMode = false;
    p = getenv("MINISQL_MODE"); // Judge if in gui mode
    if(p && strcmp(p, "SINGLE") == 0) {
        singleMode = true;
    }

    if (singleMode) { // Output the json file
        cout << "[";
        for (auto const &row : res.row) {
            cout << "[";
            for (auto const &col : row.col) {
                cout << "'" << col << "',";
            }
            cout << "],";
        }
        cout << "]";
        cerr << res.row.size() << " selected. \n";
    } else { // Output ordinary tuples
        for (auto const &row : res.row) {
            cout << " | ";
            for (auto const &col : row.col) {
                cout << col << " | ";
            }
            cout << "\n";
        }
        cerr << res.row.size() << " selected. \n";
    }
}

/**
 * Read block content to structured tuple
 * @param blockBuffer
 * @param offset
 * @param attrType
 * @param tup
 */
void RecordManager::readBlock(const char *blockBuffer, int offset, const std::vector<SqlValueType> &attrType, Tuple &tup) {
    Element e;
    tup.element.clear();
    const char *block = blockBuffer + offset + 1; // 1 for meta bit
    for (int i = 0; i < attrType.size(); i++) {
        e.reset();
        e.type = attrType[i];
        switch (attrType[i].M()) {
            case MINISQL_TYPE_INT:
                memcpy(&e.i, block, sizeof(int));
                block += sizeof(int);
                break;
            case MINISQL_TYPE_FLOAT:
                memcpy(&e.r, block, sizeof(float));
                block += sizeof(float);
                break;
            case MINISQL_TYPE_CHAR:
                e.str = block;
                block += attrType[i].charSize + 1;
                break;
        }
        tup.element.push_back(e);
    }
}

/**
 * Check if a tuple fit into the constraint
 * @param conds constraint list
 * @param tu target tuple
 * @param attr
 * @return
 */
bool RecordManager::validCheck(const std::vector<Cond> &conds, const Tuple &tu, const std::vector<std::string> &attr) {
    int condPos;
    // Scan all the attributes if they are match
    for (Cond cond : conds) {
        condPos = -1;
        for (int i = 0; i < attr.size(); i++) {
            if (attr[i] == cond.attr) {
                condPos = i;
                break;
            }
        }
        if (condPos == -1) {
            std::cerr << "Attr not found in cond test!" << std::endl;
        }
        if (!cond.test(tu.element[condPos])) {
            return false;
        }
    }
    return true;
}