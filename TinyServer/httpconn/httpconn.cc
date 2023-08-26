#include"httpconn.hpp"
#include<sys/types.h>

#include<sys/socket.h>
#include<iostream>
#include<string.h>
#include<string>
#include<sys/socket.h>   
#include<sys/stat.h>
#include<ctime>
#include"../tool/tool.hpp"
#include<assert.h>
#include"../log/log.h"

#if 0
#include"../FileUtil/FileUtil.hpp"          
#include"../FileUtil/FileInfo.hpp" 
#endif
//#include"tool.hpp"  
     


static std::string mtimeToString(time_t mtime){
        std::string res= ctime(&mtime);
        return res;
}
//分析url资源
//判断文件是否存在
//判断文件是否是一个可执行文件，如果是一个可执行文件，则调用cgi机制
//判断一个文件是否一个普通文件，则将该文件提前打开，并保存相关文件的fd

#if 0
//获取文件etag
static  std::string GetEtag(std::string filename){
      sjp::FileUtil fu(filename);
      std::string etag=fu.GetFilename();
      etag+="-";
      etag+=std::to_string(fu.GetFileSize());
      etag+="-";
      etag+=std::to_string(fu.GetFileModfityTime());
      return etag;
}
#endif


//wwwroot/
bool Request::AnalyUri(){

   //将url中的路径和参数给区分开来
    std::string& uri=m_url;
  //  cout<<"uri: "<<uri<<endl;
    //不管是Get方法还是Post方法,只要没找到"?", 则说明uri中一定没有参数
    if(uri.find("?")!=-1){
    //说明uri中带参数
    std::string seq="?";
    Tool::CuttingString(uri,m_path,m_parameter,seq);
    }else{
      //没有找到?,说明uri只有路径
       m_path=uri;      
    }
}




std::string Request::getRange()
{
  return m_range;
}
std::string Request::getUrl(){
  return m_url;
}
std::string& Request::getContent()
{
  return m_content;
}
int Request::getContentLen()
{
  return m_content_len;
}
  //获取请求正文的长度
std::string Request::getVersion()
{
  return m_version;
}
std::string Request::getEtag()
{
  return m_etag;
}
std::string Request::getHost()
{
  return m_host;
}

std::string Request::getPath()
{
  return m_path;
}


CHECK_STATE Request::getCheckState()
{
  return m_check_state;
}
std::string Request::getParameter()
{
  return m_parameter;
}


bool Request::isLongLinker()
{
  return m_long_linker;
}

Request::METHOD Request::getMethod()
{
  return m_method;
}

 int Response::sendLine(int socket)
 {
     int ret=send(socket,m_line.c_str()+m_send_line_size,m_line.size()-m_send_line_size,MSG_DONTWAIT);
     if(ret>0){
       m_send_line_size+=ret;
       if(m_send_line_size>=m_line.size()){
          m_send_state=SEND_HEADER;
          return 0;
       }
     }
     else 
     {
       return -1;
     }
 }

int Response::sendHeader(int socket)
{
   int  ret=send(socket,m_header.c_str()+m_send_header_size,m_header.size()-m_send_header_size,MSG_DONTWAIT);
  if(ret>0){
    m_send_header_size+=ret;
    if(m_send_header_size>=m_header.size()){
      m_send_state=SEND_CONTENT;
      return 0;
    }
  }
  else{
    return -1;
  }
}
int Response::sendContent(int socket)
{
   if(m_source_flag)
    {
     //会阻塞住
     sendfile(socket,m_source,0,m_content_size);
    }
    else {
     int ret=send(socket,m_content.c_str()+m_send_content_size,m_content_size-m_send_content_size,MSG_DONTWAIT);
     if(ret>0){
       m_send_content_size+=ret;
       if(m_send_content_size>=m_content_size){
         return 0;//发送完成
       }
     }else {
      return -1;
     }
  }
}
int Response::Send(int socket)
{
  if(!is_build)
  {
      BuildResponseLine();
      BuildResponseHeader();
      is_build=true;
  }

  while(true)
  {
    int ret=0;
  switch (m_send_state)
  {
  case SEND_LINE:
    ret =sendLine(socket);
     if(ret<0){
      return -1;
     }
    break;
  case  SEND_HEADER:
     ret=sendHeader(socket);
    if(ret<0)
    {
      return -1;
    }
    break;
  case SEND_CONTENT:
     ret=sendContent(socket);
    if(ret<0)
    {
      return -1;
    }
    return 0;
    break;
  }
}


 
  sendContent(socket);
  return 0;
}


int Response::SetResource(string file)
{
    std::string path=RESOURCE_DIR+file;
       struct stat buf;
        if(stat(path.c_str(),&buf)==0){
          if(S_ISDIR(buf.st_mode)){
            //路径是一个目录，打开该目录下的首页信息
            return -1;
          }
          else if(buf.st_mode&S_IXUSR||buf.st_mode&S_IXGRP||buf.st_mode&S_IXOTH){
            return -1;
          }
          else{
            //普通文件
            m_source=open(path.c_str(),O_RDONLY);
            if(m_source==-1){
              return -1;
            }
            m_content_size=buf.st_size;
            m_source_flag=true;
            return 0;
          }
        }
        else{
          //找不到资源
          return -1;
        }
}

