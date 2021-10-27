#ifndef LST_TIMER_H
#define LST_TIMER_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>
#include <time.h>



#define BUFFER_SIZE 64

class util_timer;

/*client data struct: socketaddr fd rdbuf timer*/
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

/*timer*/
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL){}

public:
    time_t expire; /*timeout*/
    void (*cb_func) (client_data*);
    client_data* user_data;
    util_timer* prev;
    util_timer* next;
};

/*timer list*/
class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL), tail(NULL) {}

    ~sort_timer_lst()
    {
        util_timer* tmp = head;
        while(tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    void add_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        if(!head)
        {
            head = tail = timer;
            return;
        }

        if(timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }

    void adjust_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        util_timer* tmp = timer->next;

        if(!tmp||(timer->expire < tmp->expire))
        {
            return;

        }

        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }

    void del_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }

        if((timer==head)&&(timer==tail))
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }

        if(timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }

        if(timer == tail)
        {
            tail = timer->prev;
            tail->next = NULL;
            delete timer;
            return;
        }

        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    void tick()
    {
        if(!head)
        {
            return;
        }
        //printf("time tick\n");
        time_t cur = time(NULL);
        util_timer* tmp = head;
        while (tmp)
        {
            if(cur < tmp->expire)
            {
                break;
            }
            tmp->cb_func(tmp->user_data);

            head = tmp->next;
            if(head)
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
        
    }

private:
    void add_timer(util_timer* timer, util_timer* lst_head)
    {
        util_timer* prev = lst_head;
        util_timer* tmp = prev->next;

        while(tmp)
        {
            if(timer->expire < tmp->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        if(!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }
    }

private:
    util_timer* head;
    util_timer* tail;
};



#endif