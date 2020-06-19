#include "IndexManager.h"

void IndexManager::create(const string &filename, const SqlValueType &type) {
    int itemSize = type.size();
    int treeDegree = type.degree();
    switch (type.typeIndex()) {
        case MINISQL_TYPE_INT: {
            auto tree = std::make_shared<decltype(intIndexMap)::value_type::second_type::element_type>(filename, itemSize, treeDegree);
            intIndexMap.insert(decltype(intIndexMap)::value_type(filename, tree));
            break;
        }
        case MINISQL_TYPE_FLOAT: {
            auto tree = std::make_shared<decltype(floatIndexMap)::value_type::second_type::element_type>(filename, itemSize, treeDegree);
            floatIndexMap.insert(decltype(floatIndexMap)::value_type(filename, tree));
            break;
        }
        case MINISQL_TYPE_CHAR: {
            auto tree = std::make_shared<decltype(charIndexMap)::value_type::second_type::element_type>(filename, itemSize, treeDegree);
            charIndexMap.insert(decltype(charIndexMap)::value_type(filename, tree));
            break;
        }
        default:
            cerr << "Undefined type!" << endl;
            break;
    }
}

void IndexManager::drop(const string &filename, const SqlValueType &type) {
    switch (type.typeIndex()) {
        case MINISQL_TYPE_INT: {
            intIndexMap.erase(intIndexMap.find(filename));
            break;
        }
        case MINISQL_TYPE_FLOAT: {
            floatIndexMap.erase(floatIndexMap.find(filename));
            break;
        }
        case MINISQL_TYPE_CHAR: {
            charIndexMap.erase(charIndexMap.find(filename));
            break;
        }
        default:
            cerr << "Undefined type!" << endl;
            break;
    }
}

int IndexManager::search(const string &filename, const Element &e) {
    switch (e.type.typeIndex()) {
        case MINISQL_TYPE_INT: {
            NodeSearchParse<int> intNode = intIndexMap.find(filename)->second->findNode(e.i);
            intOffsetMap[filename] = intNode;
            return intNode.node->keyOffset[intNode.index];
        }
        case MINISQL_TYPE_FLOAT: {
            NodeSearchParse<float> floatNode = floatIndexMap.find(filename)->second->findNode(e.r);
            floatOffsetMap[filename] = floatNode;
            return floatNode.node->keyOffset[floatNode.index];
        }
        case MINISQL_TYPE_CHAR: {
            NodeSearchParse<string> charNode = charIndexMap.find(filename)->second->findNode(e.str);
            charOffsetMap[filename] = charNode;
            return charNode.node->keyOffset[charNode.index];
        }
        default:
            cerr << "Undefined type!" << endl;
            return -1;
    }
}

int IndexManager::searchNext(const string &filename, int attrType) {
    switch (attrType) {
        case MINISQL_TYPE_INT: {
            NodeSearchParse<int> intNode = intOffsetMap.find(filename)->second;
            intNode.index++;
            if (intNode.index == intNode.node->cnt) {
                intNode.node = intNode.node->sibling;
                intNode.index = 0;
            }
            intOffsetMap[filename] = intNode;
            if (intNode.node != nullptr) {
                return intNode.node->keyOffset[intNode.index];
            }
            break;
        }
        case MINISQL_TYPE_FLOAT: {
            NodeSearchParse<float> floatNode = floatOffsetMap.find(filename)->second;
            floatNode.index++;
            if (floatNode.index == floatNode.node->cnt) {
                floatNode.node = floatNode.node->sibling;
                floatNode.index = 0;
            }
            floatOffsetMap[filename] = floatNode;
            if (floatNode.node != nullptr) {
                return floatNode.node->keyOffset[floatNode.index];
            }
            break;
        }
        case MINISQL_TYPE_CHAR: {
            NodeSearchParse<string> charNode = charOffsetMap.find(filename)->second;
            charNode.index++;
            if (charNode.index == charNode.node->cnt) {
                charNode.node = charNode.node->sibling;
                charNode.index = 0;
            }
            charOffsetMap[filename] = charNode;
            if (charNode.node != nullptr) {
                return charNode.node->keyOffset[charNode.index];
            }
            break;
        }
        default:
            cerr << "Undefined type!" << endl;
            return -1;
    }
    return -1;
}

bool IndexManager::insert(const string &filename, const Element &e, int offset) {
    switch (e.type.typeIndex()) {
        case MINISQL_TYPE_INT:
            return intIndexMap.find(filename)->second->insert(e.i, offset);
        case MINISQL_TYPE_FLOAT:
            return floatIndexMap.find(filename)->second->insert(e.r, offset);
        case MINISQL_TYPE_CHAR:
            return charIndexMap.find(filename)->second->insert(e.str, offset);
        default:
            cerr << "Undefined type!" << endl;
            return {};
    }
}

bool IndexManager::removeKey(const string &filename, const Element &e) {
    switch (e.type.typeIndex()) {
        case MINISQL_TYPE_INT:
            return intIndexMap.find(filename)->second->remove(e.i);
        case MINISQL_TYPE_FLOAT:
            return floatIndexMap.find(filename)->second->remove(e.r);
        case MINISQL_TYPE_CHAR:
            return charIndexMap.find(filename)->second->remove(e.str);
        default:
            cerr << "Undefined type!" << endl;
            return {};
    }
}

int IndexManager::searchHead(const string &filename, int attrType) {
    switch (attrType) {
        case MINISQL_TYPE_INT: {
            NodeSearchParse<int> intNode{};
            intNode.node = intIndexMap.find(filename)->second->getHeadNode();
            intNode.index = 0;
            intOffsetMap[filename] = intNode;
            return intNode.node->keyOffset[intNode.index];
        }
        case MINISQL_TYPE_FLOAT: {
            NodeSearchParse<float> floatNode{};
            floatNode.node = floatIndexMap.find(filename)->second->getHeadNode();
            floatNode.index = 0;
            floatOffsetMap[filename] = floatNode;
            return floatNode.node->keyOffset[floatNode.index];
        }
        case MINISQL_TYPE_CHAR: {
            NodeSearchParse<string> charNode{};
            charNode.node = charIndexMap.find(filename)->second->getHeadNode();
            charNode.index = 0;
            charOffsetMap[filename] = charNode;
            return charNode.node->keyOffset[charNode.index];
        }
        default:
            cerr << "Undefined type!" << endl;
            return {};
    }
}
