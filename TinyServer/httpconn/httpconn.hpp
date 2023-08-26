#pragma once
#include<unordered_map>
#include<vector>
#include<sys/socket.h>                                                                
#include<iostream>              
#include<string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include<sys/epoll.h>

#include"../lst_timer/lst_timer.h"

#define HOME_PAGE "index.html"
#define NOT_FOUND_PAGE "wwwroot/404.html"
#define NOT_FOUND "404"
#define OK "200"
#define BLANK "\r\n"
#define MAX_FD 1024
#define CODE_LEN 5
#define RESOURCE_DIR "wwwroot/"


static std::unordered_map<std::string,std::string> suffix_type{
  {"html","text/html"},
  {"txt","text/plain"},
  {"ppt","application/vnd.ms-powerpoint"},
  {"zip","application/zip"},
  {"movie","video/x-sgi-movie"}
  };
static std::unordered_map<std::string,std::string> code_desc{
  {"200","OK"},
  {"404","NOT FOUND"},
};



  enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,//解析请求行
        CHECK_STATE_HEADER=1,       //解析请求报头	
        CHECK_STATE_CONTENT=2       //解析消息体
  };

  enum HTTP_CODE
    {
        NO_REQUEST,              //请求不完整
        GET_REQUEST,            //	获得了完整的HTTP请求
        BAD_REQUEST,             //HTTP请求报文有语法错误
        NO_RESOURCE,            //没有请求资源
        FORBIDDEN_REQUEST,      //资源被禁止访问
        FILE_REQUEST,           //文件请求
        INTERNAL_ERROR,          
        CLOSED_CONNECTION       //关闭链接
  };

 class Request{

    public:
     enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
       ~Request(){
          m_content_len=0;
          
       }
    void init()
    {
      m_check_state=CHECK_STATE_REQUESTLINE;
      m_request_line.clear();
      m_url.clear();
      m_version.clear();
      m_path.clear();
      m_parameter.clear();
      m_host.clear();
      m_content_len=0;
      m_content.clear();
      m_etag.clear();
    //  request_header.clear();
    // header_map.clear();
      m_content_pos=0;
    }


public:
     std::string getUrl();
     std::string&  getContent();//获取请求正文
     int getContentLen();//获取请求正文的长度
     std::string getVersion();//获取请求版本
     std::string getEtag();//获取etag
     std::string getHost();
     std::string getPath();//获取请求路径
     std::string getParameter();//获取请求参数
     std::string getRange();
     METHOD getMethod();
    int ParseHeader(string& key,string &value);


     bool isLongLinker();//是否长连接
     CHECK_STATE getCheckState();//获取解析状态

     HTTP_CODE parse_request_line(std::string text); 
      //解析请求报头
    HTTP_CODE parse_request_header(std::string text);
private:
      //解析请求正文   
    bool AnalyUri();//将url中的路径和参数给分离开来
public:
      std::string m_request_line;
      METHOD m_method;        //请求方法
      std::string m_url;      //url
      std::string m_version;  //请求版本
      std::string m_path;     //请求路径
      std::string m_parameter;//参数
      std::string m_host;   //主机名
      bool m_long_linker;
      int m_content_len;   //请求正文的长度
      int m_content_pos;      //读取请求正文的下标
      std::string m_content;  //请求正文
      std::string content_type;
      string m_etag; //旧的old_etag;
      string m_range;//表示请求资源从哪里开始下载
     // std::vector<std::string> request_header;
    //存储响应报头
     //  std::unordered_map<std::string,std::string> header_map;     
     CHECK_STATE m_check_state;//记录解析哪个位置,请求行，请求报头，请求正文        
  };

 class Response{    
    enum SEND_STATE
    {
      SEND_LINE=0,
      SEND_HEADER,
      SEND_CONTENT
    };
    public:
      int SetCode(string code);                  //设置响应码
      void SetHeader(string key,string value); //设置响应报头
      void SetContent(string content);         //设置响应正文
      void SetVersion(string version);
      int  SetResource(string file);
      void BuildResponseLine();
      void BuildResponseHeader();
      int GetResponseSize();//获取响应的大小
      int Send(int socket);

      void init()
      {
        m_send_state=SEND_LINE;
        m_send_line_size=0;
         m_send_header_size=0;  //响应头已经发送的数据
       m_send_content_size=0; //响应正文已经发送的数据
        m_version="http/1.1";   
        m_line.clear();
        m_version.clear();
        m_header.clear();
        m_content.clear();
        cgi_res.clear();
        content_size=0;
        m_content_size=0;
        is_build=false;
      }
      ~Response(){       
           m_send_content_size=0;
           m_send_header_size=0;
           m_send_line_size=0;

      }  
  private:
       int sendLine(int socket);
       int sendHeader(int socket);
       int sendContent(int socket);

  public:
      string m_code;
      std::string m_line;    //响应行
      std::string m_version="http/1.1";    
      int content_size; //响应正文的大小
      std::string m_header;//响应报头
      std::string m_content;//响应正文
      std::string cgi_res;
      int m_content_size;//响应正文大小
      int m_source_flag;//是否发送资源
      int m_source; //资源文件描述符     
      SEND_STATE m_send_state;
      int m_send_line_size;    //响应行已经发送的数据
      int m_send_header_size;  //响应头已经发送的数据
      int m_send_content_size; //响应正文已经发送的数据

      bool is_build=false;
  };    

