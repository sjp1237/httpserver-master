#include"reactor.hpp"
#include"../threadpool/threadpool.hpp"
#include"../api/comm.h"
#include <signal.h>
#include"../log/log.h"

#define TIMESLOT 100

#if 0
void signal_handler_fun(int signum) {
	Reactor::getInstant()->setAlarm(true);
	alarm(TIMESLOT);
   // printf("catch signal %d\n", signum);
}
void timer_cb(void* client_data)
{
	event* ev=(event*)client_data;
	Reactor::getInstant()->event_del(ev);
}
#endif 

Reactor::Reactor()
{

}
Reactor* Reactor::instant=new Reactor;
Reactor* Reactor::getInstant()
{
	return instant;
}



int recv_cb(void* arg)
{
	event* ev=(event*)arg;
	httpconn* conn=ev->conn;

	//读取缓冲区中的数据
	conn->Read();
	//cout<<"read_buffer"<<conn->read_buffer<<endl;
}
#if 0
int recv_cb(void* arg)
{
	event* ev=(event*)arg;
	httpconn* conn=ev->conn;

	//读取缓冲区中的数据
	conn->Read();
	//解析出request
	HTTP_CODE ret=conn->process_read();
	if(ret==NO_REQUEST)
	{
		//请求没有完成

	}
	else if(ret==BAD_REQUEST)
	{
		//获取到一个错误的请求，进行
		Response* response=conn->GetResponse();
		response->SetCode("404");
		return 0;
	}
	//获取到了一个完整请求
	else if(ret==GET_REQUEST)
	{
		//进行业务处理
		Request* request=conn->GetRequest();
		if(request->isLongLinker())
		{
			ev->m_longlinker=true;
		}
		else{
			ev->m_longlinker=false;
		}

		#if 0
		cout<<request->getPath()<<endl;
		cout<<request->getContent()<<endl;
		#endif
		Response* response=conn->GetResponse();
		response->SetCode("200");
		//业务处理完毕
		return 0;
	}	
}
#endif

int write_cb(void* arg)
{
	event* ev=(event*)arg;
	httpconn* conn=ev->conn;
	bool ret=conn->Write();//发送数据
	if(ret==true){
		//发送数据成功
		//发送数据完毕
		ev->conn->init();//初始化
		//将事件重新添加到epoll里面
		if(ev->m_longlinker)
		{
			//保持长连接
			//切换监控读事件
			ev->conn->init();
			Reactor::getInstant()->event_add(EPOLLIN,ev);
		}else{
			//关闭连接
			//sort_timer_list::getinstant()->del_timer(ev->timer);
			Reactor::getInstant()->event_del(ev);
		}
		return 0;
	}
	else {
		//发送失败，继续监控写事件
		Reactor::getInstant()->event_add(EPOLLOUT,ev);
	}
}

void event::task_cb()
{
	if(active==false)
	{
		//该事件被关闭了
	
		return;
	}
	//cout<<"phthead id:"<<pthread_self()<<endl;
	HTTP_CODE ret=conn->process_read();
	if(ret==NO_REQUEST)
	{
		//请求没有完成,继续监控读事件
		status=0;
		Reactor::getInstant()->event_add(EPOLLIN,this);
		return;
	}
	else if(ret==BAD_REQUEST)
	{
		//获取到一个错误的请求，进行
		Response* response=conn->GetResponse();
		response->SetCode("404");
        LOG_INFO("request error ");
		//cout<<"request error"<<endl;
		status=0;
		Reactor::getInstant()->event_add(EPOLLOUT,this);
		return ;
	}
	//获取到了一个完整请求
	else if(ret==GET_REQUEST)
	{
		//进行业务处理
		Request* request=conn->GetRequest();
		if(request->isLongLinker())
		{
			m_longlinker=true;
		}
		else{
			m_longlinker=false;
 		}
      //  cout<<"content-length:"<<request->getContentLen()<<endl;
	    LOG_DEBUG("url: %s",request->getUrl().c_str());
	    Response* response=conn->GetResponse();
	    handle_business(request,response);
		
		//业务处理完毕
		//监控写事件
		status=0; //重新添加写事件
		Reactor::getInstant()->event_add(EPOLLOUT,this);
		return ;
	}

}
#if 0
void event::task_cb()
{
	if(events==EPOLLIN)
	{
		int ret=recv_cb(this);
		if(ret==0)
		{
			//读取数据，解析请求完毕，业务处理完毕
			//或者请求错误，发送错误请求页面

			status=0;
			Reactor::getInstant()->event_add(EPOLLOUT,this);
		}
		else if(ret==-1)
		{
			//请求还没完成,继续等待数据
		   status=0;
			Reactor::getInstant()->event_add(EPOLLIN,this);
		}
	}


	else if(events==EPOLLOUT)
	{
		int ret=write_cb(this);
		if(ret==0)
		{
			//发送数据完毕
			this->conn->init();//初始化

			//将事件重新添加到epoll里面
			if(m_longlinker)
			{
			  //保持长连接
			  status=0;
			  Reactor::getInstant()->event_add(EPOLLIN,this);
			}else{
				//关闭连接
			 Reactor::getInstant()->event_del(this);
			}
		}else if(ret==-1)
		{
			//数据还没有发送完成
			//将socket继续将继续等待写
			status=0;
			Reactor::getInstant()->event_add(EPOLLOUT,this);
		}
	}
}
#endif

