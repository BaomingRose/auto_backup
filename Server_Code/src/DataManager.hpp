#pragma once
#include <mysql/mysql.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>


class DataManager {
public:
    DataManager() {
        mysql_init(&conn);
    }

    bool Connect() {
        if (mysql_real_connect(&conn, "localhost", "root", "123456", "file_backup", 0, NULL, CLIENT_FOUND_ROWS)) {
            std::cout << "数据库连接成功!" << std::endl;
        } else {
            std::cout << "数据库连接失败!" << std::endl;
            return false;
        }
        return true;
    }

    void Close() {
        mysql_close(&conn);
    }

    bool Insert(const std::string& src_name, const std::string& dst_name, const std::string& file_time) {
        std::string sql = "insert into compress values('" + src_name + "', '" + dst_name + "', '" + file_time + "')";
        std::cout <<  sql << std::endl;
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "insert " << src_name << " failed!" << std::endl;
            return false;
        }
        return true;
    }

    bool Query_all(std::map<std::string, std::string>& data) {
        std::string sql = "select * from compress";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query failed" << std::endl;
            return false;
        }

        MYSQL_RES* res_ptr = mysql_store_result(&conn);
        if (!res_ptr) {
            std::cout << "query all failed" << std::endl;
            return false;
        }

        int row = mysql_num_rows(res_ptr);

        for (int i = 0; i < row; ++i) {
            MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
            data.insert(std::make_pair(std::string(result_row[0]), std::string(result_row[1])));
        }

        mysql_free_result(res_ptr);
        return true;
    }

    bool Query(const std::string& file_name) {
        std::string sql = "select * from compress where src_file_name='" + file_name + "'";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query " << file_name << " failed!" << std::endl;
            return false;
        }

        MYSQL_RES* res_ptr = mysql_store_result(&conn);
        if (!res_ptr) {
            std::cout << "query all failed" << std::endl;
            return false;
        }
        int row = mysql_num_rows(res_ptr);
        if (row == 0) {
            std::cout << "query " << file_name << " is null!" << std::endl;
            return false;
        }

        return true;
    }

    bool Query_all_with_time(std::map<std::string, std::string>& src_with_time) {
        std::string sql = "select * from compress";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query all with time failed" << std::endl;
            return false;
        }

        MYSQL_RES* res_ptr = mysql_store_result(&conn);
        if (!res_ptr) {
            std::cout << "query all with time failed" << std::endl;
            return false;
        }

        int row = mysql_num_rows(res_ptr);

        for (int i = 0; i < row; ++i) {
            MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
            src_with_time.insert(std::make_pair(std::string(result_row[0]), std::string(result_row[2])));
        }

        mysql_free_result(res_ptr);
        return true;
    }

    bool isCompress(const std::string& file_name) {
        std::string sql = "select * from compress where src_file_name='" + file_name + "'";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query " << file_name << " failed!";
            return false;
        }
        MYSQL_RES* res_ptr = mysql_store_result(&conn);
        MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
        
        std::string s1(result_row[0]);
        std::string s2(result_row[1]);
        mysql_free_result(res_ptr);
        if (s1 == s2) {
            return false;
        }
        return true;
    }

    bool getNoCompress(std::vector<std::string>& no_compress) {
        std::string sql = "select * from compress where src_file_name=compress_name";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query list of Uncompress failed" << std::endl;
            return false;
        }

        MYSQL_RES* res_ptr = mysql_store_result(&conn);
        if (!res_ptr) {
            std::cout << "get result list of Uncompress failed" << std::endl;
            return false;
        }

        int row = mysql_num_rows(res_ptr);

        for (int i = 0; i < row; ++i) {
            MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
            no_compress.push_back(std::string(result_row[0]));
        }

        mysql_free_result(res_ptr);
        return true;
    }

    bool getCompressName(const std::string& src_name, std::string& compress_name) {
        std::string sql = "select * from compress where src_file_name='" + src_name + "'";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query compress_name failed" << std::endl;
            return false;
        }

        MYSQL_RES* res_ptr = mysql_store_result(&conn);
        if (!res_ptr) {
            std::cout << "get result list of Uncompress failed" << std::endl;
            return false;
        }

        int row = mysql_num_rows(res_ptr);
        if (row != 1) {
            std::cout << "no this  src_name:" << src_name << "!"<< std::endl;
        }

        for (int i = 0; i < row; ++i) {
            MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
            compress_name = result_row[1];
        }

        mysql_free_result(res_ptr);
        return true;
    }

    bool Delete(const std::string& file_name) {
        std::string sql = "delete from compress where src_file_name='" + file_name + "'";
        std::cout << sql << std::endl;
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "delete " << file_name  << " failed!" << std::endl;
            return false;
        }

        return true;
    }

    bool Update(const std::string& file_name , const std::string& new_name,  const std::string& compress_name) {
        std::string sql = "update compress set src_file_name='" + new_name + "', compress_name='" + compress_name + "' where src_file_name='" + file_name + "'";
        std::cout << sql << std::endl;
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "Update " << file_name  << " compress_name failed!" << std::endl;
            return false;
        }

        return true;
    }

    bool Update_time(const std::string& file_name, const std::string& file_time) {
        std::string sql = "update compress set last_modify_time='" + file_time + "', compress_name='" + file_name +  "' where src_file_name='" + file_name + "'";
        std::cout << sql << std::endl;
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "update_time " << file_name  << " failed!" << std::endl;
            return false;
        }

        return true;
    }
private:
    MYSQL conn;
};
