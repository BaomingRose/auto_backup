# auto_backup
### 功能简介
该项目的功能为：该项目实现了一个类似云盘的功能。将Windows想要备份的文件放到指定监听备份路径下，将自动备份至服务端；可以通过浏览器访问url的形式获取到已经备份文件的列表，并可以下载；其中，为节省服务端的空间，服务端会有一个操作：将不常用（非热点）文件压缩，等再次被访问的时候才解压缩。
### 流程设计

客户端主要流程：
（下图实际可以简单的看做一条竖直线，我不想让图片看起来很长，才折过来一部分）
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200803203437762.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)

服务端网络监听处理流程：
![在这里插入图片描述](https://img-blog.csdnimg.cn/2020080321111589.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)

服务端不常用文件压缩流程图：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200803205025561.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)

### 代码设计
写代码不能盲目，在我看来，设计好模块很重要，如果能够写出易读性、复用性、扩展性都很好的代码，当然舒服……
首先根据上述流程划分好模块：
#### 服务端设计
服务端主要有两个模块，一个是网络通信模块，一个是不常用文件压缩模块。在压缩模块中，我们需要保留文件名称和压缩后名称的映射，如果放入内存中，不是持久存储，而且内存很宝贵，不如让数据库帮我们存。所以做如下持久层设计，包括对这个记录的增、删、查、改。（这里粘部分代码，因为后面分析中客户端要判断文件是否被修改的文件，是否要重新上传，所以服务端也要保存文件的最后修改时间和大小，所以我又给这个table添加了一个文件时间的字段）。
##### 持久层
创建表的sql：
```sql
create table(
	src_name varchar(50) primary key,
	compress_name varchar(50) not null,
	file_time varchar(30) not null
);
```
持久层封装代码：

```cpp
class DataManager {
public:
    DataManager() {
        mysql_init(&conn);
    }

    bool Connect() {
        if (mysql_real_connect(&conn, "localhost", "root", "xxx", "database", 0, NULL, CLIENT_FOUND_ROWS)) {
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

	//查询文件是否存在
    bool Query(const std::string& file_name);
	//返回所有文件以及修改时间的映射
    bool Query_all_with_time(std::map<std::string, std::string>& src_with_time);
	//判断是否被压缩
    bool isCompress(const std::string& file_name);
	//返回没有被压缩列表
    bool getNoCompress(std::vector<std::string>& no_compress);
	//返回压缩文件的名字
    bool getCompressName(const std::string& src_name, std::string& compress_name);
    //更新压缩名字
    bool Update(const std::string& file_name , const std::string& new_name,  const std::string& compress_name);
	//更新修改时间
    bool Update_time(const std::string& file_name, const std::string& file_time);
private:
    MYSQL conn;
};
```
##### 文件压缩工具
文件压缩与解压缩只是两个功能，并不需要实力对象来调用，但是为了代码易读，所以封装了一个类来封装成为两个静态方法，说白了，这个类就是给两个方法啊限制作用域，并不打算让这个类实力对象。这里的压缩功能使用了一个zlib开发包，使用`yum install zlib zlib-devel -y`来获取，代码如下：
```cpp
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
        while ((ret = gzread(gf, tmp, 4096)) > 0)
            ofs.write(tmp, ret);
        ofs.close();
        gzclose(gf);
        return true;
    }
};
```
##### 不常用文件压缩
这个模块是一个流程模块，是整个压缩流程的逻辑处理，只需要启动Start()即可，这个代码就是对上述流程图的说明：