int Response::GetResponseSize()
{
   int size=m_line.size()+m_header.size()+m_content_size;
   return size;
}


#if 0
 void  httpconn::AnalyFile(){       
        std::string path=m_request->m_path;
        m_request->m_path="wwwroot";
        m_request->m_path+=path;
        //添加wwwroot前缀
        if(m_request->m_path[m_request->m_path.size()-1]=='/'){   
          m_request->m_path+=HOME_PAGE;
          IsSendPage=true;
        //  cout<<"[AnalyFile]: m_request->m_path"<<endl;
        }

        struct stat buf;
        if(stat(m_request->m_path.c_str(),&buf)==0){
          if(S_ISDIR(buf.st_mode)){
            //路径是一个目录，打开该目录下的首页信息
            m_request->m_path+="/";
            m_request->m_path+=HOME_PAGE;//HOME_PAGE: index.html
            IsSendPage=true;
          }
          else if(buf.st_mode&S_IXUSR||buf.st_mode&S_IXGRP||buf.st_mode&S_IXOTH){
            //可执行文件
            cgi=true;
            return ;
          }
          else{
            //普通文件
            IsSendPage=true;
          }
        }
        else{
          //找不到该文件
          //打开文件描述符
          m_response->code=NOT_FOUND;//404
          m_request->m_path=NOT_FOUND_PAGE;
          m_response->suffix="html";
          stat(m_request->m_path.c_str(),&buf);
          m_response->content_size=buf.st_size;
          IsSendPage=true;
          return ;
        }
        //文件大小,也就是响应的正文的大小
        m_response->content_size=buf.st_size;
        //查找文件后缀
        int pos=m_request->m_path.rfind(".");
        if(pos>0){
          //文件后缀
          m_response->suffix=m_request->m_path.substr(pos+1);
        }else{
          //文件没有后缀,默认是html
          m_response->suffix="html";
        }
    }
#endif
//直接将数据读取上来的方式不好，会发生tcp黏包
int httpconn::Read(){
    //缓冲区满了
    if (m_sum_size >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;


#if 0
      m_read_tail+=50;
      m_read_head=20;
      m_sum_size=30;
#endif
    //LT读取数据
    //第一次读取数据


    while(true)
    {
     int first_data_len=READ_BUFFER_SIZE-m_read_tail;//判断后半段缓冲区的长度
     if(first_data_len>0)
     {
       bytes_read = recv(m_socket, read_buffer + m_read_tail, first_data_len, MSG_DONTROUTE);
        //测试 一个http报文可能需要被读取两次

       if (bytes_read == 0)
       {
         return false;
       }
       else if(bytes_read<0)
       {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
              return true;
        }
        return false;
       }
       m_sum_size += bytes_read;
       m_read_tail= (m_read_tail+bytes_read)%READ_BUFFER_SIZE;
     }
    else if(m_sum_size<READ_BUFFER_SIZE)
    {
       //判断缓冲区是否满了
       int second_data_len=m_read_head;
       bytes_read = recv(m_socket, read_buffer + m_read_tail, second_data_len, MSG_DONTROUTE);
       if (bytes_read == 0)
       {
         return false;
       }
       else if(bytes_read<0)
       {
        return true;
       }
       m_sum_size += bytes_read;
       m_read_tail= (m_read_tail+bytes_read)%READ_BUFFER_SIZE;
    }   
  }

   



    return true;
}


#if 0
httpconn::HTTP_CODE httpconn::process_write()
{
   //构建响应
   //响应行: http/1.1 响应状态码 响应状态码描述
   //响应报头：Content-Length: 
   //Content-Type: Text/html
   //Connection: keep-alive或者是close
   //将响应数据构建在response_body中
   //构建响应行和响应报头
   
   //
   //缺少添加状态描述符
   BuildReponseLine();
   //缺少添加文件类型的描述
   BuildResponseHeaer();
}
#endif

int httpconn::task_cb()
{
   if (0 == m_state)
   {
        if (Read())
        {
          // process();
        }
        else
        {

        }
    }
    else
    {
        
    }
}


#if 0
//工作线程进行解析请求
int httpconn::process(){
  //解析请求，并解析对应的url资源
   HTTP_CODE res=process_read();
   if(res==NO_REQUEST){
       //重新将socket设置进epoll对象中
      //webserver::GetInstant()->addevent(m_socket,EPOLLIN);
       return 0;
   }

   if(res==httpconn::BAD_REQUEST){
    //请求错误，得向客户端发送一个坏请求的页面
    //并关闭链接,设置一个错误码errno，通知上层
    return -1;
   }


   //读取完请求后，解析好请求，接下来构建响应
  if(showList||m_upload){
    //cout<<"showlist: 设置写文件描述符"<<endl;
   // webserver::GetInstant()->addevent(m_socket,EPOLLOUT,EPOLL_CTL_MOD,false,true);
    return 0;
  }


  //获取请求完成，处理业务
  //res=process_write();
  //响应构建完成
  //将socket设置为写事件
 // webserver::GetInstant()->addevent(m_socket,EPOLLOUT,EPOLL_CTL_MOD,false,true);
  return 0;
}
#endif


