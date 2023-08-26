#include"reactor/reactor.hpp""
#include"MysqlPool/DBPool.h"
#include"RedisPool/CachePool.h"
#include"log/log.h"
// //注册信号
// void addsig(int sig,void(handler)(int)){
//   struct sigaction sa;
//   memset(&sa,'\0',sizeof(sa));
//   sa.sa_handler=handler;
//   sigfillset(&sa.sa_mask);
//   sigaction(sig,&sa,NULL);
// }






//整体流程
int main(int argv,char* argc[]){
    Reactor* reactor=Reactor::getInstant();
    CDBPool* mysqlpool=new CDBPool("cloudserver","127.0.0.1",3306,"root","Sjp325050,,,","cloudserver",4);
    mysqlpool->Init();
    CDBManager::getInstance()->addDBCPool(mysqlpool);

    CachePool* redispool=new CachePool("token","127.0.0.1",6379,0,"",4);
    redispool->Init();
    CacheManager::getInstance()->addCachePool(redispool);

    CachePool* redispool1=new CachePool("ranking_list","127.0.0.1",6379,0,"",4);
    redispool->Init();
    CacheManager::getInstance()->addCachePool(redispool1);

    Log::get_instance()->init("server.log",0,8192,500000,6);
    reactor->init();
    reactor->run();
}



