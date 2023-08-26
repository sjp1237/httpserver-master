#pragma once
#include<iostream>
#include<mutex>
#include<pthread.h>
#include<vector>
#include<list>
#include<queue>

using namespace std;
#define QUEUECOUNT 1000
#define POOLCOUNT 4

//上下文信息
//系统调用
#if 0
typedef  void*(*handler_pt)(void*);
typedef struct task_t
{
    task_t(){
        cb=nullptr;
        arg=nullptr;
    }

    handler_pt cb;//任务处理函数
    void* arg;//任务上下文
}task_t;
#endif

//任务队列
template <typename T>
struct task_queue_t{
    task_queue_t(int _count=QUEUECOUNT)
    {
        if(_count<=0){
            throw "queue is zero";
        }

        max_count=_count;
        cur_count=0;
        queue=new T[max_count];
        if(queue==nullptr)
        {
            throw bad_alloc();
        }
        head=0;
        tail=0;
    }

    ~task_queue_t()
    {
        if(queue!=nullptr){
            delete queue;
            queue=nullptr;
        }
    }

    size_t head;
    size_t tail;
    size_t max_count;//最大的任务数量
    size_t cur_count;
    T* queue;
};


template <typename T>
class threadpool
{
    public:

    ~threadpool();
    int push(T* task);
    int wait_all_done();
     threadpool(size_t _thrd_count=POOLCOUNT,size_t _task_size=QUEUECOUNT);
    private:

    int thread_pool_free();
    static void* routine(void* arg);
   
    private:
    
    //条件变量，互斥量，任务池，线程
    pthread_mutex_t lock;
    pthread_cond_t cond;

    pthread_t* pool=nullptr;//线程池
    size_t thrd_count;//线程池数量
    int started;//当前运行的线程数。

    bool close=false;//是否关闭线程
    task_queue_t<T*>* task_queue;
    int m_errno=0;
};






template <typename T>
int threadpool<T>::push(T* task){
    
    if(pool==nullptr||task_queue==nullptr){
        return -1;
    }

    if(pthread_mutex_lock(&lock)==-1){
        return -1;
    }

    if(close){
        pthread_mutex_unlock(&lock);
        return -1;
    }

    if(task_queue->cur_count==task_queue->max_count){
        pthread_mutex_unlock(&lock);
        return -1;
    }

    cout<<task_queue->cur_count<<endl;
    task_queue->queue[task_queue->tail]=task;
    if(pthread_cond_signal(&cond)!=0){
        pthread_mutex_unlock(&lock);
        return -1;
    }

    task_queue->tail=(task_queue->tail+1)%task_queue->max_count;
    task_queue->cur_count++;
    pthread_mutex_unlock(&lock);
    return 0;
}

template <typename T>
void* threadpool<T>::routine(void* arg)
{
    threadpool<T>* pool=(threadpool<T>*) arg;

    while(true)
    {
        /*
        1.加锁，进入请求池获取任务
        2.如果没有任务，则阻塞在条件变量中，解锁
        3.如果有任务，取出任务则运行该任务
        */

       pthread_mutex_lock(&pool->lock);

       while(pool->task_queue->cur_count==0&&pool->close==false)
       {
          pthread_cond_wait(&pool->cond,&pool->lock);
       }
       
       if(pool->close==true)break;
       task_queue_t<T*>* queue=pool->task_queue;
       T* t=queue->queue[queue->head];
       pool->task_queue->head=(pool->task_queue->head+1)%pool->task_queue->max_count;
       pool->task_queue->cur_count-=1;//任务数量-1
       pthread_mutex_unlock(&pool->lock);
        t->task_cb();
    }
    //退出线程
    pool->started--;//活跃线程减1
    pthread_mutex_unlock(&pool->lock);
  
    pthread_exit(NULL);
    return NULL;
}

template <typename T>
threadpool<T>::threadpool(size_t _thrd_count,size_t _task_size)
{
    pool=new pthread_t[thrd_count];
    if(pool==nullptr)
    started=0;
    task_queue=new task_queue_t<T*>(_task_size);
    if(task_queue==nullptr)
    {
        throw bad_alloc();
    }

    pthread_mutex_init(&lock,NULL);
    pthread_cond_init(&cond,NULL);

    for(int i=0;i<_thrd_count;i++)
    {
        pthread_create(&pool[i],NULL,routine,this);
        thrd_count++;
        started++;
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    /*
    1.先请求池中的任务全部处理完
    2.在退出所有的线程
    3.释放线程池资源
    */

   if(pool==nullptr){
    return;
   }
  if(pthread_mutex_lock(&lock)<0){
    //加锁失败
     m_errno=2;
     return;
   }
   if(close==true)  
   {
     thread_pool_free();
     return;
   }

   close=true;
   if(pthread_cond_broadcast(&cond)<0||pthread_mutex_unlock(&lock)<0){
      thread_pool_free();
      m_errno=3;
      return;
   }

   wait_all_done();
   thread_pool_free();
   return;
}

template <typename T>
int threadpool<T>::thread_pool_free()
{

    if(pool!=nullptr||started>0){
        //还有活跃线程，不能还不能销毁资源
        return -1;
    }

    delete[] pool;
    pool=nullptr;

    if(task_queue!=nullptr)
    {
      delete task_queue;
      task_queue=nullptr;
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    started=0;
    thrd_count=0;
}

template <typename T>
int threadpool<T>::wait_all_done()
{
    int ret=0;
    if(pool==nullptr){
        return 1;
    }

    for(int i=0;i<thrd_count;i++)
    {
        if(pthread_join(pool[i],NULL)!=0){
            ret=1;
        }
    }
    return ret;
}


#if 0
template <typename T>
threadpool<T>* threadpool<T>::getinstant()
 {
    if(instant ==nullptr)
    {
        pthread_mutex_lock(m_lock);
        if(instant==nullptr){
            instant=new threadpool<T>;
        }
        pthread_mutex_unlock(m_lock);
    }
    
    return instant;
 }
 #endif