void httpconn::setfd(int sockfd)
{
  m_socket=sockfd;
}


void httpconn::read_line()
{
  if(m_line_len==0)
  {
     text.append("\r\n");
    m_sum_size-=(m_line_len+2); //更新缓冲区的数据长度
     m_read_head=(m_read_head+m_line_len+2)%read_buffer_size;   //更新缓冲区的可读数据起始位置 
     return;
  }
   //m_line_len是一行数据的长度
  int cur_pos=m_read_head;
  //判断一行数据没有被一分为二
  #if 0
   cur_pos+=30;
   m_line_len-=30;
  #endif 
  if(cur_pos+m_line_len<=read_buffer_size)
  {
     text.append(&read_buffer[cur_pos],m_line_len);
  }
  else
  {
    //数据被一分为二
    int right_len=read_buffer_size-cur_pos;
    int left_len=m_line_len-right_len;
    text.append(&read_buffer[cur_pos],right_len);
    text.append(read_buffer,left_len);
  }
  m_sum_size-=(m_line_len+2); //更新缓冲区的数据长度
  m_read_head=(m_read_head+m_line_len+2)%read_buffer_size;   //更新缓冲区的可读数据起始位置 
}

//将请求读取到Request中
HTTP_CODE httpconn::process_read(){
  //解析行
  LINE_STATUS line_state=LINE_OK;
  //从读缓冲区读取一行
  while(m_request->getCheckState()==CHECK_STATE_CONTENT||parse_line()==LINE_OK){
    //解析结果
    HTTP_CODE ret=NO_REQUEST;
    switch(m_request->getCheckState()){
      case CHECK_STATE_REQUESTLINE:
      {
        //解析请求行       // cout<<"请求行:"<<text<<endl;
        //获取一行的起始位置
       read_line();
        ret=m_request->parse_request_line(text);
        if(BAD_REQUEST==ret){
          return BAD_REQUEST;
        }
        text.clear();
        break;
      }

      case CHECK_STATE_HEADER:
        //解析请求报头       
       read_line(); //获取一行的起始位置
       ret=m_request->parse_request_header(text);
        if(BAD_REQUEST==ret){
          return BAD_REQUEST;
        }
        //请求完整
        else if(GET_REQUEST==ret){
          return GET_REQUEST;
        }
        text.clear();
        break;
      case CHECK_STATE_CONTENT:
        //将正文请求正文的数据读取上来
        ret=ReadContent();
     // cout<<m_request->content<<endl;
        if(GET_REQUEST==ret){
            return GET_REQUEST;
        }
        else if(ret==BAD_REQUEST){
          return BAD_REQUEST;
        }
        else if(ret==NO_REQUEST)
        {
          return NO_REQUEST;
        }
        break;
    }
  }
  return NO_REQUEST;
}
   //调用可执行文件
   //将处理结果返回给主进程，可能为静态网页，也可能是处理是计算结果
   //

  #if 0
  bool httpconn::CgiHandle(){
        //  std::cout<<"CgiHandle .."<<std::endl;
          //利用管道给cgi程序发送数据
          int pipe0[2];//父进程发送给子进程的管道
          int pipe1[2];//父进程接收子进程的管道
          if(pipe(pipe0)!=0){
            std::cout<<"pipe create fail"<<std::endl;
            return false;
          }
          if(pipe(pipe1)!=0){
            std::cout<<"pipe create fail"<<std::endl;
            return false;
          }

        if(fork()==0){
            //子进程
            close(pipe0[1]);//pipe0[0] 子进程用来读
            close(pipe1[0]);//pipe1[1] 子进程用来写
            dup2(pipe1[1],1);
            dup2(pipe0[0],0);
            //程序替换后,所有的数据和代码都会被清空,包括文件描述符，
            //只有环境变量不会替换掉
            //所以通过重定向文件描述符,则子进程可以通过cin向父进程输入数据
            //cout从管道中读取数据
            //子进程需要知道是什么方法来，从而判断是往管道还是环境变量中获取参数
            //子进程需要知道参数的大小，从而判断是否有读漏
            std::string method_env="METHOD=";
            method_env+=m_request->m_method;
            putenv((char*)method_env.c_str());
            if(m_request->m_method==Request::GET){
              std::string parameter="PARAMETER=";
              parameter+=m_request->m_parameter;
              putenv((char*)parameter.c_str());
            }
            else if(m_request->m_method==Request::POST){
              std::string body_size="Content-Length=";
              body_size+=std::to_string(m_request->m_content_len);
              putenv((char*)body_size.c_str());
          }
          //程序替换
          execl(m_request->m_path.c_str(),m_request->m_path.c_str(),NULL);  
        }
        else{
            //父进程
            close(pipe0[0]);//pipe0[1]用来写数据给子进程
            close(pipe1[1]);//pipe1[0]读取子进程中的数据
            if(m_request->m_method==Request::POST){
            auto& content=m_request->content;
            //std::cout<<"content:"<<content<<std::endl;
             int sumsize=0;
             while(true){
              //将请求正文发送给子进程
              int size=content.size();
              int sz=write(pipe0[1],content.c_str()+sumsize,size);
              if(sz>0){
                sumsize+=sz;
                if(sumsize==m_request->m_content_len)
                  break;
              }
              else{
                //写失败
                cout<<"父进程向子进程发送数据失败"<<endl;
                break;
              }
            }
          }

          //父进程读取子进程的处理结果
          while(true){
              char ch;
              int sz=read(pipe1[0],&ch,1);
              if(sz>0){
                //读取成功
                m_response->cgi_res.push_back(ch);
              }else{
                //对端关闭
                break;
              }
            }//while
            //IS-SendPage: trueX 
            auto& cgi_res=m_response->cgi_res;
            int begin1=cgi_res.find(": ")+2;
            int end1=cgi_res.find("X");

            std::string is_sendpage=m_response->cgi_res.substr(begin1,end1-begin1);
            // LOG(INFO,is_sendpage.c_str());
            if(is_sendpage=="true"){
              //发送静态网页
              //Page-index:wwwroot/regisuccess.htmlX
              IsSendPage=true;
              int begin2=cgi_res.find(": ",end1+1)+2;
              int end2=cgi_res.find("X",begin2);
              std::string page_index=cgi_res.substr(begin2,end2-begin2);
              //m_response->fd=open(page_index.c_str(),O_RDONLY);
              struct stat buf;
              stat(page_index.c_str(),&buf);
              //最终content_size大小为文件
              m_response->content_size=buf.st_size;
            }
            else{
              //请求正文
              m_response->response_content=cgi_res;
              m_response->content_size=m_response->cgi_res.size();
              //std::cout<<"response_body:"<<response.response_body<<std::endl;
            }
            close(pipe0[1]);
            close(pipe1[0]);
        }      
 }

