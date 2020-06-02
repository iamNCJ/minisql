#ifndef MINISQL_DATASTRUCTURE_H
#define MINISQL_DATASTRUCTURE_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace MiniSqlBasic {
    const int BlockSize = 4096;
    const int MaxBlocks = 128;
    const char UnUsed = 0;
    const char Used = 1;

    enum class SqlValueTypeBase {
        Integer,
        String,
        Float
    };

    enum MinisqlType {
        MINISQL_TYPE_INT,
        MINISQL_TYPE_CHAR,
        MINISQL_TYPE_FLOAT,
        MINISQL_TYPE_NULL
    };

    struct SqlValueType {
        std::string attrName;
        SqlValueTypeBase type;

        ///following fileds only for attribute type, not a single sql value type.
        bool primary = false;
        size_t charSize; // charSize does not include the terminating zero of string!
        bool unique = false;

        inline int M() const {
            switch (type) {
                case SqlValueTypeBase::Integer:
                    return MINISQL_TYPE_INT;
                case SqlValueTypeBase::Float:
                    return MINISQL_TYPE_FLOAT;
                case SqlValueTypeBase::String:
                    return MINISQL_TYPE_CHAR;
            }
        }

        inline size_t getSize() const {
            switch (M()) {
                case MINISQL_TYPE_INT:
                    return sizeof(int);
                case MINISQL_TYPE_FLOAT:
                    return sizeof(float);
                case MINISQL_TYPE_CHAR:
                    return charSize + 1;
            }
        }

        inline int getDegree() const {
            size_t keySize = getSize();
            int degree = BlockSize / (keySize + sizeof(int));

            return degree;
        }
    };

    struct SqlValue {
        SqlValueType type;
        int i;
        float r;
        std::string str;

        inline size_t M() const {
            switch (type.type) {
                case SqlValueTypeBase::Integer:
                    return MINISQL_TYPE_INT;
                case SqlValueTypeBase::Float:
                    return MINISQL_TYPE_FLOAT;
                case SqlValueTypeBase::String:
                    return MINISQL_TYPE_CHAR;
            }
        }

        bool operator<(const SqlValue &e) const {
            switch (M()) {
                case MINISQL_TYPE_INT:
                    return i < e.i;
                case MINISQL_TYPE_FLOAT:
                    return r < e.r;
                case MINISQL_TYPE_CHAR:
                    return str < e.str;
                default:
                    throw std::runtime_error("Undefined Type!");
            }
        }

        bool operator==(const SqlValue &e) const {
            switch (M()) {
                case MINISQL_TYPE_INT:
                    return i == e.i;
                case MINISQL_TYPE_FLOAT:
                    return r == e.r;
                case MINISQL_TYPE_CHAR:
                    return str == e.str;
                default:
                    throw std::runtime_error("Undefined Type!");
            }
        }

        bool operator!=(const SqlValue &e) const { return !operator==(e); }

        bool operator>(const SqlValue &e) const { return !operator<(e) && operator!=(e); }

        bool operator<=(const SqlValue &e) const { return operator<(e) || operator==(e); }

        bool operator>=(const SqlValue &e) const { return !operator<(e); }

        void reset() {
            str.clear();
            i = 0;
            r = 0;
        }

        std::string toStr() const {
            switch (M()) {
                case MINISQL_TYPE_INT:
                    return std::to_string(i);
                case MINISQL_TYPE_FLOAT:
                    return std::to_string(r);
                case MINISQL_TYPE_CHAR:
                    return this->str;
            }
        }
    };

    enum class Operator {
        GT_OP,
        GE_OP,
        LT_OP,
        LE_OP,
        EQ_OP,
        NE_OP
    };

    inline Operator flip_operator(Operator op) {
        switch (op) {
            case Operator::GT_OP:
                return Operator::LT_OP;
            case Operator::GE_OP:
                return Operator::LE_OP;
            case Operator::LT_OP:
                return Operator::GT_OP;
            case Operator::LE_OP:
                return Operator::GE_OP;
        }
        return op;      // = and != need not flip.
    }

    struct Condition {
        std::string name;
        Operator op;
        SqlValue val;
    };

    typedef struct SqlValue Element;

    struct Row {
        std::vector<std::string> col;
    };

    struct Result {
        std::vector<Row> row;
    };

    struct Tuple {
        std::vector<Element> element;

        Row fetchRow(const std::vector<std::string> &attrTable, const std::vector<std::string> &attrFetch) const {
            Row row;
            bool attrFound;
            row.col.reserve(attrFetch.size());
            for (auto fetch : attrFetch) {
                attrFound = false;
                for (int i = 0; i < attrTable.size(); i++) {
                    if (fetch == attrTable[i]) {
                        row.col.push_back(element[i].toStr());
                        attrFound = true;
                        break;
                    }
                }
                if (!attrFound) {
                    std::cerr << "Undefined attr in row fetching!!" << std::endl;
                }
            }
            return row;
        }

        const Element &fetchElement(const std::vector<std::string> &attrTable, const std::string &attrFetch) const {
            for (int i = 0; i < attrTable.size(); i++) {
                if (attrFetch == attrTable[i]) {
                    return element[i];
                }
            }
            std::cerr << "Undefined attr in element fetching from tuple!!" << std::endl;
        }
    };

    struct Table {
        Table() {};

        std::string Name;
        int attrCount, recordLength, recordCnt, size;

        std::vector<SqlValueType> attrType;
        std::vector<std::string> attrNames;
        std::vector<std::pair<std::string, std::string>> index;

        friend std::ostream &operator<<(std::ostream &os, const Table &table) {
            os << "Name: " << table.Name << " attrCount: " << table.attrCount << " recordLength: " << table.recordLength
                << " recordCnt: " << table.recordCnt << " size: " << table.size
                << " attrNames: " << table.attrNames.size();
            return os;
        }
    };

    struct IndexHint {
        Condition cond;
        std::string attrName;
        int attrType;
    };
}

#endif //MINISQL_DATASTRUCTURE_H
