#ifndef MINISQL_DATASTRUCTURE_H
#define MINISQL_DATASTRUCTURE_H

#include <string>
#include <stdexcept>

namespace MiniSqlBasic {
    const int BlockSize = 4096;
    const int MaxBlocks = 128;

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
}

#endif //MINISQL_DATASTRUCTURE_H