//get /test.txt/download
//post /updown
bool httpconn::AnalyUri(){
   //将url中的路径和参数给区分开来
    std::string& uri=m_request->m_url;
  //  cout<<"uri: "<<uri<<endl;
    //不管是Get方法还是Post方法,只要没找到"?", 则说明uri中一定没有参数
    if(uri.find("?")!=-1){
    //说明uri中带参数
    std::string seq="?";
    Tool::CuttingString(uri,m_request->m_path,m_request->m_parameter,seq);
    }else{
      //没有找到?,说明uri只有路径
       m_request->m_path=uri;      
    }

    int pos=m_request->m_path.rfind("/");
    string s=m_request->m_path.substr(pos+1);
   // cout<<"s："<<s<<endl;
    if(s=="download"){
      //111.txt/download
      //去掉download,在文件名之前添加/backdir/前缀
      m_request->m_path=m_request->m_path.substr(0,pos);
      //设置下载的文件名
      pos=m_request->m_path.rfind("/");
      fileName=m_request->m_path.substr(pos+1);
      downFile=true;
    }
    else if(s=="ListShow"){
    //  cout<<"list show"<<endl;
      showList=true;
    }
    else if(s=="upload")
    {
      m_upload=true;
    }
  //  cout<<"m_request->m_path: "<<m_request->m_path<<endl;
}

void httpconn::OpenPage()
{
  fd=open(m_request->m_path.c_str(),O_RDONLY);
  if(fd<0){
    //打开失败
    //cout<<"m_request->m_path: "<<m_request->m_path<<endl;
   // cout<<"fd == -1"<<endl;
  }
}

void httpconn::ShowList()
 {
    cout<<"listshow start"<<endl;    
    std::vector<FileInfo> arry;    
    FileInfoManger::GetInstant()->GetAllInfo(arry);    
    std::stringstream ss;    

    ss<<"<!DOCTYPE html>"<<"<html><meta charset=\"UTF-8\"><head><title>Download</title></head><body><h1>Download</h1><table>";      
                  ss<<"<tr><td><h3>文件名</h3></a></td>";        
    ss<<"<td align=\"left\">"<<"<h3>最近修改时间</h3>"<<"</td>";                                                 
    ss<<"<td align=\"right\">"<<"<h3>文件大小</h3>"<<"</td></tr>";     
    for(int i=0;i<arry.size();i++){    
      //>    
      /*<td><a href="/download/test.txt">test.txt</a></td>    
        <td align="right">2021-12-29 10:10:10</td>    
        <td align="right">28k</td>    
        * */    
      std::string path=arry[i].back_path;    
      sjp::FileUtil fu(path);    
      std::string atimestr=mtimeToString(arry[i].modify_time);    
    //  ss<<"<tr><td><a href=\"/download/"<<fu.GetFilename()<<"\">"<<fu.GetFilename()<<"</a></td>";    
      ss<<"<tr><td><a href=\"/"<<fu.GetFilename()<<"/download"<<"\">"<<fu.GetFilename()<<"</a></td>";    

      ss<<"<td align=\"right\">"<<atimestr<<"    </td>";    
      ss<<"<td align=\"right\">"<<arry[i].file_size/1024<<"k"<<"</td></tr>";                                                                                                          
    }    
    ss<<"</table></body></html>";
    ss<<"<form action=\"http://119.23.41.13:8081/upload\" method=\"post\" enctype=\"multipart/form-data\"><div><input type=\"file\" name=\"file\"></div><div><input type=\"submit\" va  lue=\"上传\"></div></form>";
    m_response->response_content=ss.str();

   m_response->response_body="http/1.1 200 OK\r\n";
   m_response->response_body+="Content-Type: text/html\r\n";
   m_response->response_body+="Content-Length: ";
   m_response->response_body+=to_string(m_response->response_content.size());
   m_response->response_body+=BLANK;
   m_response->response_body+=BLANK;
   //cout<<m_response->response_body<<endl;
  // cout<<m_response->response_content<<endl;
}



