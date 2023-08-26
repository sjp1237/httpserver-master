#include"TcpServer.hpp"

TcpServer* TcpServer::tcp_svr=nullptr;

TcpServer* TcpServer::GetTcpServer(int _port)
{
      static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
      if(tcp_svr==nullptr)
      {
        pthread_mutex_lock(&lock); 
        if(tcp_svr==nullptr)
        {
          tcp_svr=new TcpServer(_port);
          tcp_svr->InitServer();
        }
        pthread_mutex_unlock(&lock);
        return tcp_svr;
      }
}

int TcpServer::Accept(){
    struct sockaddr_in addr;
    socklen_t addrlen;
    int fd=accept(listen_sock,(struct sockaddr*)&addr,&addrlen);
    if(fd>0){
       // LOG("INFO","accept success");
    return fd;
    }
    else{
      return -1;
    }
}



int TcpServer::GetListenfd()
{
    return listen_sock;
}


void TcpServer::InitServer()
{
    Socket();
    Bind();
    Listen();
//  LOG("INFO","Tcp InitServer success");
//    LOG(INFO,"InitServer Success..");
}

void TcpServer::Socket()
{
    listen_sock=socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock<0)
    {
    std::cout<<"socket error"<<std::endl;
    exit(1);
    }
    //LOG(INFO,"Socket success");
    int opt=1;
    setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
}

void TcpServer::Bind()
{
     struct sockaddr_in local;
     memset(&local,'\0',sizeof(local));
     local.sin_family=AF_INET;
     local.sin_port=htons(port);
     local.sin_addr.s_addr=INADDR_ANY;
     std::cout<<port<<std::endl;
     if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local))<0)
     {
       std::cout<<"bind error"<<std::endl;
       exit(2);
     }
   //  LOG(INFO,"bind success");
   }

void TcpServer::Listen()
{
     if(listen(listen_sock,BACKLOG)<0)
     {
       std::cout<<"Listen error"<<std::endl;
       exit(3);
     }
   //  LOG(INFO,"Listen success");
}


