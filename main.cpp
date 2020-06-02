#include <iostream>

#include "BufferManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include "DataStructure.h"
#include "sys/stat.h"

using namespace std;
using namespace MiniSqlBasic;

int main() {
    std::cout << "Welcome to miniSQL!" << std::endl;
    BufferManager bm;
    IndexManager im;
    RecordManager rm(&bm, &im);

    rm.createTable("test");
    struct stat buf{};
    Table tb;
    tb.Name = "test";
    tb.attrCount = 3;
    SqlValue intAttr, flAttr, charAttr;

    SqlValueType i;
    i.attrName = "i";
    i.type = SqlValueTypeBase::Integer;

    SqlValueType f;
    f.attrName = "f";
    f.type = SqlValueTypeBase::Float;

    SqlValueType c;
    c.attrName = "c";
    c.type = SqlValueTypeBase::String;
    c.charSize = 5;

    std::vector<std::pair<std::string, SqlValueType>> schema;
    schema.push_back(make_pair("i", i));
    schema.push_back(make_pair("f", f));
    schema.push_back(make_pair("c", c));

    int len = 0;
    for (const auto &sch: schema) {
        len += sch.second.getSize();
        tb.attrNames.push_back(sch.first);
        auto t = sch.second;
        t.attrName = sch.first;
        tb.attrType.push_back(t);
        if (sch.first == "i") {
            (tb.attrType.end() - 1)->primary = true;
            (tb.attrType.end() - 1)->unique = true;
        }
    }
    tb.recordLength = len;
    tb.recordCnt = 0;
    char auto_ind{'A'};
//    for (auto &t: tb.attrType)
//    {
//        if (t.unique && !t.primary)
//        {
//            tb.index.push_back(std::make_pair(t.attrName, std::string("auto_ind_") + (auto_ind++)));
//            rm.createIndex(tb, t);
//        }
//    }
    Tuple tu;
    intAttr.i = 1;
    intAttr.type = i;
    tu.element.push_back(intAttr);
    flAttr.r = 2.0;
    flAttr.type = f;
    tu.element.push_back(flAttr);
    charAttr.str = "test!";
    charAttr.type = c;
    tu.element.push_back(charAttr);
    
    
    rm.insertRecord(tb, tu);
    tu.element[0].i = 2;
    tu.element[1].r = -2.3;
    tu.element[2].str = "Oh~";
    rm.insertRecord(tb, tu);
    vector<string> attrs;
    attrs.push_back("i");
    attrs.push_back("f");
    attrs.push_back("c");
    vector<MiniSqlBasic::Cond> conds;
    rm.selectRecord(tb, attrs, conds);
}