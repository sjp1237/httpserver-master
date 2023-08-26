#include "ApiMyfiles.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include "comm.h"
#include "ApiMyfiles.h"
#include "jsoncpp/json/json.h"
#include<memory>

//解析的json包, 登陆token
template <typename... Args>
static std::string formatString(const std::string &format, Args... args)
{
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
int decodeCountJson(string &str_json, string &user_name, string &token)
{
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res)
    {
        LOG_ERROR ("parse reg json failed ");
        return -1;
    }
    int ret = 0;

    // 用户名
    if (root["user"].isNull())
    {
        LOG_ERROR("user null\n");
        return -1;
    }
    user_name = root["user"].asString();

    //密码
    if (root["token"].isNull())
    {
        LOG_ERROR ( "token null\n");
        return -1;
    }
    token = root["token"].asString();

    return ret;
}

int encodeCountJson(int ret, int total, string &str_json)
{
    Json::Value root;
    if(ret==0)
    {
        //获取文件成功
        root["code"] = "110";
        root["num"] = total; // 正常返回的时候才写入token
    }
   else if (ret == 1)
    {
       root["code"] = "111";//登录失败
    }

    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}

//获取用户文件个数
int getUserFilesCount(string &user_name, int &count)
{
    int ret = 0;
    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);
    // 先查看用户是否存在
    string str_sql;

    str_sql = formatString("select `count` from user_file_count where user='%s'", user_name.c_str());
    CResultSet *pResultSet = pDBConn->ExecuteQuery(str_sql.c_str());
    if (pResultSet && pResultSet->Next())
    {
        // 存在在返回
        count = pResultSet->GetInt("count");
        cout << "count: " << count<<endl;
        ret = 0;
        delete pResultSet;
    }
    else if (!pResultSet)
    { // 操作失败
        LOG_ERROR (str_sql.c_str(),"操作失败");
        ret = -1;
    }
    else
    {
        // 没有记录则初始化记录数量为0
        ret = 0;
        count = 0;
        // 创建
        str_sql = formatString("insert into user_file_count (user, count) values('%s', %d)", user_name.c_str(), 0);
        if (!pDBConn->ExecuteCreate(str_sql.c_str()))
        { // 操作失败
            LOG_ERROR (str_sql.c_str() ,"操作失败");
            ret = -1;
        }
    }
    return ret;
}

int handleUserFilesCount(string &user_name, int &count)
{
    int ret = getUserFilesCount(user_name, count);
    return ret;
}

//解析的json包
// 参数
// {
// "count": 2,
// "start": 0,
// "token": "3a58ca22317e637797f8bcad5c047446",
// "user": "qingfu"
// }
int decodeFileslistJson(string &str_json, string &user_name, string &token, int &start, int &count)
{
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res)
    {
        LOG_ERROR ( "parse reg json failed ");
        return -1;
    }
    int ret = 0;

    // 用户名
    if (root["user"].isNull())
    {
        LOG_ERROR ( "user null\n");
        return -1;
    }
    user_name = root["user"].asString();

    //密码
    if (root["token"].isNull())
    {
        LOG_ERROR("token null\n");
        return -1;
    }
    token = root["token"].asString();

    if (root["start"].isNull())
    {
        LOG_ERROR("start null\n");
        return -1;
    }
    start = root["start"].asInt();

    if (root["count"].isNull())
    {
        LOG_ERROR ("count null\n");
        return -1;
    }
    count = root["count"].asInt();

    return ret;
}



