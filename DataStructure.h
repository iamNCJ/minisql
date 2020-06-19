#ifndef MINISQL_DATASTRUCTURE_H
#define MINISQL_DATASTRUCTURE_H

#define MINISQL_COND_EQUAL 0
#define MINISQL_COND_UEQUAL 1
#define MINISQL_COND_LEQUAL 2
#define MINISQL_COND_GEQUAL 3
#define MINISQL_COND_LESS 4
#define MINISQL_COND_MORE 5

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace MiniSqlBasic {
    const int BlockSize = 4096;
    const int MaxBlocks = 128;
    const char Unused = 0;
    const char Used = 1;

    inline std::string indexFilename(const std::string &table, const std::string &index) {return table + "_" + index + ".ind";}

    enum class SqlValueTypeEnum {
        Integer,
        String,
        Float
    };

    enum MiniSqlType {
        MINISQL_TYPE_INT,
        MINISQL_TYPE_CHAR,
        MINISQL_TYPE_FLOAT,
        MINISQL_TYPE_NULL
    };

    struct SqlValueType {
        std::string attrName;
        SqlValueTypeEnum type;

        bool unique = false;
        bool primary = false;
        size_t charSize;

        inline int typeIndex() const {
            switch (type) {
                case SqlValueTypeEnum::Integer:
                    return MINISQL_TYPE_INT;
                case SqlValueTypeEnum::Float:
                    return MINISQL_TYPE_FLOAT;
                case SqlValueTypeEnum::String:
                    return MINISQL_TYPE_CHAR;
            }
        }

        inline size_t size() const {
            switch (typeIndex()) {
                case MINISQL_TYPE_INT:
                    return sizeof(int);
                case MINISQL_TYPE_FLOAT:
                    return sizeof(float);
                case MINISQL_TYPE_CHAR:
                    return charSize + 1;
            }
        }

        inline int degree() const {return BlockSize / (size() + sizeof(int));}
    };

    struct SqlValue {
        SqlValueType type;
        int i;
        float r;
        std::string str;

        inline size_t M() const {
            switch (type.type) {
                case SqlValueTypeEnum::Integer:
                    return MINISQL_TYPE_INT;
                case SqlValueTypeEnum::Float:
                    return MINISQL_TYPE_FLOAT;
                case SqlValueTypeEnum::String:
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

    struct Cond {
        Cond() = default;

        Cond(const std::string &attr, const Element &value, int cond) : attr(attr), value(value), cond(cond) {}

        Cond(const Condition &condition)
                : attr(condition.name),
                  value(condition.val) {
            switch (condition.op) {
                case Operator::GE_OP:
                    cond = MINISQL_COND_GEQUAL;
                    break;
                case Operator::LE_OP:
                    cond = MINISQL_COND_LEQUAL;
                    break;
                case Operator::GT_OP:
                    cond = MINISQL_COND_MORE;
                    break;
                case Operator::LT_OP:
                    cond = MINISQL_COND_LESS;
                    break;
                case Operator::EQ_OP:
                    cond = MINISQL_COND_EQUAL;
                    break;
                case Operator::NE_OP:
                    cond = MINISQL_COND_UEQUAL;
                    break;
            }
        }

        int cond;
        std::string attr;
        Element value;

        bool test(const Element &e) const {
            switch (cond) {
                case MINISQL_COND_EQUAL:
                    return e == value;
                case MINISQL_COND_UEQUAL:
                    return e != value;
                case MINISQL_COND_LEQUAL:
                    return e <= value;
                case MINISQL_COND_GEQUAL:
                    return e >= value;
                case MINISQL_COND_LESS:
                    return e < value;
                case MINISQL_COND_MORE:
                    return e > value;
                default:
                    std::cerr << "Undefined condition width cond " << cond << "!" << std::endl;
            }
        }
    };

    struct IndexedAttrList {
        Cond cond;
        std::string attrName;
        int attrType;
    };
}

#endif //MINISQL_DATASTRUCTURE_H
