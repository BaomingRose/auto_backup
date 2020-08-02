#pragma once
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include "DataManager.hpp"
#include "CompressUtil.hpp"

#define NONHOT_TIME 10
#define INTERVAL 30 //没隔时间检测
#define BACKUP_DIR "../backup/"  //文件备份路径
#define GZFILE_DIR "../gzfile/"   //压缩包路径

class NonHotCompress {
public:
    NonHotCompress() {

    }

    //需要使用数据管理
    bool Start() {
        //是一个循环持续的过程，每隔一段时间判断有没有非热点文件，然后压缩
        //问题：什么文件是非热点：当前时间减去最后一次访问时间>n秒
        while (1) {
            //1.获取一下所有的未压缩文件列表
            std::vector<std::string> no_compress;
            _dm->getNoCompress(no_compress);
            //2. 逐个判断这个文件是否是热点文件
            for (size_t i = 0; i < no_compress.size(); ++i) {
                std::string src_path = BACKUP_DIR + no_compress[i];
                bool ret = FileIsHot(src_path);
                //3. 如果是非热点文件，则压缩，删除源文件
                if (ret == false) {
                    std::string compress_name = no_compress[i] + ".gz";
                    std::string dst_path = GZFILE_DIR + compress_name;
                    if (CompressUtil::Compress(src_path, dst_path)) {
                        _dm->Update(no_compress[i], no_compress[i], compress_name);    //更新数据信息
                        unlink(src_path.c_str());
                    }
                }
            }
            //4. 休眠一会
            sleep(INTERVAL);
        }
    }
private:
    //判断一个文件是否是一个热点文件
    bool FileIsHot(const std::string& name) {
        //当前时间减去最后一次访问时间>n秒
        time_t cur_t = time(NULL);
        struct stat st;
        if (stat(name.c_str(), &st) < 0) {
            std::cout << "get file " << name << " stat failed!" << std::endl;
            return true;
        }

        if ((cur_t - st.st_atim.tv_sec) > NONHOT_TIME) {
            return false;
        }

        return true;
    }
public:
    static DataManager* _dm;
};

DataManager* NonHotCompress::_dm = nullptr;
