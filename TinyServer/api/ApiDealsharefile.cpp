#include "ApiDealsharefile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/time.h>
#include "comm.h"

int decodeDealsharefileJson(string &str_json, string &user_name, string &md5, string &filename)
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

    if (root["user"].isNull())
    {
        LOG_ERROR("user null\n");
        return -1;
    }
    user_name = root["user"].asString();

    if (root["md5"].isNull())
    {
        LOG_ERROR("md5 null\n");
        return -1;
    }
    md5 = root["md5"].asString();

    if (root["filename"].isNull())
    {
        LOG_ERROR("filename null\n");
        return -1;
    }
    filename = root["filename"].asString();

    return 0;
}

int encodeDealsharefileJson(int ret, string &str_json)
{
    Json::Value root;
    if(ret==18)
    {
        //取消分享成功
       root["code"] = "018";
    }
    else if(ret==19)
    {
        //取消分享失败
       root["code"] = "019";
    }
    else if(ret==20)
    {
        //转存成功
       root["code"] = "020";
    }
    else if(ret==21)
    {
        //文件已存在
       root["code"] = "021";
    }
    else if(ret==22)
    {
        //转存失败
       root["code"] = "022";
    }

    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}



//取消分享文件
int handleCancelShareFile(string &user_name, string &md5, string &filename)
{
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    char fileid[1024] = {0};
    int count = 0;
    int ret2;

    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);

    //===1、mysql记录操作
    // sql语句
    sprintf(sql_cmd, "update user_file_list set shared_status = 0 where user = '%s' and md5 = '%s' and file_name = '%s'", user_name.c_str(), md5.c_str(), filename.c_str());
    cout << "执行: " << sql_cmd<<endl;
    if (!pDBConn->ExecuteUpdate(sql_cmd, false))
    {
         LOG_ERROR(sql_cmd , " 操作失败");
        ret = -1;
        goto END;
    }

    //查询共享文件数量
    sprintf(sql_cmd, "select count from user_file_count where user = '%s'", "share_file_count");
    cout << "执行: " << sql_cmd<<endl;
    count = 0;
    ret2 = GetResultOneCount(pDBConn, sql_cmd, count); //执行sql语句
    if (ret2 != 0)
    {
        LOG_ERROR(sql_cmd , " 操作失败");
        ret = -1;
        goto END;
    }

    //更新用户文件数量count字段
    if (count >= 1)
    {
        sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count - 1, "share_file_count");
         cout << "执行: " << sql_cmd<<endl;
        if (!pDBConn->ExecutePassQuery(sql_cmd))
        {
            LOG_ERROR(sql_cmd , " 操作失败");
            ret = -1;
            goto END;
        }
    }
    else
    {
        LOG_WARN ( "出现异常, count:%d " , count);
    }
   

    //删除在共享列表的数据
    sprintf(sql_cmd, "delete from share_file_list where user = '%s' and md5 = '%s' and file_name = '%s'", user_name.c_str(), md5.c_str(), filename.c_str());
    cout << "执行: " << sql_cmd << ", ret =" << ret<<endl;
    if (!pDBConn->ExecuteDrop(sql_cmd))
    {
         LOG_ERROR(sql_cmd , " 操作失败");
        ret = -1;
        goto END;
    }

END:
    /*
    取消分享：
        成功：{"code": 0}
        失败：{"code": 1}
    */
    if (ret == 0)
    {
        return 18;
    }
    else
    {
        return 19;
    }
}

