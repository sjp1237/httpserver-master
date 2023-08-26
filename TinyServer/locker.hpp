#ifndef LOCKER
#define LOCKER
#include<pthread.h>
#include<exception>
#include<semaphore.h>
//互斥锁类
class locker{
  public:
    locker(){
      if(pthread_mutex_init(&m_mutex,NULL)!=0){
        throw std::exception(); 
      }
    }
      ~locker(){
         pthread_mutex_destroy(&m_mutex);
      }

      bool lock(){
        return pthread_mutex_lock(&m_mutex)==0;
      }

      bool unlock(){
        return pthread_mutex_unlock(&m_mutex)==0;
      }

      pthread_mutex_t getMutex(){
        return m_mutex;
      }
  private:
    pthread_mutex_t m_mutex;
};

//条件变量类

class cond{
  public:
    cond(){
      if(pthread_cond_init(&m_cond,NULL)!=0){
        throw std::exception();
      }
    }

    ~cond(){
      pthread_cond_destroy(&m_cond);
    }
    //等待
    bool wait(pthread_mutex_t& lock){
      return pthread_cond_wait(&m_cond,&lock)==0;
    }

    bool signal(){
      return pthread_cond_signal(&m_cond)==0;
    }
  private:
    pthread_cond_t m_cond;
};

//信号量类
class sem{
  public:
  sem(){
    if(sem_init(&m_sem,0,0)!=0){
      throw std::exception();
    }
  }
  sem(int num){
    if(sem_init(&m_sem,0,num)!=0){
      throw std::exception();
    }
  }
  //对信号量加锁，调用一次对信号量的值-1，如果值为0，就阻塞
  bool wait(){
     sem_wait(&m_sem);
     return true;
  }
  //增加信号量
  bool post(){
     sem_post(&m_sem);
     return true;
  }
  private:
  sem_t m_sem;
};
#endif