```cpp
class NonHotCompress {
public:
    NonHotCompress() {}

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
        if ((cur_t - st.st_atim.tv_sec) > NONHOT_TIME)
            return false;
        return true;
    }
public:
    static DataManager* _dm;
};
```
##### 网络通信模块
这里使用了人家已经写好了的cpp-httplib.h，封装了http协议，用起来很方便，这个库的GitHub地址：[https://github.com/yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)，使用上我就不多做说明了，我的使用：
(逻辑处理代码没有粘，处理的时候逻辑层和视图层没有分离，所以代码较多)
```cpp
class Server {
public:
    bool Start() {
        _server.Put("/rose_backup/upload/(.*)", Upload);
        _server.Get("/rose_backup/list", List);
        _server.Get("/rose_backup/download/(.*)", DownLoad);
        _server.Get("/rose_backup/delete/(.*)", Delete);
        _server.Get("/rose_backup/get_all", Get_all_src_with_time);
        _server.listen("0.0.0.0", 8080);
        return true;
    }

private:
    //文件上传处理函数
    static void Upload(const httplib::Request& req, httplib::Response& rsp);
    //文件列表处理函数
    static void List(const httplib::Request& req, httplib::Response& rsp);
    //文件下载处理函数
    static void DownLoad(const httplib::Request& req, httplib::Response& rsp);
    //删除请求
    static void Delete(const httplib::Request& req, httplib::Response& rsp);
    //客户端初始化时的请求
    static void Get_all_src_with_time(const httplib::Request& req, httplib::Response& rsp);
private:
    std::string _file_dir;  //文件上传备份路径
    httplib::Server _server;
public:
    static DataManager* _dm;
};
```
##### 概括总流程
因为服务端的mysql连接对象没有必要建立过多，所以只在程序开始创建一个连接对象，然后两个类公用这两个对象，所以简单的设计成一个连接对象的全局变量，然后让两个对象的连接对象指针成员指向这个连接对象。当然，我这客户端很少，连接对象也没有搞很多，如果客户量很大，你也可以实现一个连接池。不常用文件压缩和网络通信模块，设置为两个线程，主流程代码如下：（我不想讲日志打印到屏幕，而是将其打印到日志文件，所以使用了文件描述符重定向）

```cpp
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
    if (fd == -1)
        std::cout << "打开log.txt日志文件失败！" << std::endl;
    dup2(fd, 1);
    if (boost::filesystem::exists(GZFILE_DIR) == false)
        boost::filesystem::create_directory(GZFILE_DIR);    
    if (boost::filesystem::exists(BACKUP_DIR) == false)
        boost::filesystem::create_directory(BACKUP_DIR);
    dm.Connect();
    thread t1(testHttp);
    thread t2(testCompress); 
    t1.join();
    t2.join();
    dm.Close();
    close(fd);
    return 0;
}
```
##### 客户端代码设计
客户端不再说明，流程图已经很清晰。这里说一说客户端服务端都有使用的boost库中的处理系统文件的内容。
##### boost库的使用
VS配置使用boost库很容易，下载boost之后，只需要添加图中两个路径即可，这里说明较简单，如果想配置可以自行搜索如何配置boost库。
![在这里插入图片描述](https://img-blog.csdnimg.cn/2020080422401443.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)

下列代码为遍历当前路径下所有文件，并判断是否是一个目录，以及获取文件最后修改时间以及文件大小的代码，如下：
```cpp
#include <boost/filesystem.hpp>                                                                
#include <iostream>
#include <string>
using namespace std;

void testFileTraverse() {
    string dir = "./";
    boost::filesystem::directory_iterator begin(dir);
    boost::filesystem::directory_iterator end;
    cout << "遍历 " << dir << " 下所有文件，目录提示:" << endl;
    for (auto it = begin; it != end; ++it) {
        //it->path() 获取文件boost::filesystem::path 对象
        string pathname = it->path().string();
        string name = it->path().filename().string();
        //判断文件是否为目录
        if (boost::filesystem::is_directory(it->status())) {
            cout << pathname << " is a directory!" << endl;
            continue;
        }
        cout << "文件路径为：" << pathname << "\t" << "文件名为：" << name << endl;
    }
}

void testFileLastTime() {    
    int64_t fsize = boost::filesystem::file_size("./testFileLastTime.cpp");
    time_t mtime = boost::filesystem::last_write_time("./testFileLastTime.cpp");
    cout << "文件大小为：" << to_string(fsize) << endl;
    cout << "文件最后修改时间戳为：" << to_string(mtime) << endl;
    struct tm* stm_ptr;
    stm_ptr = localtime(&mtime);
    char buf[1024];
    strftime(buf, 1024, "%Y-%m-%d %H:%M:%S", stm_ptr);
    cout << "文件最后被修改时间为：" << buf << endl;
}
```
### 调式中问题总概
一、服务端在接收到客户端上传的文件成功后，还压缩成功了，但是更新数据库失败，然后服务端根据未修改的数据库记录，始终尝试压缩此文件，但是已被压缩的文件已经不存在，服务端会错误。

错误原因：经过调试，数据库更新失败的原因是原来的文件名长度在给定范围内，但是压缩后文件名长度超出范围，文件名的字段长度给的不够，所以无法修改该记录，所以将文件名字段扩长了。

二、客户端更新unordered_map时，时间处理错误（忘记了关键的一句代码，就是`localtime(&time_t)`，忘记将time_t转换为struct tm类型，而直接使用未赋值的struct tm类型导致后面转换字符串异常）

```cpp
	//把上传之后的文件更改时间
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
```
这步是在上传文件成功的时候执行，所以服务端那边正常接收文件，但是客户端运行到此处崩溃。
### 可以完善内容
1. 可以增加管理用户模块，像云盘一样供用户使用
实现思路：
设计登录注册的MVC模式，持久层管理数据库中的用户信息，并可以实现连接池提升效率，然后为每个用户创建一个目录来保存用户需要备份的文件。

2. 可以将多文件压缩为一个文件

3. 文件下载时，当前服务端不支持断点续传，可以支持断点续传
实现思路：
当客户端下载中断时，记录当前文件中断的位置，继续下载的时候，将这个位置信息发送给服务端，服务端根据位置信息找到应该发送的文件位置继续传送。如下图的HTTP协议的请求与相应：

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200805232444707.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)

&nbsp;&nbsp;&nbsp;&nbsp;浏览器向服务端申请下载文件，服务端将文件版本标识符和允许文件范围传输的单位发送给客户端，然后客户端每次将这个标识符作为请求头发送给服务端，并且指定文件范围，下次每次服务端判断请求的服务端资源是否变动，如有变动则要重新传送，没有变动就把指定范围数据返回客户端，并且状态码设置为206表示范围数据传送成功。


4. 每次发送文件，都要将文件全部提到内存，文件过大并不合适——可以对文件分块传输，核心思想和断点续传类似
5. 前端界面可以改善
6. 当前使用http协议传输，明文传输，可以使用https或者其他加密传输