void httpconn::UpLoad()
{
    /* 1.判断是否有文件上传
      * 2.如果有，则获取该文件的名称和内容
      * 3.在backdir中创建一个文件
      * 4.将文件信息存储到FileInfoManger中
      * */
    //判断是否有文件上传
    std::cout<<"upload begin"<<endl;
    //设置文件路径
    std::string filepath=Config::GetInstant()->GetBackDir()+fileName;
    std::string body=file_content;

    sjp::FileUtil fu(filepath);
    fu.SetContent(body);
    FileInfo fileinfo(filepath);
    FileInfoManger::GetInstant()->Insert(fileinfo);  
    FileInfoManger::GetInstant()->Storage();
 //   cout<<filepath<<endl;
  //  cout<<file_content<<endl;
    // ListShow(req,rep);
}

 

//给下载的文件设置相对路径，并查询下载文件的大小
void httpconn::ParseDownFile()
{
     //路径设置为的相对路径 
  m_request->m_path=Config::GetInstant()->GetBackDir();
  m_request->m_path+=fileName;

  //通过back路径获取文件信息
  FileInfo fileinfo;
  FileInfoManger::GetInstant()->GetoneByURL(m_request->m_path,fileinfo);

   //判断文件是否被压缩
  if(fileinfo.pack_sign==true)
  {
    //解压缩
    sjp::FileUtil fu(fileinfo.pack_path);
    fu.UnpackFile(fileinfo.back_path);
    fileinfo.pack_sign=false;
    FileInfoManger::GetInstant()->Insert(fileinfo);
    fu.Remove();
  }

//判断是否为断点续传
  //  bool flag=false;
  // if(req.has_header("If-Range")){
  //   //断点续传
  //   std::string oldetag=req.get_header_value("If-Range"); 
  //   if(oldetag==GetEtag(fileinfo.back_path)){
  //       //需要断点续传
  //       cout<<"oldetag: "<<oldetag<<endl;
  //       cout<<"etag: "<<GetEtag(fileinfo.back_path)<<endl;                                                                                                                           
  //       flag=true;
  //   }
  // }
  
 // m_response->content_size=
   sjp::FileUtil fu(m_request->m_path);
   m_response->content_size=fu.GetFileSize();
   fd=open(m_request->m_path.c_str(),O_RDONLY);
  if(fd<0){
    cout<<"文件打开失败"<<endl;
    //打开失败
    //cout<<"m_request->m_path: "<<m_request->m_path<<endl;
   // cout<<"fd == -1"<<endl;
  }

  if(!m_request->old_etag.empty())
  {
    if(m_request->old_etag==GetEtag(m_request->m_path))
    {
        /*
       1. 将断点续传的标志设置为true
       2. 分析文件断点的起始位置，和传输的文件的大小
        */
        break_point_resume=true;
        //解析Range请求字段：
        //假设每次断点续传都是从文件 开始到文件结束请求整个文件结束
        //Range: bytes=100-
        //将文件的起始位置给 解析出来
        int pos=m_request->range.find("byte=");
        int pos1=m_request->range.find("-");
        string start_str=m_request->range.substr(pos+1,pos1-pos-1);
        start_pos=atoi(start_str.c_str());//转换为整数
        //start_int是请求文件的起始位置
        m_response->content_size=m_response->content_size-start_pos;
    }
  }
}
#endif


#if 0
int httpconn::do_request()
{
  //将url中的路径和参数给分开来
  //解析资源
  //判断文件是否存在
  //判断文件是否是一个可执行文件，如果是一个可执行文件，则调用cgi机制
  //判断一个文件是否一个普通文件，则将该文件提前打开，并返回相关文件的fd
  if(m_upload)
  {
    //解析post请求中的文件名和文件内容
    ParseUpLoadFile();
    //将文件上传到
   // UpLoad();
    return 0;
  }
  if(showList){
    return 0;//返回下载网页
  }

  if(downFile)
  {
   // ParseDownFile();
    return 0;
  }
  AnalyFile();
  if(cgi)
  {
    //处理cgi文件
     CgiHandle();
  }
  else if(IsSendPage){
    //处理发送静态网页
    OpenPage();
  }
}
#endif