//转存文件
//返回值：0成功，-1转存失败，-2文件已存在
int handleSaveFile(string &user_name, string &md5, string &filename)
{
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    int ret2 = 0;
    //当前时间戳
    struct timeval tv;
    struct tm *ptm;
    char time_str[128];
    int count;
    CDBManager *pDBManager = CDBManager::getInstance();
    CDBConn *pDBConn = pDBManager->GetDBConn("cloudserver");
    AUTO_REAL_DBCONN(pDBManager, pDBConn);

    //查看此用户，文件名和md5是否存在，如果存在说明此文件存在
    sprintf(sql_cmd, "select * from user_file_list where user = '%s' and md5 = '%s' and file_name = '%s'", user_name.c_str(), md5.c_str(), filename.c_str());
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    count = -1;
    ret2 = GetResultOneCount(pDBConn, sql_cmd, count); //执行sql语句, 最后一个参数为NULL
    if (ret2 == 2)                                      //如果有结果，说明此用户已有此文件
    {
        cout << "user_name: " << user_name << ", filename: " << filename << ", md5: " << md5 << ", 已存在"<<endl;
        ret = -2; //返回-2错误码
        goto END;
    }

    //文件信息表，查找该文件的计数器
    sprintf(sql_cmd, "select count from file_info where md5 = '%s'", md5.c_str());
    count = 0;
    ret2 = GetResultOneCount(pDBConn, sql_cmd, count); //执行sql语句
    if (ret2 != 0)
    {
         LOG_ERROR(sql_cmd , " 操作失败");
        ret = -1;
        goto END;
    }

    // 1、修改file_info中的count字段，+1 （count 文件引用计数）
    sprintf(sql_cmd, "update file_info set count = %d where md5 = '%s'", count + 1, md5.c_str());
    if (!pDBConn->ExecuteUpdate(sql_cmd))
    {
         LOG_ERROR(sql_cmd , " 操作失败");
        ret = -1;
        goto END;
    }

    // 2、user_file_list插入一条数据

    //使用函数gettimeofday()函数来得到时间。它的精度可以达到微妙
    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec); //把从1970-1-1零点零分到当前时间系统所偏移的秒数时间转换为本地时间
    // strftime() 函数根据区域设置格式化本地时间/日期，函数的功能将时间格式化，或者说格式化一个时间字符串
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", ptm);

    // sql语句
    /*
    -- =============================================== 用户文件列表
    -- user	文件所属用户
    -- md5 文件md5
    -- create_time 文件创建时间
    -- file_name 文件名字
    -- shared_status 共享状态, 0为没有共享， 1为共享
    -- pv 文件下载量，默认值为0，下载一次加1
    */
    sprintf(sql_cmd, "insert into user_file_list(user, md5, create_time, file_name, shared_status, pv) values ('%s', '%s', '%s', '%s', %d, %d)",
            user_name.c_str(), md5.c_str(), time_str, filename.c_str(), 0, 0);
    if (!pDBConn->ExecuteCreate(sql_cmd))
    {
         LOG_ERROR(sql_cmd , " 操作失败");
        ret = -1;
        goto END;
    }

    // 3、查询用户文件数量，更新该字段
    sprintf(sql_cmd, "select count from user_file_count where user = '%s'", user_name.c_str());
    count = 0;
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    ret2 = GetResultOneCount(pDBConn, sql_cmd, count); //指向sql语句
    if (ret2 == 1)                                      //没有记录
    {
        //插入记录
        sprintf(sql_cmd, " insert into user_file_count (user, count) values('%s', %d)", user_name.c_str(), 1);
        if (!pDBConn->ExecuteCreate(sql_cmd))
        {
           LOG_ERROR(sql_cmd , " 操作失败");
            ret = -1;
            goto END;
        }
    }
    else if (ret2 == 0)
    {
        //更新用户文件数量count字段
        sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count + 1, user_name.c_str());
        if (!pDBConn->ExecuteUpdate(sql_cmd))
        {
            LOG_ERROR(sql_cmd , " 操作失败");
            ret = -1;
            goto END;
        }
    }

END:
    /*
    返回值：0成功，-1转存失败，-2文件已存在
    转存文件：
        成功：{"code":0}
        文件已存在：{"code":5}
        失败：{"code":1}
    */
    if (ret == 0)
    {
        return 20;
    }
    else if (ret == -1)
    {
        return 22;
    }
    else if (ret == -2)
    {
        return 21;
    }
    return 0;
}

int ApiDealsharefile(string &url, string &post_data, string &str_json)
{
    char cmd[20];
    string user_name;
    string token;
    string md5;      //文件md5码
    string filename; //文件名字
    int ret = 0;

    //解析命令
    QueryParseKeyValue(url.c_str(), "cmd", cmd, NULL);
    
    ret = decodeDealsharefileJson(post_data, user_name, md5, filename);
    cout << "cmd: " << cmd << ", user_name:" << user_name << ", md5:" << md5 << ", filename:" << filename<<endl;
    if (ret != 0)
    {
        encodeDealsharefileJson(HTTP_RESP_FAIL, str_json);  
        return 0;
    }
    ret = 0;
    if (strcmp(cmd, "cancel") == 0) //取消分享文件
    {
        ret = handleCancelShareFile(user_name, md5, filename);
    }
    else if (strcmp(cmd, "save") == 0) //转存文件
    {
        ret = handleSaveFile(user_name, md5, filename);
    }

    
    encodeDealsharefileJson(ret, str_json);

    return 0;
}
