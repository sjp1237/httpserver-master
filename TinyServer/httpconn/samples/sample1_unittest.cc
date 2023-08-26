#include <gtest/gtest.h>
#include"../httpconn.hpp"
#include"string.h"

//通过
TEST(Request, parse_request_line) 
{ 
    Request request;
    request.parse_request_line("get wwwroot/upload http1.1");

    EXPECT_EQ(request.getPath(),"wwwroot/upload");
    EXPECT_EQ(request.getUrl(),"wwwroot/upload");
    EXPECT_EQ(request.getVersion(),"http1.1");
    EXPECT_EQ(request.getMethod(),Request::GET);


   Request request1;
    request1.parse_request_line("post wwwroot/upload?a=100:b=200 http1.1");

    EXPECT_EQ(request1.getPath(),"wwwroot/upload");
    EXPECT_EQ(request1.getUrl(),"wwwroot/upload?a=100:b=200");
    EXPECT_EQ(request1.getVersion(),"http1.1");
    EXPECT_EQ(request1.getMethod(),Request::POST);
    EXPECT_EQ(request1.getParameter(),"a=100:b=200");
}


TEST(Request, parse_request_header) 
{ 
  Request request;
  
  request.parse_request_header("Content-Length: 18");
  EXPECT_EQ(request.getContetnLen(),18);

 request.parse_request_header("Connection: keep-alive");
  EXPECT_EQ(request.isLongLinker(),true);

   request.parse_request_header("Host: 119.23.41.13");
  EXPECT_EQ(request.getHost(),"119.23.41.13");

  request.parse_request_header("If-Range: 123456");
  EXPECT_EQ(request.getEtag(),"123456");

  request.parse_request_header("Range: 666666");
  EXPECT_EQ(request.getRange(),"666666");  
}




TEST(Response, SetHeader) 
{ 
    Response response;
    
    response.SetCode("200");
    response.BuildResponseLine();
    EXPECT_EQ(response.m_line,"http/1.1 200 OK\r\n");

    response.SetHeader("Connection","keep-alive");
    EXPECT_EQ(response.m_header,"Connection: keep-alive\r\n");
    response.SetHeader("Content-typedef","text/html");
    EXPECT_EQ(response.m_header,"Connection: keep-alive\r\nContent-typedef: text/html\r\n");

    response.SetHeader("Content-Length","100");

    response.SetContent("12345678");
    response.BuildResponseHeader();
    EXPECT_EQ(response.m_header,"Connection: keep-alive\r\nContent-typedef: text/html\r\nContent-Length: 8\r\n\r\n");
    EXPECT_EQ(response.m_content,"12345678");

   // EXPECT_EQ(res, 1); 
    /* 比较返回和期望值 */ 
}


TEST(httpconn, read_line) 
{ 
    httpconn conn;
    const char *s="123456\r\n";
    conn.m_sum_size=8;
    conn.m_read_tail=8;
    strcpy(conn.read_buffer,s);
    int ret=conn.parse_line();
    EXPECT_EQ(ret,httpconn::LINE_OK);
    EXPECT_EQ(conn.m_line_len,6);
     EXPECT_EQ(conn.m_read_tail,8);
    conn.read_line();
    EXPECT_EQ(conn.text,"123456");
    EXPECT_EQ(conn.m_sum_size,0);
    EXPECT_EQ(conn.m_read_head,8);
}

TEST(httpconn, read_line1) {
    httpconn conn;
    const char *s1="123456\r\n12";
    conn.m_sum_size=10;
    conn.m_read_tail=8;
    strcpy(conn.read_buffer,s1);
    int ret1=conn.parse_line();
    EXPECT_EQ(ret1,httpconn::LINE_OK);
    EXPECT_EQ(conn.m_line_len,6);
    EXPECT_EQ(conn.m_read_tail,8);
    conn.read_line();
    EXPECT_EQ(conn.text,"123456");
    EXPECT_EQ(conn.m_sum_size,2);
    EXPECT_EQ(conn.m_read_head,8);

    //s1 =”12“
    int ret2=conn.parse_line();
    EXPECT_EQ(ret2,httpconn::LINE_OPEN);

}


TEST(httpconn, read_line2) {
    httpconn conn;
    const char *s1="123456\r12";
    conn.m_sum_size=10;
    conn.m_read_tail=8;
    strcpy(conn.read_buffer,s1);
    int ret1=conn.parse_line();
    EXPECT_EQ(ret1,httpconn::LINE_BAD);
}



TEST(httpconn, read_line3) {
    //从第"1"开始读取数据
    httpconn conn;
    const char *s1="11\r\n123456\r\n12";

    conn.m_sum_size=strlen(s1);
    conn.m_read_head=4;
    conn.m_read_tail=4;
    strcpy(conn.read_buffer,s1);
    int ret1=conn.parse_line();
    EXPECT_EQ(ret1,httpconn::LINE_OK);

    conn.read_line();
    EXPECT_EQ(conn.text,"123456");
    EXPECT_EQ(conn.m_sum_size,strlen(s1)-8);
    EXPECT_EQ(conn.m_read_head,12);
    conn.text.clear();
    int ret2=conn.parse_line();
    EXPECT_EQ(ret2,httpconn::LINE_OK);
    conn.read_line();

    EXPECT_EQ(conn.text,"1211");
    EXPECT_EQ(conn.m_sum_size,0);
    EXPECT_EQ(conn.m_read_head,4);
}

