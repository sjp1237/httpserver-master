#include "ApiSharefiles.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#include <sys/time.h>
#include "comm.h"


template <typename... Args>
static std::string formatString(const std::string &format, Args... args)
{
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
//获取用户文件个数
int getShareFilesCount(CDBConn *pDBConn, int &count)
{
    int ret = 0;
    // 先查看用户是否存在
    string str_sql;

    str_sql = formatString("select count from user_file_count where user='%s'", "share_file_count");
    CResultSet *pResultSet = pDBConn->ExecuteQuery(str_sql.c_str());
    if (pResultSet && pResultSet->Next())
    {
        // 存在在返回
        count = pResultSet->GetInt("count");
        cout << "count: " << count<<endl;
        ret = 0;
    }
    else if (!pResultSet)
    { // 如果不存在则注册
        LOG_ERROR(str_sql.c_str() , " 操作失败");
        ret = -1;
    }
    else
    {
        delete pResultSet;
        // 没有记录则初始化记录数量为0
        ret = 0;
        count = 0;
        // 创建
        str_sql = formatString("insert into user_file_count (user, count) values('%s', %d)", "share_file_count", 0);
        if (!pDBConn->ExecuteCreate(str_sql.c_str()))
        {
            LOG_ERROR(str_sql.c_str() , " 操作失败");
            ret = -1;
        }
    }

    return ret;
}

//获取共享文件个数
int handleGetSharefilesCount(int &count)
{
    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);
    int ret = 0;
    ret = getShareFilesCount(pDBConn, count);

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
int decodeShareFileslistJson(string &str_json, int &start, int &count)
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

    if (root["start"].isNull())
    {
        LOG_ERROR ("start null\n");
        return -1;
    }
    start = root["start"].asInt();

    if (root["count"].isNull())
    {
        LOG_ERROR ("count null\n");
        return -1;
    }
    count = root["count"].asInt();

    return 0;
}


//获取共享文件列表
//获取用户文件信息 127.0.0.1:80/api/sharefiles&cmd=normal
void handleGetShareFilelist(int start, int count, string &str_json)
{
    int ret = 0;
    string str_sql;
    int total = 0;
    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);
    CResultSet *pResultSet = NULL;
    int file_count = 0;
    Json::Value root, files;

    ret = getShareFilesCount(pDBConn, total); // 获取文件总数量
    if (ret != 0)
    {
        LOG_ERROR ("getShareFilesCount err");
        ret = -1;
        goto END;
    }

    // sql语句
    str_sql = formatString("select share_file_list.*, file_info.url, file_info.size, file_info.type from file_info, \
        share_file_list where file_info.md5 = share_file_list.md5 limit %d, %d", start, count);
    cout << "执行: " << str_sql<<endl;
    pResultSet = pDBConn->ExecuteQuery(str_sql.c_str());
    if (pResultSet)
    {
        // 遍历所有的内容
        // 获取大小
        file_count =0;
        while (pResultSet->Next())
        {
            Json::Value file;
            file["user"] = pResultSet->GetString("user");
            file["md5"] = pResultSet->GetString("md5");
            file["file_name"] = pResultSet->GetString("file_name");
            file["share_status"] = pResultSet->GetInt("share_status");
            file["pv"] = pResultSet->GetInt("pv");
            file["create_time"] = pResultSet->GetString("create_time");
            file["url"] = pResultSet->GetString("url");
            file["size"] = pResultSet->GetInt("size");
            file["type"] = pResultSet->GetString("type");
            files[file_count] = file;
            file_count++;
        }
        if(file_count > 0)
            root["files"] = files;
        ret = 0;
        delete pResultSet;
    }
    else
    {
        ret = -1;
    }
END:
    if (ret == 0)
    {
        root["code"] = 0;
        root["total"] = total;
        root["count"] = file_count;
    }
    else
    {
        root["code"] = 1;
    }
    str_json = root.toStyledString();
}


int encodeSharefilesJson(int ret, int total, string &str_json)
{
    Json::Value root;

    if(ret==0)
    {
        root["num"] = total; // 正常返回的时候才写入token
         root["code"] = "110";
    }

    if (ret == 1)
    {
        root["code"] = "111";
    }
    Json::FastWriter writer;
    str_json = writer.write(root);
}

int ApiSharefiles(string &url, string &post_data, string &str_json)
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

    cout << " post_data: " <<  post_data.c_str()<<endl;

    //解析命令 解析url获取自定义参数
    QueryParseKeyValue(url.c_str(), "cmd", cmd, NULL);
    cout << "cmd = "<< cmd<<endl;

    if (strcmp(cmd, "count") == 0) // count 获取用户文件个数
    {
        // 解析json
        if (handleGetSharefilesCount(count) < 0)//获取共享文件个数
        { 
            encodeSharefilesJson(1, 0, str_json);
        } 
        else
        {
            //获取成功
            encodeSharefilesJson(0, count, str_json);
        }
        return 0;
    }
    else 
    {
        //获取共享文件信息 127.0.0.1:80/sharefiles&cmd=normal
        if(decodeShareFileslistJson(post_data, start, count) < 0) //通过json包获取信息
        {
            encodeSharefilesJson(1, 0, str_json);
            return 0;
        }
        if (strcmp(cmd, "normal") == 0)
        {
             handleGetShareFilelist(start, count, str_json);    // 获取共享文件
        }
        else 
        {
            encodeSharefilesJson(1, 0, str_json);
        }
    }
    return 0;
}