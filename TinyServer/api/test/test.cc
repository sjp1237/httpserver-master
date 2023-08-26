#include<iostream>
#include<string>
#include<stdlib.h>
#include<string.h>
using namespace std;


int test_parsefile(string post_data)
{
    char suffix[1023] = {0};
    int ret = 0;
    char boundary[10000] = {0}; //分界线信息
    char file_name[128] = {0};
    char file_content_type[128] = {0};
    char file_path[128] = {0};
    char new_file_path[128] = {0};
    char file_md5[128] = {0};
    char file_size[32] = {0};
    long long_file_size = 0;
    char user[32] = {0};
    char *begin = (char *)post_data.c_str();
    char *p1, *p2,*p3;

    // 1. 解析boundary
    // Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryjWE3qXXORSg2hZiB
    // 找到起始位置
    p1 = strstr(begin, "\r\n"); // 作用是返回字符串中首次出现子串的地址
    if (p1 == NULL)
    {
        cout << "wrong no boundary!";
        ret = -1;
      
    }
    //拷贝分界线
    strncpy(boundary, begin, p1 - begin); // 缓存分界线, 比如：WebKitFormBoundary88asdgewtgewx
    boundary[p1 - begin] = '\0';          //字符串结束符
    cout << "boundary: " << boundary<<endl;

    // 查找文件名file_name
    begin = p1 + 2;
    p2 = strstr(begin, "user="); //找到user字段
    if (!p2)
    {
        cout << "wrong no file_name!";
        ret = -1;
        
    }
    p3 = strstr(p2, "\" "); // 找到"
    begin=p2+6;
    strncpy(user, begin, p3 - begin);
    user[p3-begin]='\0';
    cout << "user: " << user<<endl;

    // 查找文件类型file_content_type
    begin = p3 + 2;
    p2 = strstr(begin, "filename="); //
    if (!p2)
    {
        cout << "wrong no file_content_type!";
        ret = -1;
       
    }
    p3 = strstr(p2, "\" ");
    begin = p2+10;

    strncpy(file_name, begin, p3 - begin);
    file_name[p3-begin]='\0';
    cout << "filename: " << file_name<<endl;
   
    // 查找文件file_path
    begin = p2 + 2;
    p2 = strstr(begin, "md5="); //
    if (!p2)
    {
        cout << "wrong no md5!";
        ret = -1;
        
    }
    p3 = strstr(p2, "\" ");
    begin =p2+5;
    strncpy(file_md5, begin, p3 - begin);
    file_md5[p3-begin]='\0';
    cout << "md5: " << file_md5<<endl;

    // 查找文件file_md5
    begin = p2 + 2;
    p2 = strstr(begin, "size="); //
    if (!p2)
    {
        cout << "wrong no file_size!";
        ret = -1;
       
    }

    p3 = strstr(p2, "\r\n");
    begin=p2+5;
    strncpy(file_size, begin, p3 - begin);
     file_size[p3-begin]='\0';
    cout << "file_size: " << file_size<<endl;
    long_file_size = strtol(file_size, NULL, 10); //字符串转long

}

int main()
{
    string data="------WebKitFormBoundaryzAGwANqkEWo4MJuu\r\nContent-Disposition: form-data; user=\"zhangsan\" filename=\"111.rtf\" md5=\"f62a1153ee395d8a731472725d0edd58\" size=365\r\nContent-Type: application/octet-stream\r\n\r\n﻿1324444444444444444432\r\n\r\n------WebKitFormBoundaryzAGwANqkEWo4MJuu\r\n";                                                                                                                                                                                 
    test_parsefile(data);
}