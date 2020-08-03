#pragma once
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include "httplib.h"
#include <Windows.h>

#define LISTEN_DIR "../listen_backup_dir/"
#if 0
//�����ip
#define svr_ip "192.168.70.133"
#endif
#if 1
//�Ʒ�����ip
#define svr_ip "182.92.194.63"
#endif
#define svr_port 10000

class FileUtil {
public:
	static bool Read(const std::string& fileName, std::string& body) {
		std::ifstream fs(fileName, std::ios::binary);
		if (fs.is_open() == false) {
			std::cout << " open file " <<  fileName << " error!"<< std::endl;
			return false;
		}
		int64_t fsize = boost::filesystem::file_size(fileName);
		body.resize(fsize);
		fs.read(&body[0], fsize);

		if (fs.good() == false) {
			std::cout << "" << fileName << " read error" << std::endl;
			fs.close();
			return false;
		}
		fs.close();
		return true;
	}

	static bool Write(const std::string& fileName, const std::string& body) {
		std::ofstream fs(fileName, std::ios::binary);
		if (fs.is_open() == false) {
			std::cout << "open file "<< fileName << "error" << std::endl;
			return false;
		}
		fs.write(&body[0], body.size());
		if (fs.good() == false) {
			std::cout <<  fileName << " write error" << std::endl;
			fs.close();
			return false;
		}
		fs.close();
		return true;
	}
};

class Client {
public:
	Client() {
		_client = new httplib::Client(svr_ip, svr_port);
	}

	~Client() {
		if (_client != nullptr) {
			delete _client;
			_client = nullptr;
		}
	}

	void Start() {
		Init_map();
		while (1) {
			//���������ļ���
			std::string dir = LISTEN_DIR;
			boost::filesystem::directory_iterator begin(dir);
			//end����ʲô��û�о���β��
			boost::filesystem::directory_iterator end;

			for (auto it = begin; it != end; ++it) {
				//������Ŀ¼
				if (boost::filesystem::is_directory(it->status()))
					continue;
				std::string pathname = it->path().string();
				std::string filename = it->path().filename().string();

				//��������ļ������ϴ�
				if (_file_etg.count(filename) < 1) {
					std::cout << filename << " ���ļ�ԭ��������" << std::endl;
					if (Upload(pathname, filename) == false) {
						std::cout << "upload " << filename << " failed!" << std::endl;
						continue;
					} else {
						std::cout << "upload " << filename << " success!" << std::endl;
					}
					continue;
				}
				//�������ļ���Ƚ�ʱ��
				time_t old_time;
				int64_t old_size;
				time_t new_time;
				int64_t new_size;
				get_etag(pathname, new_size, new_time);
				get_etag_from_str(_file_etg[filename], old_size, old_time);
				//����ļ���С���˻����ļ��޸�ʱ���ԭ��ʱ�䳤���ϴ�
				if (new_size != old_size || new_time > old_time) {
					if (new_size != old_size) {
						std::cout << "�´�С:" << new_size << std::endl;
						std::cout << "�ɴ�С:" << old_size << std::endl;
						std::cout << filename << "��С�б䶯" << std::endl;
					}
					if (new_time > old_time) {
						std::cout << "��ʱ��:" << new_time << std::endl;
						std::cout << "��ʱ��:" << old_time << std::endl;
						std::cout << filename << "ʱ���б䶯" << std::endl;
					}
					if (Upload(pathname, filename) == false) {
						std::cout << "upload " << filename << " failed!" << std::endl;
					} else {
						std::cout << "upload " << filename << " success!" << std::endl;
					}
				}
			}
			Sleep(3000);
		}
	}
private:
	bool Init_map() {
		auto rsp = _client->Get("/rose_backup/get_all");
		std::stringstream sistream;
		sistream << rsp->body;

		char buf[1024] = { 0 };
		while (sistream.getline(buf, 1024)) {
			char* ptr = buf;
			while (*ptr != '\t') {
				++ptr;
			}
			std::string file_name;
			file_name.assign(buf, ptr - buf);
			std::string file_info(ptr + 1);

			_file_etg.insert(std::make_pair(file_name, file_info));
		}

		for (const auto& e : _file_etg) {
			std::cout << e.first << "\t" << e.second << std::endl;
		}
		return true;
	}

	bool Upload(const std::string& pathname, const std::string& filename) {
		std::cout << "׼���ϴ��ļ�: " << filename << std::endl;
		std::string body;
		FileUtil::Read(pathname, body);
		std::string req_path = "/rose_backup/upload/" + filename;
		auto rsp = _client->Put(req_path.c_str(), body, "application/octet-stream");
		if (rsp == NULL || rsp->status != 200) {
			std::cout << "backup " << filename << " failed!" << std::endl;
			if (rsp != NULL) {
				std::cout << "return status:" << rsp->status << std::endl;
			}
			return false;
		}

		update_map(pathname, filename);

		return true;
	}

	//���ϴ�֮����ļ�����ʱ��
	void update_map(const std::string& pathname, const std::string& filename) {
		int64_t file_size;
		time_t file_time;
		get_etag(pathname, file_size, file_time);
		struct tm* tmp;
		tmp = localtime(&file_time);
		char buf[20];
		strftime(buf, 20, "%Y-%m-%d %H:%M:%S", tmp);
		std::string time(buf);

		std::string file_info = std::to_string(file_size) + " " + time;
		_file_etg[filename] = file_info;
	}

	//��ȡ�����ļ�·���Ĵ�С������޸�ʱ��
	static void get_etag(const std::string& pathname, int64_t& file_size, time_t& file_time) {
		file_size = boost::filesystem::file_size(pathname);
		file_time = boost::filesystem::last_write_time(pathname);
	}

	//�������˴�������ʱ���ַ������õ��ϴ�ʱ����ļ���С
	static void get_etag_from_str(const std::string& info, int64_t& file_size, time_t& file_time) {
		size_t index = info.find(' ');
		std::string fileSize = info.substr(0, index);
		file_size = std::stoi(fileSize);
		std::string fileTime = info.substr(index + 1);
		struct tm tmp;
		memset(&tmp, 0, sizeof(tm));
		int year;
		int month;
		sscanf(fileTime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &tmp.tm_mday, &tmp.tm_hour, &tmp.tm_min, &tmp.tm_sec);
		tmp.tm_year = year - 1900;
		tmp.tm_mon = month - 1;
		file_time = mktime(&tmp);
	}
private:
	std::unordered_map<std::string, std::string> _file_etg;
	httplib::Client* _client;
};

