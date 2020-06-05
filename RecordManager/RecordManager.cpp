#include "RecordManager.h"

bool RecordManager::createTable(const string &table) {
    bm->createFile(table + ".tb");
}

bool RecordManager::dropTable(const string &table) {
    string tableFileStr = table + ".tb";
    bm->removeFile(tableFileStr);
    return true;
}

bool RecordManager::createIndex(const Table &table, const SqlValueType &index) {
    string indexFileStr = table.Name + "_" + index.attrName + ".ind";

    bm->createFile(indexFileStr);
    im->create(indexFileStr, index);
    
    int blockID = 0;
    char *block = bm->getBlock(table.Name + ".tb", blockID);
    int length = table.recordLength + 1;
    int recordsPerBlock = BlockSize / length;
    int offset = 1;
    Element attr;
    const char *dest;

    for (auto const &attrType : table.attrType) {
        if (attrType.attrName == index.attrName) {
            attr.type = attrType;
            break;
        }
        offset += attrType.getSize();
    }
    while (block) {
        for (int i = 0; i < recordsPerBlock; i++) {
            if (block[i * length] != Used) { continue; }
            dest = block + i * length + offset;
            switch (attr.M()) {
                case MINISQL_TYPE_INT:
                    memcpy(&attr.i, dest, attr.type.getSize());
                    cout << attr.i << '\n';
                    break;
                case MINISQL_TYPE_FLOAT:
                    memcpy(&attr.r, dest, attr.type.getSize());
                    cout << attr.r << '\n';
                    break;
                case MINISQL_TYPE_CHAR:
                    attr.str.replace(0, attr.type.getSize(), dest);
                    break;
                default:
                    cerr << "Undefined type in RM::createIndex." << endl;
            }
            im->insert(indexFileStr, attr, blockID * recordsPerBlock + i); 
        }
        bm->setFree(table.Name + ".tb", blockID);
        blockID++;
        block = bm->getBlock(table.Name + ".tb", blockID);
    }
    return true;
}

bool RecordManager::dropIndex(const Table &table, const string &index) {
    string indexFileStr = table.Name + "_" + index + ".ind";
    bm->removeFile(indexFileStr);
    bool foundAttr = false;

    for (auto &attr : table.attrType) {
        if (attr.attrName == index) {
            foundAttr = true;
            im->drop(indexFileStr, attr);
            break;
        }
    }
    if (!foundAttr) {
        cerr << "Drop index on undefined attr!" << endl;
    }
}

int RecordManager::insertRecord(const Table &table, const Tuple &record) {
    string tableName = table.Name + ".tb";
    int tailBlockID = bm->getTailBlock(tableName);
    char *content;
    if (tailBlockID >= 0) {
        content = bm->getBlock(tableName, tailBlockID);
    } else {
        tailBlockID = 0;
        content = bm->getBlock(tableName, tailBlockID, true);
    }

    int len =  table.recordLength + 1;
    int rcdPerBlock = BlockSize / len;

    int rcdOffset = 0;
    bool validWord = false;
    while (rcdOffset < rcdPerBlock) {
        if (content[rcdOffset * len] == Used) {
            rcdOffset++;
        } else {
            validWord = true;
            content += rcdOffset * len;
            break;
        }
    }

    if (!validWord) {
        rcdOffset = 0;
        bm->setFree(tableName, tailBlockID);
        content = bm->getBlock(tableName, ++tailBlockID, true);
    }

    int offset = 1;
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
    content[0] = Used;
    bm->setDirty(tableName, tailBlockID);
    bm->setFree(tableName, tailBlockID);
    return tailBlockID * rcdPerBlock + rcdOffset;
}
int RecordManager::selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond) {
    int blockID = 0;
    char *block = bm->getBlock(table.Name + ".tb", blockID);
    int length = table.recordLength + 1;
    int blocks = BlockSize / length;
    Tuple tup;
    Row row;
    Result res;

    while (block) {
        for (int i = 0; i < blocks; i++) {
            if (block[i * length] != Used) { continue; }
            convertToTuple(block, i * length, table.attrType, tup);
            if (condsTest(cond, tup, table.attrNames)) {
                row = tup.fetchRow(table.attrNames, attr);
                res.row.push_back(row);
            }
        }
        bm->setFree(table.Name + ".tb", blockID);
        blockID++;
        block = bm->getBlock(table.Name + ".tb", blockID);
    }

    dumpResult(res);
    return res.row.size();
}

int RecordManager::selectRecord(const Table &table, const vector<string> &attr, const vector<Cond> &cond,
                                 const IndexHint &indexHint, bool printResult) {
    string tableFileName = table.Name + ".tb";
    string indexFileName = table.Name + "_" + indexHint.attrName + ".ind";
    int recordPos;
    if (indexHint.cond.cond == MINISQL_COND_LESS || indexHint.cond.cond == MINISQL_COND_LEQUAL || indexHint.cond.cond == MINISQL_COND_UEQUAL) {
        recordPos = im->searchHead(indexFileName, indexHint.attrType);
    } else {
        recordPos = im->search(indexFileName, indexHint.cond.value);
    }

    int length = table.recordLength + 1;
    int recordsPerBlock = BlockSize / length;
    char *block;
    int blockID;
    Tuple tup;
    Row row;
    Result res;
    Element e;
    bool degrade = false;
    int threshold = table.recordCnt / recordsPerBlock / 3;
    int cnt = 0;

    while (recordPos != -1) {
        blockID = recordPos / recordsPerBlock;
        block = bm->getBlock(tableFileName, blockID) + recordPos % recordsPerBlock * length;
        convertToTuple(block, 0, table.attrType, tup);
        if (condsTest(cond, tup, table.attrNames)) {
            row = tup.fetchRow(table.attrNames, attr);
            res.row.push_back(row);
        } else {
            e = tup.fetchElement(table.attrNames, indexHint.attrName);
            if (indexHint.cond.cond == MINISQL_COND_MORE) {
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
            bm->setFree(tableFileName, blockID);
            break;
        }
        bm->setFree(tableFileName, blockID);
    }

    if (!degrade) {
        if (printResult) {
            dumpResult(res);
        }
        return cnt;
    } else {
        return selectRecord(table, attr, cond);
    }
}

//TODO
bool RecordManager::deleteRecord(const Table &table, const vector<Cond> &cond) {
    int blockOffset = 0;
    char *block = bm->getBlock(table.Name + ".tb", blockOffset);
    int length = table.recordLength + 1;
    int blocks = BlockSize / length;
    Tuple tup;

    while (block) {
        for (int i = 0; i < blocks; i++) {
            if (block[i * length] != Used) { continue; }
            convertToTuple(block, i * length, table.attrType, tup);
            if (condsTest(cond, tup, table.attrNames)) {
                block[i * length] = UnUsed;
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

//@ Output the result
void RecordManager::dumpResult(const Result &res) const {
    char *p;
    bool singleMode = false;
    p = getenv("MINISQL_MODE");
    if(p && strcmp(p, "SINGLE") == 0) {
        singleMode = true;
    }

    if (singleMode) {
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
    } else {
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

/*
* @convert a attr vector to tuple
*/
void RecordManager::convertToTuple(const char *blockBuffer, int offset, const std::vector<SqlValueType> &attrType, Tuple &tup) {
    const char *block = blockBuffer + offset + 1; // 1 for meta bit
    Element e;
    tup.element.clear();
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

bool RecordManager::condsTest(const std::vector<Cond> &conds, const Tuple &tup, const std::vector<std::string> &attr) {
    int condPos;
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
        if (!cond.test(tup.element[condPos])) {
            return false;
        }
    }
    return true;
}