void Response::BuildResponseLine()
{
     //构建响应行,版本 状态码 状态码描述
      m_line +="http/1.1";
      m_line +=" ";
      m_line +=m_code;
      m_line +=" ";
      m_line +=code_desc[m_code];
      m_line +=BLANK;
        //std::cout<<response.response_line<<std::endl;
}

void Response::BuildResponseHeader()
{
    string size=to_string(m_content_size);
    m_header.append("Content-Length: ");
    m_header.append(size.c_str());
    m_header.append(BLANK);//设置换行符
    m_header.append(BLANK);
}






    //构建请求报头

  #if 0
    void httpconn::BuildResponseHeaer(){
       //构建Content-Type: 
       //std::string content_type;
       if(downFile)
        {       
          cout<<"download"<<endl;
          cout<<"filename: "<<fileName<<endl;
          m_response->response_body+="Content-Type: application/octet-stream";   
          m_response->response_body+=BLANK;
          m_response->response_body+="Content-Disposition: attachment; filename=";
          m_response->response_body+=fileName;
          m_response->response_body+=BLANK;
          m_response->response_body+="Accept-Ranges: bytes";
          m_response->response_body+=BLANK;
         //  m_response->response_body+="Etag: ";
         // m_response->response_body+=GetEtag(m_request->m_path);
          //cout<<m_response->response_body<<endl;
         // m_response->response_body+=BLANK;       
          if(break_point_resume)  
          {
            //断点续传
            //Content-Range: bytes 200-1000/67589
            m_response->response_body+="Content-Range: bytes ";
             m_response->response_body+=to_string(start_pos);
             m_response->response_body+="-";
             m_response->response_body+=to_string(m_response->content_size+start_pos);
             m_response->response_body+="/";
             m_response->response_body+=to_string(m_response->content_size+start_pos);
             m_response->response_body+=BLANK;
          }
        }
        else{
          m_response->response_body="Content-Type: ";
          m_response->response_body+=suffix_type[m_response->suffix];
          m_response->response_body+=BLANK;
        }
        //m_response->response_body+=content_type;
        //构建Content-Length
        //非cgi的响应正文是文件的大小
        //cgi处理的响应正文是cgi程序处理的结果
        //构建响应
        std::string content_size="Content-Length: ";
        content_size+=std::to_string(m_response->content_size);
        content_size+=BLANK;
        m_response->response_body+=content_size;

        std::string connection="Connection: ";
        if(m_linger)
        {
          connection+="keep-alive";
        }
        else{
          connection+="close";
        }
        connection+=BLANK;
        m_response->response_body+=connection;
        m_response->response_body+=BLANK;
        cout<<m_response->response_body<<endl;
    }
#endif

//测试成功
//测试用例：abcdef abcdef\r\n abcdef\r
//从环形缓冲区中[判断是否有完整行
httpconn::LINE_STATUS httpconn::parse_line(){
  //从缓冲区中读取一行数据
  //遇到\r\n就停止
  int cur_data_len=0; 
  for( ;cur_data_len<m_sum_size;cur_data_len++){
      int pos=(m_read_head+cur_data_len)%read_buffer_size;
      if(read_buffer[pos]=='\r'){
        //m_sum_size表示当前缓冲区数据的长度
        //cur_data_len表示已经获取的缓冲区的长度
        if(cur_data_len+1>=m_sum_size){
          return LINE_OPEN;
        }
        else {
            
            //完整行
            int pos=(m_read_head+cur_data_len+1)%read_buffer_size;
             if(read_buffer[pos]=='\n')
             {     
               m_line_len=cur_data_len;      //-1的目的是减去 \r    
               return LINE_OK;
             }
            else{
                 return LINE_BAD;
            }
        }

      }
#if 0
      else if(m_check_idx>0&&read_buffer[m_check_idx+1]=='\n'){
           
           read_buffer[m_check_idx-1]='\0';
           read_buffer[m_check_idx++]='\0';
           return LINE_OK;
      }

#endif
  }
  return LINE_OPEN;
}

#if 0
//测试成功
//测试用例：abcdef abcdef\r\n abcdef\r
httpconn::LINE_STATUS httpconn::parse_line(){
  //从缓冲区中读取一行数据
  //遇到\r\n就停止
  //xxxxxx\r\nxxxxxx\r\n
  //m_check_idx检查缓冲区的位置
  //m_read_idx读取缓冲区的位置
  for( ;m_check_idx<read_buffer.size();m_check_idx++){
      if(read_buffer[m_check_idx]=='\r'){
        if(m_check_idx+1>=read_buffer.size()){
          return LINE_OPEN;
        }
        else if(read_buffer[m_check_idx+1]=='\n'){
             //完整行
              read_buffer[m_check_idx++]='\0';
              read_buffer[m_check_idx++]='\0';
              return LINE_OK;
        }
        else{
          return LINE_BAD;
        }
      }
      else if(m_check_idx>0&&read_buffer[m_check_idx+1]=='\n'){
           read_buffer[m_check_idx-1]='\0';
           read_buffer[m_check_idx++]='\0';
           return LINE_OK;
      }
  }
  return LINE_OPEN;
}
#endif

