#include <zlib.h>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

//读写文件的功能模块
class FileUtil {
public:
    static bool Read(const std::string& filename, std::string* body) {
        //以二进制方式打开文件,输出文件流
        std::ifstream ifs(filename, std::ios::binary);
        if (ifs.is_open() == false) {
            std::cout << "open file" << filename << " failed\n";
            return false;
        }

        //获取文件大小，boost库中函数
        int64_t fsize = boost::filesystem::file_size(filename);
        body->resize(fsize);
        ifs.read(&(*body)[0], fsize);

        if (ifs.good() == false) {
            std::cout << "file " << filename << " read data failed!\n";
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }

    static bool Write(const std::string& filename, const std::string& body) {
        std::ofstream ofs(filename, std::ios::binary);
        if (ofs.is_open() == false) {
            std::cout << "open file" << filename << " failed\n";
            return false;
        }

        ofs.write(&body[0], body.size());
        if (ofs.good() == false) {
            std::cout << "file " << filename << " write data failed!\n";
            ofs.close();
            return false;
        }
        ofs.close();
        return true;
    }
};


class CompressUtil {
public:
    static bool Compress(const std::string& src, const std::string& dst) {
        std::string body;
        FileUtil::Read(src, &body);

        //打开压缩包
        gzFile gf = gzopen(dst.c_str(), "wb");
        if (gf == NULL) {
            std::cout << "open file " << dst << " failed!\n";
            return false;
        }

        size_t wlen = 0;
        while (wlen < body.size()) {  //防止body中数据没有一次压缩成功
            //若一次没有压缩成功，则从未压缩数据继续压缩
            int ret = gzwrite(gf, &body[wlen], body.size() - wlen);
            if (ret == 0) {
                std::cout << "file " << dst << " write compress data failed!\n";
                gzclose(gf);
                return false;
            }
            wlen += ret;
        }

        gzclose(gf);
        return true;
    }

    static bool Uncompress(const std::string& dst, const std::string& src) {
        std::ofstream ofs(src, std::ios::binary);
        if (ofs.is_open() == false) {
            std::cout << "open file " << dst << " failed!\n";
            return false;
        }

        gzFile gf = gzopen(dst.c_str(), "rb");
        if (gf == NULL) {
            std::cout  << "open file " << src << " failed!\n";
            ofs.close();
            return false;
        }

        char tmp[4096] = { 0 };
        int ret = 0;
        //从压缩包一次取出4096，因为每次可以读的大小是未知的
        while ((ret = gzread(gf, tmp, 4096)) > 0) {
            ofs.write(tmp, ret);
        }

        ofs.close();
        gzclose(gf);
        return true;
    }
};
