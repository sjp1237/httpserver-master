#include"comm.h"
#include"ApiRegister.h"
#include"ApiLogin.h"
#include"ApiMd5.h"
#include"ApiUpload.h"
#include"ApiMyfiles.h"
#include"ApiDealfile.h"
#include"../log/log.h"
#include"../RedisPool/CachePool.h"
#include"ApiSharefiles.h"
#include"ApiDealsharefile.h"

//处理业务
int handle_business(Request* request,Response* response)
{
       string path=request->getPath();
    string post_data=request->getContent();
    if(request->getMethod()==Request::GET)
    {
        //get请求
        if(path=="/sharefiles"){
         cout<<"分享文件"<<endl;
         string res_content;
         string url=request->getUrl();
         ApiSharefiles(url,post_data,res_content);
         response->SetContent(res_content);
         response->SetCode("200");
        }
    }
    else if( request->getMethod()==Request::POST)
    {
        //post请求
        string post_data=request->getContent();
        // LOG_DEBUG("post data: %s",post_data.c_str());
        string path=request->getPath();
        if(path=="/register"){
            //用户注册
            string res_content;
            int ret=ApiRegisterUser(path,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
            return 0;
        }
        else if(path=="/login"){
            cout<<"登录功能";
            string res_content;
            ApiUserLogin(path,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
        }
        else if(path=="/myfiles"){
            cout<<"获取文件列表";
            string res_content;
             string url=request->getUrl();
            ApiMyfiles(url,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
        }

        else if(path=="/md5"){
            cout<<"秒传文件"<<endl;
            string res_content;
            ApiMd5(path,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
        }
        else if(path=="/upload"){
            cout<<"上传文件"<<endl;
            string res_content;
            ApiUpload(path,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
        }
        else if(path=="/sharefiles"){
         cout<<"获取文件列表"<<endl;
         string res_content;
         string url=request->getUrl();
         ApiSharefiles(url,post_data,res_content);
         response->SetContent(res_content);
         response->SetCode("200");
        }

        else if(path=="/dealfile"){
            string res_content;
            string url=request->getUrl();
            ApiDealfile(url,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
        }
        else if(path=="/dealsharefile"){
            cout<<"处理共享文件";
            string res_content;
            string url=request->getUrl();
            ApiDealsharefile(url,post_data,res_content);
            response->SetContent(res_content);
            response->SetCode("200");
        }
    }
}



//处理数据库查询结果，结果集保存在count，如果要读取count值则设置为0，如果设置为-1则不读取
//返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
int GetResultOneCount(CDBConn *pDBConn, char *sql_cmd, int &count)
{
    int ret = -1;
    CResultSet *pResultSet = pDBConn->ExecuteQuery(sql_cmd);

    if (!pResultSet)
    {
        ret = -1;
    }

    if (count == 0)
    {
        // 读取
        if (pResultSet->Next())
        {
            ret = 0;
            // 存在在返回
            count = pResultSet->GetInt("count");
            //LOG_DEBUG("count: %d" ,count);
        }
        else
        {
            ret = 1; // 没有记录
        }
    }
    else
    {
        if (pResultSet->Next())
        {
            ret = 2;
        }
        else
        {
            ret = 1; // 没有记录
        }
    }

    delete pResultSet;

    return ret;
}

int CheckwhetherHaveRecord(CDBConn *pDBConn, char *sql_cmd)
{
    int ret = -1;
    CResultSet *pResultSet = pDBConn->ExecuteQuery(sql_cmd);

    if (!pResultSet)
    {
        ret = -1;
    }
    else if (pResultSet && pResultSet->Next())
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    delete pResultSet;

    return ret;
}

int GetResultOneStatus(CDBConn *pDBConn, char *sql_cmd, int &shared_status)
{
    int ret = 0;
    int row_num = 0;

    CResultSet *pResultSet = pDBConn->ExecuteQuery(sql_cmd);

    if (!pResultSet)
    {
        LOG_ERROR("!pResultSet faled");
        ret = -1;
    }
    else
    {
        row_num = pDBConn->GetRowNum();
    }

    if (pResultSet->Next())
    {
        ret = 0;
        // 存在在返回
        shared_status = pResultSet->GetInt("shared_status");
       // LOG_INFO ("shared_status: %d" , shared_status);
    }
    else
    {
      //  cout << "pResultSet->Next()";
        ret = -1;
    }

    delete pResultSet;

    return ret;
}


//验证登陆token，成功返回0，失败-1
int VerifyToken(string &user_name, string &token)
{    
    int ret = 0;
    CacheManager *pCacheManager = CacheManager::getInstance();
    // increase message count
    CacheConn *pCacheConn = pCacheManager->GetCacheConn("token");
    AUTO_REAL_CACHECONN(pCacheManager, pCacheConn);

    if (pCacheConn)
    {
        string tmp_token = pCacheConn->get(user_name);
        if (tmp_token == token)
        {
            ret = 0;
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}



//通过文件名file_name， 得到文件后缀字符串, 保存在suffix 如果非法文件后缀,返回"null"
int GetFileSuffix(const char *file_name, char *suffix)
{
    const char *p = file_name;
    int len = 0;
    const char *q = NULL;
    const char *k = NULL;

    if (p == NULL)
    {
        return -1;
    }

    q = p;

    // mike.doc.png
    //              ↑

    while (*q != '\0')
    {
        q++;
    }

    k = q;
    while (*k != '.' && k != p)
    {
        k--;
    }

    if (*k == '.')
    {
        k++;
        len = q - k;

        if (len != 0)
        {
            strncpy(suffix, k, len);
            suffix[len] = '\0';
        }
        else
        {
            strncpy(suffix, "null", 5);
        }
    }
    else
    {
        strncpy(suffix, "null", 5);
    }

    return 0;
}

/**
 * @brief  去掉一个字符串两边的空白字符
 *
 * @param inbuf确保inbuf可修改
 *
 * @returns
 *      0 成功
 *      -1 失败
 */
int TrimSpace(char *inbuf)
{
    int i = 0;
    int j = strlen(inbuf) - 1;

    char *str = inbuf;

    int count = 0;

    if (str == NULL)
    {
        return -1;
    }

    while (isspace(str[i]) && str[i] != '\0')
    {
        i++;
    }

    while (isspace(str[j]) && j > i)
    {
        j--;
    }

    count = j - i + 1;

    strncpy(inbuf, str + i, count);

    inbuf[count] = '\0';

    return 0;
}


/**
 * @brief  解析url query 类似 abc=123&bbb=456 字符串
 *          传入一个key,得到相应的value
 * @returns
 *          0 成功, -1 失败
 */
int QueryParseKeyValue(const char *query, const char *key, char *value, int *value_len_p)
{
    char *temp = NULL;
    char *end = NULL;
    int value_len = 0;

    //找到是否有key
    temp = (char *)strstr(query, key);
    if (temp == NULL)
    {
        return -1;
    }

    temp += strlen(key); //=
    temp++;              // value

    // get value
    end = temp;

    while ('\0' != *end && '#' != *end && '&' != *end)
    {
        end++;
    }

    value_len = end - temp;

    strncpy(value, temp, value_len);
    value[value_len] = '\0';

    if (value_len_p != NULL)
    {
        *value_len_p = value_len;
    }

    return 0;
}
 