int Reactor::init()
{
	m_threadpool=new  threadpool<event>;
	m_evblks = (struct eventblock *) malloc(sizeof(struct eventblock));
	memset(m_evblks, 0, sizeof(struct eventblock));
	if (m_evblks == NULL) {
		printf("reactor_alloc eventblock failed\n");
		return -3;
	}
	
    m_listenfd=TcpServer::GetTcpServer()->GetListenfd();

	m_epfd = epoll_create(1);
	if (m_epfd <= 0) {
		printf("create epfd in %s err %s\n", __func__, strerror(errno));
		return -2;
	}


	struct event* evs = (struct event*)malloc((MAX_EPOLL_EVENTS) * sizeof(struct event));
	if (evs == NULL) {
		printf("create epfd in %s err %s\n", __func__, strerror(errno));
		close(m_epfd);
		return -3;
	}
	memset(evs, 0, (MAX_EPOLL_EVENTS) * sizeof(struct event));

	struct eventblock *block = (struct eventblock*)malloc(sizeof(struct eventblock));
	if (block == NULL) {
		free(evs);
		close(m_epfd);
		return -3;
	}
	m_evblks->events = evs;
	m_evblks->next = NULL;
    m_blkcnt = 1;	

#if 0
	alarm(TIMESLOT);
	signal(SIGALRM, signal_handler_fun);
#endif
}



int Reactor::run() {

	if (m_epfd < 0) return -1;
	if (m_evblks == NULL) return -1;
	
	//设置信号处理函数

	struct epoll_event events[MAX_EPOLL_EVENTS+1];
	int checkpos = 0, i;
	struct event* event=reactor_idx(m_listenfd) ;
	event_set(event, m_listenfd, nullptr,nullptr);
	event_add(EPOLLIN,event);
	while (1) {
        //监听套接字
		int nready = epoll_wait(m_epfd, events, MAX_EPOLL_EVENTS, 1000);
		
		if (nready < 0) {
			printf("epoll_wait error, exit\n");
			continue;
		}

		for (i = 0;i < nready;i ++) {
			int sockfd =events[i].data.fd;
			//处理新到的客户连接
            if (sockfd == m_listenfd)
            {
                bool flag = dealclinetdata();
            }
			else if ((events[i].events & EPOLLIN)) {
		  	//	cout<<nready<<":"<<events[i].data.fd<<endl;
				 dealwithread(sockfd);
			}
			else if ((events[i].events & EPOLLOUT)) {
				 dealwithwrite(sockfd);
			}
			
		}

#if 0
		if(is_alarm)
		{
			//检测是否有过期事件
			sort_timer_list::getinstant()->tick();
			setAlarm(false);
		}
#endif
	}
}


bool Reactor::dealclinetdata()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);

	int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
	//新的连接
	cout<<"accept linker fd: "<<connfd<<endl;
	if (connfd < 0)
	{
		//LOG_ERROR("%s:errno is:%d", "accept error", errno);
		return false;
	}
	struct event* event=reactor_idx(connfd);


	event_set(event, connfd, recv_cb,write_cb);
    event_add(EPOLLIN,event);


    return true;
}