//测试成功
//获取请求方法，url
HTTP_CODE  Request::parse_request_line(std::string text){
    int pos=text.find(' ');
    if(pos==std::string::npos){
      return BAD_REQUEST;
    }
    //解析出请求方法
   // cout<<text.substr(0,pos)<<endl;
   string tmp=text.substr(0,pos);
    if(tmp=="GET"||tmp=="get"){
       m_method=Request::GET;
    }
    else if(tmp=="post"||tmp=="POST"){
       m_method=Request::POST;
    }
    else{
      return BAD_REQUEST;
    }

    //解析出url
    int pos1=text.find(' ',pos+1);
    m_url=text.substr(pos+1,pos1-pos-1);
    m_version=text.substr(pos1+1);
    AnalyUri();//解析url
    m_check_state=CHECK_STATE_HEADER;
 }


int Request::ParseHeader(string& key,string &value)
{
     if(key=="Content-Length"){
       int comtent_len=atoi(value.c_str());
        m_content_len=comtent_len;
        return 0;
     }
     else if(key=="Connection"){
        if(strcmp(value.c_str(),"keep-alive")==0){
            m_long_linker=true;
            return 0;
        }
     }
    else if(key=="Host"){   
        m_host =value;
        return 0;
    }
    else if(key=="Content-Type"){
      content_type=value;
       return 0;
    }
    else if(key=="If-Range")
    {
      m_etag=value;
       return 0;
    }
    else if(key=="Range")
    {
       m_range=value;
       return 0;
    }

    return -1;
}
//解析请求报头
//测试成功
HTTP_CODE Request::parse_request_header(std::string text){
     if(text=="\r\n"){
      //遇到空行
      //如果请求正文不等于0，则需要解析请求正文
      if(m_content_len!=0){
        m_check_state= CHECK_STATE_CONTENT;
        return NO_REQUEST;
      }
      return GET_REQUEST;//解析报头完成
    }

     //查找": ”
     int pos=text.find(":");
     if(pos==std::string::npos){
       return NO_REQUEST;
     }

     string key=text.substr(0,pos);
     string value=text.substr(pos+2);  
     ParseHeader(key,value);
    return NO_REQUEST;
}

//将数据从环形缓冲区给读取上来
HTTP_CODE httpconn::ReadContent()
{

    //读取请求正文
    //通过content-Length去判断是否读取完毕
    //m_request->getContetnLen()是整个请求正文的长度
    //m_cur_content_len是已经读取请求正文的长度
    //len是需要读取的正文长度
   //m_sum_size是缓冲区中可以读的数据  
    if(m_sum_size==0){
      // http请求中的正文没有发送过来
      return NO_REQUEST;
    }
    int len=m_request->getContentLen()-m_cur_content_len;
    string& content=m_request->getContent(); //获取到请求正文
#if 0
    cout<<"httpconn address:"<<this<<endl;
    cout<<"httpconn socket:"<<m_socket<<endl;
    cout<<"httpconn len:"<<len<<endl;
    cout<<"httpconn 缓冲区可以读的数据:"<<m_sum_size<<endl;
#endif
    if(len>m_sum_size)
    {
      //读取没有完成
      
      int cur_pos=m_read_head;
      //判断一行数据没有被一分为二
     
      if(cur_pos+m_sum_size<=read_buffer_size)
      {
        content.append(&read_buffer[m_read_head],m_sum_size);
      }
      else
      {
        
        int right_len=read_buffer_size-cur_pos;
        int left_len=m_sum_size-right_len;
        content.append(&read_buffer[m_read_head],right_len);
        content.append(read_buffer,left_len);
      }
           
      m_cur_content_len+=m_sum_size;//更新读取长度
      m_sum_size=0;//更新可读数据的长度
      m_read_head=(m_read_head+m_sum_size)%read_buffer_size; //更新可读数据的位置
      return NO_REQUEST;
    }else{
      //读取len数据
      //读取没有完成
      //m_sum_size是需要读取的数据       
      int cur_pos=m_read_head;
      //判断一行数据没有被一分为二
      if(cur_pos+len<read_buffer_size)
      {
         content.append(&read_buffer[m_read_head],len);
      }
      else
      {
        //数据被一分为二
        int right_len=read_buffer_size-cur_pos;
        int left_len=len-right_len;
        content.append(&read_buffer[m_read_head],right_len);
        content.append(read_buffer,left_len);
      }
           
      m_cur_content_len+=len;//更新读取长度
      m_sum_size-=len;//更新可读数据的长度
      m_read_head=(m_read_head+len)%read_buffer_size; //更新可读数据的位置
      return GET_REQUEST;
    }
    return NO_REQUEST;
}

#if 0
httpconn::HTTP_CODE httpconn::parse_request_content()
{
    //读取请求正文
    //通过content-Length去判断是否读取完毕
    //请求中的正文read_buffer.size()-m_check_idx
    if(read_buffer.size()-m_check_idx<m_request->m_content_len){
      return NO_REQUEST;
    }else{
      m_request->content=read_buffer.substr(m_check_idx,m_request->m_content_len);
      return GET_REQUEST;
    }
}

