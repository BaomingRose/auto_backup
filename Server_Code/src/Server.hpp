#pragma once
#include <string>
#include <sstream>
#include "httplib.h"
#include "DataManager.hpp"
#include "CompressUtil.hpp"

#define BACKUP_DIR "../backup/"  //文件备份路径
#define GZFILE_DIR  "../gzfile/" //压缩包存放路径

class Server {
public:
    Server() {  }

    ~Server() {  }

    bool Start() {
        _server.Put("/rose_backup/upload/(.*)", Upload);
        _server.Get("/rose_backup/list", List);
        _server.Get("/rose_backup/download/(.*)", DownLoad);
        _server.Get("/rose_backup/delete/(.*)", Delete);
        _server.Get("/rose_backup/get_all", Get_all_src_with_time);

        _server.listen("0.0.0.0", 10000);

        return true;
    }

private:
    //文件上传处理函数
    static void Upload(const httplib::Request& req, httplib::Response& rsp) {
        std::string filename = req.matches[1];
        std::string pathname = BACKUP_DIR + filename;
        //将文件内容写到路径下
        FileUtil::Write(pathname, req.body);

        std::string fsize = std::to_string(req.body.size());
        
        //将当前时间更新为上传时间
        time_t cur_t = time(NULL);
        struct tm* stm_ptr = localtime(&cur_t);
        char buf[20];
        strftime(buf, 20, "%Y-%m-%d %H:%M:%S", stm_ptr);
        std::string filetime(buf);
        filetime = fsize + " " + filetime;

        //插入到数据库中
        bool res = _dm->Insert(filename, filename, filetime);
        if (res == false) {
            _dm->Update_time(filename, filetime);
        }
        
        rsp.status = 200;
        // rsp.set_content("upload", 6, "text/html");
        return;
    }
    
    //文件列表处理函数
    static void List(const httplib::Request& req, httplib::Response& rsp) {
        std::string method = req.method;

        //从数据库中获取所有名称
        std::map<std::string, std::string> m;
        _dm->Query_all_with_time(m);
        
        std::stringstream tmp;
        tmp << "<html><head><meta charset=\"UTF-8\"><style>table{border-collapse:collaps; border-spacing:0;} td{text-align:center;}</style></head><body><center><hr/><table><tr><td style=\"width:500px; height:70px;\">文件名/下载</td><td style=\"width:300px; height:70px\">文件大小</td><td style=\"width:300px; height:70px\">上传时间</td><td style=\"width:150px;\">是否删除</td></tr></table><hr/><table>";
        for (const auto& e : m) {
            int index = e.second.find(' ');
            std::string fsize = e.second.substr(0, index);
            std::string modify_time = e.second.substr(index + 1, e.second.size()-index);
            tmp << "<tr><td style=\"width:500px; height:40px;\"><a href='/rose_backup/download/" << e.first << "'>" << e.first << "</a></td><td style=\"width:300px;\">&nbsp;" << fsize << "&nbsp;字节</td><td style=\"width:300px;\">" << modify_time << "</td><td style=\"width:150px;\"><a href=\"/rose_backup/delete/" << e.first << "\">删除</a></td></tr>";
        }
        tmp << "</table></center></body></html>";

        rsp.set_content(tmp.str().c_str(), tmp.str().size(), "text/html");
        rsp.status = 200;
        return;
    }

    //文件下载处理函数
    static void DownLoad(const httplib::Request& req, httplib::Response& rsp) {
        rsp.status = 200;
        std::string filename = req.matches[1];

        //数据库中是否存在,返回404
        if (_dm->Query(filename) == false) {
            rsp.status = 404;
            return;
        }

        std::string pathname = BACKUP_DIR + filename;
        //判断是否压缩，如果未压缩直接返回
        if (_dm->isCompress(filename) == false) {
            FileUtil::Read(pathname, &rsp.body);
            std::cout << "从" << pathname << "读到body" << std::endl;
            rsp.set_header("Content-Type", "application/octet-stream");
            rsp.status = 200;
            return;
        } else {
            //压缩则解压缩，修改数据库
            //获取压缩名称
            std::string gzfile;
            _dm->getCompressName(filename, gzfile);
            //解压缩
            std::string gzpath = GZFILE_DIR + gzfile;
            CompressUtil::Uncompress(gzpath, pathname);
            //删除压缩包
            unlink(gzpath.c_str());
            //更改数据库,张文超忘记做的
            _dm->Update(filename, filename, filename);
            rsp.set_header("Content-Type", "application/octet-stream");
            FileUtil::Read(pathname, &rsp.body);
            rsp.status = 200;
            return;
        }
    }

    //删除请求
    static void Delete(const httplib::Request& req, httplib::Response& rsp) {
        rsp.status = 200;
        std::string filename = req.matches[1];

        //数据库中是否存在,返回404
        if (_dm->Query(filename) == false) {
            rsp.status = 404;
            return;
        }
        _dm->Delete(filename);
        std::string filepath = BACKUP_DIR + filename;
        std::string gzpath = GZFILE_DIR + filename + ".gz";
        if (boost::filesystem::exists(filepath) == true) {
            unlink(filepath.c_str());
            std::cout << "delete " << filepath << " success!\n";
        }
        if (boost::filesystem::exists(gzpath) == true) {
            unlink(gzpath.c_str());
            std::cout << "delete " << gzpath << " success!\n";
        }

        rsp.set_redirect("/rose_backup/list");
        return;
    }

    //客户端初始化时的请求
    static void Get_all_src_with_time(const httplib::Request& req, httplib::Response& rsp) {
        std::string method = req.method;
        
        //从数据库中获取所有名称
        std::map<std::string, std::string> m;
        _dm->Query_all_with_time(m);
        std::stringstream tmp;
        for (const auto& e : m) {
            tmp << e.first << "\t" << e.second << std::endl;
        }
        rsp.set_content(tmp.str(), "text/html");
        return;
    }

private:
    std::string _file_dir;  //文件上传备份路径
    httplib::Server _server;
public:
    static DataManager* _dm;
};

DataManager* Server::_dm = nullptr;