int getUserFileList(string &cmd, string &user_name, int &start, int &count, string &str_json)
{
    int ret = 0;

    string str_sql;
    cout << "getUserFileList into"<<endl;

    int total = 0;
    ret = getUserFilesCount(user_name, total);
    if(ret < 0) {
        LOG_ERROR("getUserFilesCount failed");
        return -1;
    }


    //多表指定行范围查询
    if (cmd == "normal") //获取用户文件信息
    {
        //联合查表，从file_info，user_file_list中获取 user中所有 的文件信息，查找count个文件信息
        str_sql = formatString("select user_file_list.*, file_info.url, file_info.size,  file_info.type from file_info, user_file_list where user = '%s' \
            and file_info.md5 = user_file_list.md5 limit %d, %d",
                               user_name.c_str(), start, count);
    }
    else if (cmd == "pvasc") //按下载量升序
    {
        // sql语句
        str_sql = formatString("select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, \
         user_file_list where user = '%s' and file_info.md5 = user_file_list.md5  order by pv asc limit %d, %d",
                               user_name.c_str(), start, count);
    }
    else if (cmd == "pvdesc") //按下载量降序
    {
        // sql语句
        str_sql = formatString("select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, \
            user_file_list where user = '%s' and file_info.md5 = user_file_list.md5 order by pv desc limit %d, %d",
                               user_name.c_str(), start, count);
    }
    else
    {
        LOG_ERROR("unknown cmd: " , cmd.c_str());
        return -1;
    }

    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);
    cout << "执行: " << str_sql<<endl;
    CResultSet *pResultSet = pDBConn->ExecuteQuery(str_sql.c_str());
    if (pResultSet)
    {
        // 遍历所有的内容
        // 获取大小
        int file_index = 0;
        Json::Value root, files;

        while (pResultSet->Next())
        {
            Json::Value file;
            file["user"] = pResultSet->GetString("user");
            file["md5"] = pResultSet->GetString("md5");
            file["create_time"] = pResultSet->GetString("create_time");
            file["file_name"] = pResultSet->GetString("file_name");
            file["share_status"] = pResultSet->GetInt("shared_status");
            file["pv"] = pResultSet->GetInt("pv");
            file["url"] = pResultSet->GetString("url");
            file["size"] = pResultSet->GetInt("size");
            file["type"] = pResultSet->GetString("type");
            files[file_index] = file;
            file_index++;
        }
        root["files"] = files;

        Json::FastWriter writer;
        str_json = writer.write(root);
        // std::cout << "str_json:" << str_json;
        delete pResultSet;
        return 0;
    }
    else
    {
        LOG_ERROR (str_sql.c_str() , " 操作失败");
        return -1;
    }
}

int ApiMyfiles(string &url, string &post_data, string &str_json)
{
    // 解析url有没有命令

    // count 获取用户文件个数
    // display 获取用户文件信息，展示到前端
    char cmd[20];
    string user_name;
    string token;
    int ret = 0;
    int start = 0; //文件起点
    int count = 0; //文件个数
   

    //解析命令 解析url获取自定义参数
    QueryParseKeyValue(url.c_str(), "cmd", cmd, NULL);
    cout << "url: " << url << ", cmd = " << cmd<<endl;

    if (strcmp(cmd, "count") == 0)
    {
         //获取用户文件信息 127.0.0.1:80/api/myfiles&cmd=count
        // 解析json
        if (decodeCountJson(post_data, user_name, token) < 0)
        {
            encodeCountJson(1, 0, str_json);
            LOG_ERROR( "decodeCountJson failed");
            return -1;
        }
        //验证登陆token，成功返回0，失败-1
        ret = VerifyToken(user_name, token); // util_cgi.h
        if (ret == 0)
        {
            // 获取文件数量
            if (handleUserFilesCount(user_name, count) < 0)
            { //获取用户文件个数
                LOG_ERROR("handleUserFilesCount failed");
                encodeCountJson(1, 0, str_json);
            }
            else
            {
                cout << "handleUserFilesCount ok, count:" << count<<endl;
                encodeCountJson(0, count, str_json);
            }
        }
        else
        {
            LOG_ERROR ("VerifyToken failed");
            encodeCountJson(1, 0, str_json);
        }
        return 0;
    }
    else 
    {
        if( (strcmp(cmd, "normal") != 0) 
            && (strcmp(cmd, "pvasc") != 0) 
            && (strcmp(cmd, "pvdesc") != 0) )
        {
             LOG_ERROR("unknow cmd: " ,cmd);
            encodeCountJson(1, 0, str_json);
        }
        //获取用户文件信息 127.0.0.1:80/api/myfiles&cmd=normal
        //按下载量升序 127.0.0.1:80/api/myfiles?cmd=pvasc
        //按下载量降序127.0.0.1:80/api/myfiles?cmd=pvdesc
        ret = decodeFileslistJson(post_data, user_name, token, start, count); //通过json包获取信息
        cout << "user_name:" << user_name << ", token:" << token << ", start:" << start << ", count:" << endl;
        if (ret == 0)
        {
            //验证登陆token，成功返回0，失败-1
            ret = VerifyToken(user_name, token); // util_cgi.h
            if (ret == 0)
            {
                string str_cmd = cmd;
                if (getUserFileList(str_cmd, user_name, start, count, str_json) < 0)
                { //获取用户文件列表
                    LOG_ERROR ("getUserFileList failed");
                    encodeCountJson(1, 0, str_json);
                }
            }
            else
            {
                LOG_ERROR ( "VerifyToken failed");
                encodeCountJson(1, 0, str_json);
            }
        }
        else
        {
            LOG_ERROR ("decodeFileslistJson failed");
            encodeCountJson(1, 0, str_json);
        }
    }  

    return 0;
}