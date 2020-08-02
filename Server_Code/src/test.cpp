#include "Server.hpp"
#include <iostream>
#include <thread>
#include "DataManager.hpp"
#include "CompressUtil.hpp"
#include "NonHotCompress.hpp"
using namespace std;

DataManager dm;

void testHttp() {
    Server s;
    s._dm = &dm;
    s.Start();
}

void testCompress() {
    NonHotCompress nhc;
    nhc._dm = &dm;
    nhc.Start();
}

int main() {
    if (boost::filesystem::exists(GZFILE_DIR) == false) {                                                 
        boost::filesystem::create_directory(GZFILE_DIR);    
    }    
    if (boost::filesystem::exists(BACKUP_DIR) == false) {    
        boost::filesystem::create_directory(BACKUP_DIR);    
    }

    dm.Connect();
    thread t1(testHttp);
    thread t2(testCompress);

    t1.join();
    t2.join();

    dm.Close();
    return 0;
}
