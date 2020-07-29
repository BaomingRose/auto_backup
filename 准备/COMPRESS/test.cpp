#include "CompressUtil.hpp"

int main() {
    //CompressUtil::Compress("./tmp", "./tmp.zip");

    CompressUtil::Uncompress("./tmp.zip", "./tmp1");

    return 0;
}
