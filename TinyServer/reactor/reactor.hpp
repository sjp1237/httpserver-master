
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>


#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include"../httpconn/httpconn.hpp"
#include"../threadpool/threadpool.hpp"
#include"../tcpserver/TcpServer.hpp"
#include"../lst_timer/lst_timer.h"

#define BUFFER_LENGTH 1024
#define MAX_EPOLL_EVENTS 1024


#if 0
int recv_cb(int fd, int events,void *arg); //接收数据
int send_cb(int fd, int events,void *arg) ;//发送数据
int accept_cb(int fd, int events, void *arg);//获取新连接
#endif

typedef int (* NCALLBACK)(void *arg);
int recv_cb(void* arg);
int write_cb(void* arg);
class  event{
public:
    int socket;
    int events;  //判断是读时间还是写事件,0是读事件，1是写事件
	int status;  //判断使添加事件还是修改事件

    event();
    httpconn* conn;
  //  util_timer* timer;
    void task_cb();//提供给线程池中的线程去运行
    NCALLBACK writecb;
    NCALLBACK readcb; 
    bool m_longlinker=false;//是否为长连接

    bool active=false;//用来判断事件是否活跃
#if 0
    char* resource;
    NCALLBACK callback;
    NCALLBACK acceptcb;
#endif
    void* arg;                 //获取到reactor指针
};

//
struct eventblock {
	struct eventblock *next;
	struct event *events;
};


class Reactor
{
public:
    static Reactor* getInstant();
public:
   int init();
   int run();
    int reactor_alloc() ;//申请 eventblock 内存
    bool dealclinetdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);
   //将文件描述符设置进反应堆里
    struct event* reactor_idx(int sockfd) ;

    //设置event结构体中的数据
    void event_set(struct event *ev, int socket, NCALLBACK readcb,NCALLBACK writecb) ;
    //将事件添加到epoll中
    int event_add(int events, struct event *ev) ;
    //将事件从epoll中删除
    int event_del(struct event *ev);
    int event_stop(int socket); 
    void setAlarm(bool flag){
        is_alarm=flag;
    }
private:
	int m_epfd;
	int m_listenfd;
	int m_blkcnt;//记录 m_evblks的个数
	struct eventblock *m_evblks;
    threadpool<event>* m_threadpool;
    //单例模式
    Reactor();
    Reactor(Reactor&)=delete;
    static Reactor* instant;

    bool is_alarm;
};