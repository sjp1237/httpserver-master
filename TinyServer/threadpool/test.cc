#include"threadpool.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int nums = 0;




  
struct task{
    static  int done ;
    static  pthread_mutex_t lock;
    public:
    void process(){
     usleep(10000);
      pthread_mutex_lock(&lock);
      done++;
      printf("%d:doing %d task\n",pthread_self(), done);
      pthread_mutex_unlock(&lock);
    }
};
int task::done = 0;
pthread_mutex_t task::lock;

int main()
{
    threadpool<task>* pool=new threadpool<task>();
    if (pool == NULL) {
        printf("thread pool create error!\n");
        return 1;
    }

    task* t=new task;
    while (pool->push(t)==0) {
       /// pthread_mutex_lock(&lock);
        nums++;
      //  pthread_mutex_unlock(&lock);
        t=new task;
    }

    printf("add %d tasks\n", nums);
    
    pool->wait_all_done();

   // printf("did %d tasks\n", done);
}