# zlib的使用

### CentOS 安装工具及开发包

```
sudo yum install zlib -y 
sudo yum install zlib-devel -y
```

### 引用头文件

```
#include <zlib.h>
```

### 安装boost库

```
sudo yum install boost -y
sudo yum install boost-devel -y
```

### boost库使用到的头文件

```
#include <boost/filesystem.hpp>
```

### 编译语句

```
test:test.cpp
	g++ -std=c++11 $^ -o $@ -lz -lboost_filesystem -lboost_system
```

