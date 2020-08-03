#include "Server.hpp"
#include <iostream>
#include <thread>
#include "DataManager.hpp"
#include "CompressUtil.hpp"
#include "NonHotCompress.hpp"
#include <unistd.h>
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
    int fd = open("../log.txt", O_WRONLY);
    if (fd == -1) {
        std::cout << "打开log.txt日志文件失败！" << std::endl;
    }
    dup2(fd, 1);
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
    close(fd);
    return 0;
}
