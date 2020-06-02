#include <iostream>

#include "BufferManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include "DataStructure.h"

int main() {
    std::cout << "Welcome to miniSQL!" << std::endl;
    BufferManager bm;
    IndexManager im;
    RecordManager rm(&bm, &im);

    rm.createTable("student");
    Table t;

}