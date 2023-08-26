#pragma once

#include<iostream>
#include"../httpconn/httpconn.hpp"
#include"../MysqlPool/DBPool.h"
#include "jsoncpp/json/json.h"
#include "../RedisPool/CachePool.h"
#include"../log/log.h"
#include<string.h>
#define FILE_NAME_LEN (256)    //文件名字长度
#define TEMP_BUF_MAX_LEN (512) //临时缓冲区大小
#define FILE_URL_LEN (512)     //文件所存放storage的host_name长度
#define HOST_NAME_LEN (30)     //主机ip地址长度
#define USER_NAME_LEN (128)    //用户名字长度
#define TOKEN_LEN (128)        //登陆token长度
#define MD5_LEN (256)          //文件md5长度
#define PWD_LEN (256)          //密码长度
#define TIME_STRING_LEN (25)   //时间戳长度
#define SUFFIX_LEN (8)         //后缀名长度
#define PIC_NAME_LEN (10)      //图片资源名字长度
#define PIC_URL_LEN (256)      //图片资源url名字长度

#define HTTP_RESP_OK 0
#define HTTP_RESP_FAIL 1           //
#define HTTP_RESP_USER_EXIST 2     // 用户存在
#define HTTP_RESP_DEALFILE_EXIST 3 // 别人已经分享此文件
#define HTTP_RESP_TOKEN_ERR 4      //  token验证失败
#define HTTP_RESP_FILE_EXIST 5     //个人已经存储了该文件
#define SQL_MAX_LEN (512) // sql语句长度

#define FILE_PUBLIC_ZSET "file_public_zset"
#define FILE_NAME_HASH "file_name_hash"

#define UNUSED(expr)  \
    do                \
    {                 \
        (void)(expr); \
    } while (0)

    
int handle_business(Request* request,Response* response);


//处理数据库查询结果，结果集保存在buf，只处理一条记录，一个字段, 如果buf为NULL，无需保存结果集，只做判断有没有此记录
//返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
int GetResultOneCount(CDBConn *pDBConn, char *sql_cmd, int &count);
int GetResultOneStatus(CDBConn *pDBConn, char *sql_cmd, int &shared_status);

// 检测是否存在记录，-1 操作失败，0:没有记录， 1:有记录
int CheckwhetherHaveRecord(CDBConn *pDBConn, char *sql_cmd);


int VerifyToken(string &user_name, string &token);


//通过文件名file_name， 得到文件后缀字符串, 保存在suffix 如果非法文件后缀,返回"null"
int GetFileSuffix(const char *file_name, char *suffix);

int TrimSpace(char *inbuf);
int QueryParseKeyValue(const char *query, const char *key, char *value, int *value_len_p);