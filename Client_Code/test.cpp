#if 0
#define _CRT_SECURE_NO_WARNINGS
#include <boost/filesystem.hpp>
#include <iostream>
#include "Client.hpp"
using namespace std;


int main() {
#if 0
	string dir = "./";
	boost::filesystem::directory_iterator begin(dir);
	boost::filesystem::directory_iterator end;

	for (; begin != end; ++begin) {
		string pathname = begin->path().string();
		cout << pathname << endl;
	}
#endif

#if 0
	Client cl;
	cl.Init_map();
#endif

#if 0
	int64_t file_size;
	time_t file_time;
	Client::get_etag_from_str("100 2020-08-02 18:00:00", file_size, file_time);
	cout << file_time << endl;

	cout << time(NULL) << endl;
#endif
	if (boost::filesystem::exists(LISTEN_DIR) == false) {
		boost::filesystem::create_directory(LISTEN_DIR);
	}
	Client ci;
	ci.Start();

	return 0;
}
#endif