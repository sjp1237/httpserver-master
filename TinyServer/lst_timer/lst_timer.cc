#include"lst_timer.h"


  sort_timer_list* sort_timer_list::instant=new sort_timer_list;
  sort_timer_list* sort_timer_list::getinstant()
  {
    return instant;
  }

    sort_timer_list::sort_timer_list():head(nullptr),tail(nullptr){}
    sort_timer_list::~sort_timer_list(){
      util_timer*  cur=head;
      while(cur!=NULL){
        util_timer* tmp=cur;
        cur=cur->next;
        delete tmp;
      }
    }


    //将目标定时器添加到链表当中
    //add_timer测试完毕
    void sort_timer_list::add_timer(util_timer* timer){
      //如果目标定时器超时时间小于当前链表中所有定时器的超时时间
      //把该定时器插入到链表的头部，作为链表的新的头节点
      //否则调用重载函数add_timer,把它插入到链表中合适的位置，以保证升序的特性
      
      if(!timer)
      return;
      //如果链表为空，则将head和tail设置为timer；
      if(head==nullptr)
      {
         head=tail=timer;
         return;
      }

      //timer的超时时间比head还要小，则将timer插入到链表的最前面
      if(head->expire>=timer->expire){
        timer->next=head;
        head->pre=timer;
        head=timer;
        return;
      }
      //将timer插入到head之后的节点
      add_timer(timer,head);
      return;
    }

    //将timer插入到lst_head后面部分的链表中
    void sort_timer_list::add_timer(util_timer* timer,util_timer* lst_head){
       //找到超时时间比timer小的节点next_timer的节点，然后插入到next_timer该节点前面
       if(!timer||!lst_head){
        return;
       }

       util_timer* next_timer=lst_head->next;

       while(next_timer)
       {
         if(timer->expire<=next_timer->expire)
         {
           lst_head->next=timer;
           timer->pre=lst_head;
           timer->next=next_timer;
           next_timer->pre=timer;
           return;
         }
         lst_head=next_timer;
         next_timer=lst_head->next;
       }

       //如果timer比最后一个节点的时间还要长，则将该节点插入到末尾
       //此时的lst_head已经走到tail位置
       tail->next=timer;
       timer->pre=tail;
       tail=timer;
       return;
    }

    //判断timer放在最后面
    void sort_timer_list::adjust_timer(util_timer* timer)
    {
      //如果timer是head或者为空，则不需要改变
      if(!timer){
        return;
      }

      //如果timer等于最后一个节点，或者timer节点超时时间小于下一个节点的超时时间，则不需要改变
      if(timer==tail||timer->next->expire>=timer->expire){
        return; 
      }

      util_timer* next_timer=timer->next;
      util_timer* pre_timer=timer->pre;
      //连接两个节点

      next_timer->pre=pre_timer; 
      if(pre_timer){
          pre_timer->next=next_timer;        
      }else{
          //如果pre_timer为nullptr说明timer为head
          head=next_timer;
      }
      //将timer节点放在next_timer的后面
      add_timer(timer,next_timer);
      return ;
    }

    //调试成功
    void sort_timer_list::del_timer(util_timer* timer){
      if(!timer){
        return;
      }

      //只有一个节点
      if(head==tail){
        delete head;
        head=nullptr;
        tail=nullptr;
        timer=nullptr;
        return;
      }

      //如果该节点是头节点
      if(timer==head){
        //删除头节点
        util_timer* next_timer=head->next;
        delete timer;
        timer=nullptr;
        head=next_timer;
        head->pre=nullptr;
        return;
      }


      if(timer==tail){
        util_timer* pre_timer=tail->pre;
        pre_timer->next=nullptr;
        delete timer;
        tail=pre_timer;
        tail->next=nullptr;
        return;
      }
      util_timer* next_timer=timer->next;
      util_timer* pre_timer=timer->pre;
      next_timer->pre=pre_timer;
      pre_timer->next=next_timer;
      delete timer;
      timer=nullptr;
      return;
    }
    
    //SIGALARM信号每次被触发就在信号处理函数中执行以此tick函数
    //处理链表上到期的任务
    void sort_timer_list::tick(){
      //获取当前系统时间
      int cur_time=time(NULL);
      //遍历每一个节点的时间
      //如果当前时间小于节点的超时时间则跳出
      //如果大于当前时间，则调用处理函数，将删除该节点
      util_timer* cur_timer=head;
      while(cur_timer)
      {
         if(cur_timer->expire<=cur_time){
          //当前时间大于节点的超时时间,则说明该节点超时
          //清除用户数据
          util_timer* next_timer=cur_timer->next;
          cur_timer->cb_func(cur_timer->user_data);
          //删除对应的节点
          head=next_timer;;
          cur_timer=head; 
         }else{
          break;
         }
      }
      return;
    }

