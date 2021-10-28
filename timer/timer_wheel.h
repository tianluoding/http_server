#ifndef TIME_WHEEL_TIMER
#define TIME_WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include "../lock/locker.h"

#define BUFFER_SIZE 64

class tw_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer* timer;
};

class tw_timer
{
public:
    tw_timer(int rot, int ts) : next(NULL), prev(NULL), rotation(rot), time_slot(ts) {}
public:
    int rotation;
    int time_slot;
    void (*cb_func) (client_data*);
    client_data* user_data;
    tw_timer* next;
    tw_timer* prev;
};

class time_wheel
{
public:
    time_wheel() : cur_slot(0)
    {
        lock = new locker();
        for(int i = 0; i < N; ++i)
        {
            slots[i] = NULL;
        }
    }
    ~time_wheel()
    {
        for(int i = 0; i < N; ++i)
        {
            tw_timer* tmp = slots[i];
            while(tmp)
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
        delete lock;
    }

    tw_timer* add_timer(int timeout)
    {
        if(timeout < 0)
        {
            return NULL;
        }
        int ticks = 0;

        if(timeout < SI)
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI + 1;
        }

        int rotation = ticks / N;

        int ts = (cur_slot + (ticks % N)) % N;

        tw_timer* timer = new tw_timer(rotation, ts);

        lock->lock();
        if( !slots[ts])
        {
            //printf("add timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        lock->unlock();
        return timer;
    }

    void del_timer(tw_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        int ts = timer->time_slot;

        lock->lock();
        if(timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if(slots[ts])
            {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else
        {
            if(!timer->prev)
            {
                printf("nullptr!\n");
                printf("%p rotation is %d, ts is %d, cur_slot is %d\n", timer, timer->rotation, timer->time_slot, cur_slot);
                printf("%p slot[ts] rotation is %d, ts is %d, cur_slot is %d\n",slots[ts], slots[ts]->rotation, slots[ts]->time_slot, cur_slot);
            }
            timer->prev->next = timer->next;
            if(timer->next)
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
        lock->unlock();
    }

    void adjust_timer(tw_timer* timer, int timeout)
    {
        if(!timer || timeout < 0)
        {
            return;
        }
        int ts = timer->time_slot;
        lock->lock();
        if(timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if(slots[ts])
            {
                slots[ts]->prev = NULL;
            }
        }
        else
        {
            if(!timer->prev)
            {
                printf("nullptr!\n");
                printf("%p rotation is %d, ts is %d, cur_slot is %d\n", timer, timer->rotation, timer->time_slot, cur_slot);
                printf("%p slot[ts] rotation is %d, ts is %d, cur_slot is %d\n", slots[ts], slots[ts]->rotation, slots[ts]->time_slot, cur_slot);
            }
            
            timer->prev->next = timer->next;
            if(timer->next)
            {
                timer->next->prev = timer->prev;
            }
        }
        int ticks = 0;
        if(timeout < SI)
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI + 1;
           
        }
        int rotation = ticks / N;
        ts = (cur_slot + (ticks % N)) % N;
        timer->rotation = rotation;
        timer->time_slot = ts;

        if(!slots[ts])
        {
            //printf("adjsut timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        lock->unlock();
    }

    void tick()
    {
        tw_timer* tmp = slots[cur_slot];
        printf("current slot is %d\n", cur_slot);
        while(tmp)
        {
            //printf("tick the timer once\n");
            if(tmp->rotation > 0)
            {
                tmp->rotation--;
                tmp = tmp->next;
            }
            else
            {
                tmp->cb_func(tmp->user_data);
                lock->lock();
                if(tmp == slots[cur_slot])
                {
                    //printf("delete header in cur_slot\n");
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if(slots[cur_slot])
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    if(!tmp->prev)
                    {
                        printf("nullptr!\n");
                        printf("%p rotation is %d, ts is %d, cur_slot is %d\n", tmp, tmp->rotation, tmp->time_slot, cur_slot);
                        printf("%p slot[ts] rotation is %d, ts is %d, cur_slot is %d\n", slots[cur_slot], slots[cur_slot]->rotation, slots[cur_slot]->time_slot, cur_slot);
                    }
                    tmp->prev->next = tmp->next;
                    if(tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer* tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
                lock->unlock();
            }
        }
        cur_slot = ++cur_slot % N;
    }

private:
    static const int N = 60;
    static const int SI = 1;

    tw_timer* slots[N];
    int cur_slot;
    locker* lock;
};
#endif