class httpconn{
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
  public:
  //util_timer timers[MAX_FD];
  //int pipefd;
  public:
  ~httpconn()
  {
    delete m_request;
    delete m_response;
  }
    httpconn(int sockfet=0)
    :m_socket(sockfet)
    {
      m_request=new Request();
      m_response=new Response();
    }

    enum LINE_STATUS
    {
        LINE_OK = 0,  //完整读取一行
        LINE_BAD,     //报文语法有误
        LINE_OPEN     //读取的行不完整
    };

    enum Option{
      UPLOAD=0, //上传请求
      DOWNLOAD, //下载请求
      WEBPAGE,  //网页请求
      CGI,      //CGI请求
      SHOWLIST //显示下载网页
    };
    

    void setReadBuffer(char* s);
public:

  // HTTP_CODE process_write();
    //由工作线程进行工作
    // int process(); 

    //处理read_buffer中的数据,有限状态机处理
    //将response中的response_body和response_content中的数据发送给对端
    //bool Write();
    // void ShowList();

    // void BuildReponseLine();
   // void BuildResponseHeaer()
    //  void  AnalyFile();//分析文件
    // void ParseDownFile();//解析下载文件
    // void ParseUpLoadFile();
    // void UpLoad();
    // //解析文件
    // //do_request中的cgihandle没有测试完成，其余的测试好了
    // int  do_request();
    // void OpenPage();
      //  void send_response();
    // void SendResponseHeader();
    // void SendResponseContent();
    
    int Read();//将缓冲区中的数据读取到read_buffer中
    bool Write();
    HTTP_CODE ReadContent( );
    HTTP_CODE process_read();//将数据解析到 Request
    int task_cb();
    //初始化数据
    void init();
    void setfd(int sockfd);
    void clear();
    
    Request* GetRequest();
    Response* GetResponse();
    //判断读缓冲区中是否 构成一行
    LINE_STATUS parse_line();
    //返回一行的起始位置
    void read_line();
  private:
    bool CgiHandle();
  public:
    Response* m_response;
    Request* m_request;
public:
    //环形缓冲区-----读缓冲区
    char read_buffer[READ_BUFFER_SIZE]; //读缓冲区
    int read_buffer_size=READ_BUFFER_SIZE;
    string text; //从read_buffer解析出一行数据
    size_t m_read_head=0;//缓冲区可读数据的起始位置
    size_t m_read_tail=0;//缓冲区可写数据的起始位置
    size_t m_sum_size=0;//缓冲区中可读的数据
    size_t m_line_len=0;//一行数据的长度，从环形缓冲区中获取
    size_t m_cur_content_len=0;//已经读取的请求正文的长度

    int m_socket;
    bool cgi=false;
    int epoll_fd;
    //webserver* server;
    string fileName;//下载的文件名，上传的文件名
    string file_content;//上次的文件文本内容
    bool break_point_resume=false;
    int start_pos; //断点续传需要用到，文件断点下载时的位置
    Option m_op;
    int m_state=0;
};
