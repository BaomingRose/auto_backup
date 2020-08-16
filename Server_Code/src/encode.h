#include <iconv.h>
#include <assert.h>
#include <stdio.h>
#include <cstring>

/**********************************************************************
 *  函数名称： UTF8ToGB2312
 *  功能描述： UTF-8 字符串转换 GB2312 函数
 *  访问的表： 无
 *  修改的表： 无
 *  输入参数：
 *  char* srcStr，UTF-8字符串
 *  输出参数：
 *  char* desBuff：接收GB2312转换结果的缓存；
 *  int desBuffLength：指定接收转换结果的缓存大小
 *  返 回 值： 转换大小
 ************************************************************************/
int UTF8ToGB2312(char* srcStr, char* desBuff, size_t desBuffLength) {
    assert(strlen(srcStr) > 0);
    size_t iLen = strlen(srcStr);

    iconv_t cd;
    cd = iconv_open("gb2312","utf-8");
    assert(cd != 0);
    iconv(cd, &srcStr, &iLen, &desBuff, &desBuffLength);
    iconv_close(cd);

    return desBuffLength;
}

/**********************************************************************
 * 函数名称： GB2312ToUTF8
 * 功能描述： GB2312 字符串转换 UTF-8 函数
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数：
 * char* srcStr，GB2312字符串
 * 输出参数：
 * char* desBuff：接收UTF-8转换结果的缓存；
 * int desBuffLength：指定接收转换结果的缓存大小
 * 返 回 值： 转换大小
 ************************************************************************/
int GB2312ToUTF8(char* srcStr, char* desBuff, size_t desBuffLength) {
    assert(strlen(srcStr) > 0);
    size_t iLen = strlen(srcStr);

    iconv_t cd;
    cd = iconv_open("utf-8","gb2312");
    assert(cd != 0);
    iconv(cd, &srcStr, &iLen, &desBuff, &desBuffLength);
    iconv_close(cd);

    return desBuffLength;
}
