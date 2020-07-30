#include <mysql/mysql.h>
#include <string>
#include <map>
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

    bool Insert(const std::string& src_name, const std::string& dst_name) {
        std::string sql = "insert into compress values('" + src_name + "', '" + dst_name + "')";
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
            std::cout << "query failed" << std::endl;
            return false;
        }

        int row = mysql_num_rows(res_ptr);

        for (int i = 0; i < row; ++i) {
            MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
            data.insert(std::make_pair(result_row[0], result_row[1]));
        }

        mysql_free_result(res_ptr);
        return true;
    }

    bool Query(const std::string& file_name) {
        std::string sql = "select * from where src_file_name='" + file_name + "'";
        int res = mysql_query(&conn, sql.c_str());
        if (res) {
            std::cout << "query " << file_name <<  " failed" << std::endl;
            return false;
        }
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
            std::cout << "delete " << file_name  << " failed!" << std::endl;
            return false;
        }

        return true;
    }
private:
    MYSQL conn;
};
