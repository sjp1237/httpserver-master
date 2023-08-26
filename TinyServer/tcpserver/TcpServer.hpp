#pragma once
#include<iostream>

#include<sys/types.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#define PORT 8081
#define BACKLOG 5  
#include"../Log.hpp"
class TcpServer
{
  private:
    int port;
    int listen_sock;
    static TcpServer* tcp_svr;
  private:
    TcpServer(int _port):port(_port),listen_sock(-1)
    {}
    int Accept();
    void InitServer();
    void Socket();
    void Bind();
    void Listen();
  public:
    static TcpServer* GetTcpServer(int _port=PORT);
   int GetListenfd();
   ~TcpServer()
   {}
};