void httpconn::SendResponseHeader()
{
   int start=0;
  auto& response_body=m_response->response_body;
  //发送响应报头完成
  while(true)
  {
     //发送response_body
     int len=response_body.size()-start;
     int size=send(m_socket,response_body.c_str()+start,len,0);
      if(size>0){
        start+=size;
        if(start==response_body.size())
          break;
      }else{
        //发送失败
        return;
      }
  }
}
void httpconn::SendResponseContent()
{
  int content_size=m_response->content_size; 
  cout<<"content_size:"<<content_size<<endl;

  //std::cout<<"content_size: "<<content_size<<std::endl;
  off_t start=start_pos;
  while(true){
  //cout<<"m_socket: "<<m_socket<<endl;
 // cout<<"fd: "<<fd<<endl;
    int size=sendfile(m_socket,fd,&start,content_size);
    if(start+size>=content_size){
      cout<<"发送响应正文成功"<<endl;
      return ;
    }
    if(size<=0){
    //发送失败
    return ;
    }
  }          
}

#endif

//生成请求报头

#if 0
bool httpconn::Write()
{
  if()
  //先发送response_body后，
  //判断响应正文是发送静态网页还是发送response_content
  if(showList||m_upload){
 //   ShowList();
  //  return true;
  }
 SendResponseHeader();
 cout<<"发送响应报头成功"<<endl;

  if(IsSendPage||downFile)
    {
        //cout<<fd<<endl;
        //fd == -1
        if(fd!=-1){
         SendResponseContent();
        //LOG(INFO,"send page success! ");
       }
    }
    else if(cgi||showList||m_upload)
    {
      //发送cgi处理结果
      int start=0;
      auto& response_content=m_response->response_content;
      while(true)
      {
      int len=response_content.size()-start;
      int size=send(m_socket,response_content.c_str()+start,len,0);
      if(size>0){
          start+=size;
          if(start==response_content.size())
            break;
      }else{
            //发送失败
          return false;
      }
    }
  }
  cout<<"发送响应正文成功"<<endl;
  return true;
}
#endif



bool httpconn::Write()
{
  if(m_response->Send(m_socket)==0){
      return true;
  }
  else{
    return false;
  }
}

void httpconn::clear()
{
  init();
  //delete m_request;
  //delete m_response;
}

void httpconn::init()
{
  m_request->init();
  m_response->init();
//  m_cur_content_len=0;
  
 // m_read_idx=0;
 // m_read_start=0;
  // m_check_idx=0;
  // m_line_idx-0;
  // m_start_line=0;

  //m_start_content=0;
  start_pos=0;
//  file_size=0;
  cgi=false;
}

//解析http上传文件
/*
------WebKitFormBoundaryAhWTT8nb3TdW3yMj
Content-Disposition: form-data; name="file"; filename="111.txt"
Content-Type: text/plain

111111111
------WebKitFormBoundaryAhWTT8nb3TdW3yMj--
*/

#if 0
void httpconn::ParseUpLoadFile()
{
  auto& content_type=m_request->content_type;
  int pos=content_type.find("boundary");
  if(pos==-1){
    cout<<"no boundary"<<endl;
    return;
  }

  string boundry=content_type.substr(pos+9);
  auto& content=m_request->content;
  //cout<<content<<endl;
  pos=content.find("filename");
  if(pos!=-1){
    int pos1=content.find('"',pos+10);
    fileName=content.substr(pos+10,pos1-pos-10);
  }
  //pos2+4是文件内容的起始位置
  //pos3是文件边界的起始位置
  //111111111\r\n------WebKitFormBoundaryAhWTT8nb3TdW3yMj--
  //pos3-pos2-4是111111111\r\n的字符个数
  //pos3-pos2-6是111111111的字符个数
  int pos2=content.find("\r\n\r\n");
  if(pos2!=-1){
    int pos3=content.find(boundry,pos2+4);
    file_content=content.substr(pos2+4,pos3-pos2-6);
  }

  cout<<fileName<<endl;
  cout<<file_content<<endl;

  // cout<<"------------"<<endl;
  // cout<<file_content<<endl;
  // cout<<"------------"<<endl;
}
#endif

int Response::SetCode(string code)
{
  m_code=code;
}

//"Content-Type: application/octet-stream";   
void Response::SetHeader(string key,string value){
      if(key=="Content-Length")
      {
        return;
      }
      m_header.append(key);
      m_header.append(": ");
      m_header.append(value);
      m_header.append(BLANK);
}

void Response::SetContent(string content){
     m_content=content;
     m_content_size=content.size();
}

void Response::SetVersion(string version)
{
     m_version=version;
}
      


  void httpconn::setReadBuffer(char* s)
  {
    int len=strlen(s);

    m_sum_size=len;
    strncpy(read_buffer,s,len);
  }


Request* httpconn::GetRequest()
{
  return m_request;
}
Response* httpconn::GetResponse()
{
  return m_response;
}