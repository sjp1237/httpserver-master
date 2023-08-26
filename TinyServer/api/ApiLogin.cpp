#include "ApiLogin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<memory>
#include <time.h>
#include "comm.h"


#define LOGIN_RET_OK 0   // 成功
#define LOGIN_RET_FAIL 1 // 失败


string RandomString(const int len) /*参数为字符串的长度*/
{
    /*初始化*/
    string str; /*声明用来保存随机字符串的str*/
    char c;     /*声明字符c，用来保存随机生成的字符*/
    int idx;    /*用来循环的变量*/
    /*循环向字符串中添加随机生成的字符*/
    for (idx = 0; idx < len; idx++)
    {
        /*rand()%26是取余，余数为0~25加上'a',就是字母a~z,详见asc码表*/
        c = 'a' + rand() % 26;
        str.push_back(c); /*push_back()是string类尾插函数。这里插入随机字符c*/
    }
    return str; /*返回生成的随机字符串*/
}

// 解析登录信息
int decodeLoginJson(const std::string &str_json, string &user_name, string &pwd)
{
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res)
    {
        cout << "parse reg json failed ";
        return -1;
    }
    // 用户名
    if (root["user"].isNull())
    {
        cout << "user null\n";
        return -1;
    }
    user_name = root["user"].asString();

    //密码
    if (root["pwd"].isNull())
    {
        cout << "pwd null\n";
        return -1;
    }
    pwd = root["pwd"].asString();

    return 0;
}

// 封装登录结果的json
int encodeLoginJson(int ret, string &token, string &str_json)
{
    Json::Value root;
    if(ret==-1){
     //登录失败
     root["code"] = "001";
    }

    if (ret == 0)
    {
        root["code"] = "000";
        root["token"] = token; // 正常返回的时候才写入token
    }
    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}

template <typename... Args>
std::string formatString1(const std::string &format, Args... args)
{
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}


//验证账号密码
int verifyUserPassword(string &user_name, string &pwd)
{
    int ret = 0;
    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);

    // 先查看用户是否存在
    string strSql;

    strSql = formatString1("select password from user_info where user_name='%s'", user_name.c_str());
    CResultSet *pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
    uint32_t nowtime = time(NULL);
    if (pResultSet && pResultSet->Next())
    {
        // 存在在返回
        string password = pResultSet->GetString("password");
        LOG_DEBUG("mysql-pwd: %s", password.c_str());
        LOG_DEBUG(" user-pwd: %s",pwd.c_str());
        if (pResultSet->GetString("password") == pwd)
            ret = 0;
        else
            ret = -1;
    }
    else
    { // 如果不存在则注册
        ret = -1;
    }

    delete pResultSet;

    return ret;
}

/* -------------------------------------------*/
/**
 * brief  生成token字符串, 保存redis数据库
 *
 * username 		用户名
 * token     生成的token字符串
 *
 * returns
 *      成功: 0
 *      失败：-1
 */
/* -------------------------------------------*/
int setToken(string &user_name, string &token)
{
    int ret = 0;
    CacheManager *pCacheManager = CacheManager::getInstance();
    // increase message count
    CacheConn *pCacheConn = pCacheManager->GetCacheConn("token");
    AUTO_REAL_CACHECONN(pCacheManager, pCacheConn);

    token = RandomString(32);//随机生成32位的字符串

    if (pCacheConn)
    {
        //用户名：token, 86400有效时间为24小时
        pCacheConn->setex(user_name, 86400, token);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int ApiUserLogin(string &url, string &post_data, string &str_json)
{
    UNUSED(url);
    string user_name;
    string pwd;
    string token;
    // 判断数据是否为空
    if (post_data.empty())
    {
        return -1;
    }
    // 解析json
    if (decodeLoginJson(post_data, user_name, pwd) < 0)
    {
        cout << "decodeRegisterJson failed";
        encodeLoginJson(1, token, str_json);
        return -1;
    }

    // 验证账号和密码是否匹配
    if (verifyUserPassword(user_name, pwd) < 0)
    {
        cout << "verifyUserPassword failed";
        encodeLoginJson(1, token, str_json);
        return -1;
    }

    // 生成token
    if (setToken(user_name, token) < 0)
    {
        cout << "setToken failed";
        encodeLoginJson(1, token, str_json);
        return -1;
    }
    // 注册账号
    // ret = registerUser(user_name, nick_name, pwd, phone, email);
    encodeLoginJson(0, token, str_json);
    return 0;
}