void Reactor::dealwithread(int sockfd)
{
	struct event* event=reactor_idx(sockfd) ;

	//
	//event->timer->expire+=EXPIRE_TIME;
   // sort_timer_list::getinstant()->adjust_timer(event->timer);
	event_stop(sockfd);//暂停对事件的监控
	event->readcb(event);//调用读取数据
	if(event->conn->m_sum_size>0)
	{
      m_threadpool->push(event);
	}
 
}


void Reactor::dealwithwrite(int sockfd)
{
	struct event* event=reactor_idx(sockfd) ;
	//event->timer->expire+=EXPIRE_TIME;
  //  sort_timer_list::getinstant()->adjust_timer(event->timer);
	event->writecb(event);
	//event_stop(sockfd);//暂停对事件的监控
}


 int Reactor::event_stop(int socket)
 {
	struct epoll_event ep_ev = {0, {0}};
	if(epoll_ctl(m_epfd,EPOLL_CTL_DEL,socket,&ep_ev)<0){
		printf("event delete failed [fd=%d]\n", socket);
	}
 }

//将事件设置到epoll中
//events是读事件还是写事件
int Reactor::event_add(int events, struct event *ev) {
    //创建一个epollevent 
	struct epoll_event ep_ev = {0, {0}};
	
//	ep_ev.data.ptr = ev;
	ep_ev.events = ev->events = events;
	ep_ev.data.fd=ev->socket;

	int op;
	if (ev->status == 1) {
		op = EPOLL_CTL_MOD;
	} else {
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	ev->active=true;
	if (epoll_ctl(m_epfd, op, ev->socket, &ep_ev) < 0) {
		printf("event add failed [fd=%d], events[%d]\n", ev->socket, events);
		//cout<<errno<<endl;
		return -1;
	}

	return 0;
}


void Reactor::event_set(struct event *ev, int socket, NCALLBACK readcb,NCALLBACK writecb) {

	ev->socket = socket;
	ev->readcb=readcb;
	ev->writecb=writecb;
	//ev->callback = callback;
	ev->events = 0;
	//ev->arg = arg;
	//ev->last_active = time(NULL);

	if(socket!=m_listenfd)
	{
		//将socket设置为非阻塞
		int flag = fcntl(socket, F_GETFL);
       flag |= O_NONBLOCK;
       int ret = fcntl(socket, F_SETFL, flag);
		httpconn* conn=new httpconn(socket);
        ev->conn=conn;

	#if 0
	    //创建定时器
		time_t m_time;
	    int curtime=time(&m_time);
	    util_timer* timer=new util_timer(curtime);
	    timer->user_data=(void*)ev;
     	timer->cb_func=timer_cb;
    	sort_timer_list::getinstant()->add_timer(timer);
    	ev->timer=timer;
#endif
	}
	return ;
}

int Reactor::reactor_alloc() {

	if (m_evblks == NULL) return -1;
	
	struct eventblock *blk = m_evblks;

	while (blk->next != NULL) {
		blk = blk->next;
	}

	struct event* evs = (struct event*)malloc((MAX_EPOLL_EVENTS) * sizeof(struct event));
	if (evs == NULL) {
		printf("reactor_alloc event failed\n");
		return -2;
	}
	memset(evs, 0, (MAX_EPOLL_EVENTS) * sizeof(struct event));

	struct eventblock *block = (struct eventblock *) malloc(sizeof(struct eventblock));
	memset(block, 0, sizeof(struct eventblock));
	if (block == NULL) {
		printf("ntyreactor_alloc eventblock failed\n");
		return -3;
	}

	block->events = evs;
	block->next = NULL;

	blk->next = block;
	m_blkcnt ++;

	return 0;
}

struct event* Reactor::reactor_idx(int sockfd) {

	if (m_evblks == NULL) return NULL;
	int blkidx = sockfd / MAX_EPOLL_EVENTS;
	while (blkidx >= m_blkcnt) {
		reactor_alloc();
	}

	int i = 0;
	struct eventblock *blk = m_evblks;
	while (i++ != blkidx && blk != NULL) {
		blk = blk->next;
	}

	return &blk->events[sockfd % MAX_EPOLL_EVENTS];
}

#if 0
int accept_cb(int listen_fd, int events, void *arg) {
	Reactor* reactor=(Reactor*) arg;
   //获取新连接
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int clientfd;

	if ((clientfd = accept(listen_fd, (struct sockaddr*)&client_addr, &len)) == -1) {
		if (errno != EAGAIN && errno != EINTR) {
			
		}
		printf("accept: %s\n", strerror(errno));
		return -1;
	}

	//将文件描述符设置为非阻塞
	int flag = 0;
	if ((flag = fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) {
		printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
		return -1;
	}

	//将新连接存放进反应堆里面
	struct event *event =reactor-> reactor_idx(clientfd);
	if (event == NULL) return -1;
		
	reactor->event_set(event, clientfd, recv_cb);
	reactor->event_add(EPOLLIN, event);

#if 0
    //测试代码
	if (curfds++ % 1000 == 999) {
		struct timeval tv_cur;
		memcpy(&tv_cur, &tv_begin, sizeof(struct timeval));
		
		gettimeofday(&tv_begin, NULL);

		int time_used = TIME_SUB_MS(tv_begin, tv_cur);
		printf("connections: %d, sockfd:%d, time_used:%d\n", curfds, clientfd, time_used);
	}
#endif
	//printf("new connect [%s:%d], pos[%d]\n", 
	//	inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), clientfd);

	return 0;

}




//读取数据
int recv_cb(int fd, int events,void* arg) {
	Reactor* reactor=(Reactor*) arg;
	//获取事件
	struct event* ev = reactor->reactor_idx(fd);
	if (ev == NULL) return -1;
	//读取缓冲区
	int len = recv(fd, ev->ruffer, BUFFER_LENGTH, 0);

	if (len > 0) {
		
		ev->rlength = len;
		ev->ruffer[len] = '\0';
		if(len==BUFFER_LENGTH)
		{

		}
		else 
		{
			//没有数据可以读了
			reactor->event_del(ev);
		}
		//printf("recv [%d]:%s\n", fd, ev->buffer);

		//http_request(ev); // parser http hdr

		//设置读的回调函数
		reactor->event_set(ev, fd, send_cb);
		reactor->event_add(EPOLLOUT, ev);
				
	} else if (len == 0) {

		reactor->event_del(ev);
		//printf("recv_cb --> disconnect\n");
		close(ev->fd);
		 
	} else {

		if (errno == EAGAIN && errno == EWOULDBLOCK) { //
			reactor->event_del(ev);
			//没有数据可以读，
		} else if (errno == ECONNRESET){
			reactor->event_del(ev);
			close(ev->fd);
		}
		//printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));	
	}
	return len;
}


int send_cb(int fd, int events,void *arg) {
	Reactor* reactor=(Reactor*) arg;
	struct event *ev = reactor->reactor_idx(fd);

	if (ev == NULL) return -1;

	//nty_http_response(ev); //encode

	int len = send(fd, ev->wbuffer, ev->wlength, 0);
	if (len > 0) {
		//printf("resource: %s\n", ev->resource);
#if 0
		int filefd = open(ev->resource, O_RDONLY);
		//if (filefd < 0) return -1;
		
		struct stat stat_buf;
		fstat(filefd, &stat_buf);
		
		int flag = fcntl(fd, F_GETFL, 0);
		flag &= ~O_NONBLOCK;
		fcntl(fd, F_SETFL, flag);
			
		int ret = sendfile(fd, filefd, NULL, stat_buf.st_size);
		if (ret == -1) {
			printf("sendfile: errno: %d\n", errno);
		}

		flag |= O_NONBLOCK;
		fcntl(fd, F_SETFL, flag);

		close(filefd);

		send(fd, "\r\n", 2, 0);
#endif

		//设置为
	    //reactor->event_del(ev);
		reactor->event_set(ev, fd, recv_cb);
		reactor->event_add(EPOLLIN, ev);
		
	} else {
		reactor->event_del( ev);
		close(ev->fd);
		//printf("send[fd=%d] error %s\n", fd, strerror(errno));
	}
	return len;
}
#endif 

int Reactor::event_del(struct event *ev) {

	//关闭连接
	close(ev->socket);
	struct epoll_event ep_ev = {0, {0}};

	if (ev->status != 1) {
		return -1;
	}

	ev->active=false;//该事件被关闭

	delete ev->conn;
	ev->conn=nullptr;
	ep_ev.data.ptr = ev;
	ev->status = 0;
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, ev->socket, &ep_ev);
	return 0;
}