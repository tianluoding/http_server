#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template<typename T>
class threadpool
{
public:
    threadpool(int actor_model, connection_pool*connPool, int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append_p(T* request);/*往请求队列中添加任务*/
    bool append(T* request, int state);

private:
    static void* worker(void* arg); /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    void run();

private:
    int m_thread_number;
    int m_max_requests;
    pthread_t* m_threads; /*描述线程池的数组，其大小为m_thread_number*/
    std::list<T*> m_workqueue; /*请求队列*/
    locker m_queuelocker;
    sem m_queuestat; /*是否有任务需要处理*/
    bool m_stop;
    connection_pool *m_connPool;
    int m_actor_model;

};

template<typename T>
threadpool<T>::threadpool(int actor_model, connection_pool*connPool, int thread_number, int max_requests) : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL), m_connPool(connPool)
{
    if((thread_number <= 0) || (max_requests <= 0))
    {
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_number];
    if(!m_threads)
    {
        throw std::exception();
    }
    for(int i = 0; i < thread_number; i++)
    {
        printf("create the %dth thread\n", i);
        if(pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete []m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i]))
        {
            delete []m_threads;
            throw std::exception();

        }
    }
}

template<typename T>
threadpool<T>::~threadpool()
{
    delete []m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append_p(T* request)
{
    //printf("append()!\n");
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();

    //printf("append() end!\n");

    return true;
}

template<typename T>
bool threadpool<T>::append(T* request, int state)
{
    //printf("append()!\n");
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();

    //printf("append() end!\n");

    return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool*pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run()
{
    //printf("run()!\n");
    while(!m_stop)
    {
        
        m_queuestat.wait();
        //printf("test1 test1\n");
        m_queuelocker.lock();
       
        //printf("empty()\n");
        if(m_workqueue.empty())
        {
            
            m_queuelocker.unlock();
            continue;
        }
        //printf("test test test\n");
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        //printf("get http_requst!\n");

        if(!request)
        {
            continue;
        }

        //printf("start process!\n");
        //request->mysql = m_connPool->GetConnection();

        //request->process();

        //m_connPool->ReleaseConnection(request->mysql);
        if(m_actor_model == 1) //reactor
        {
            if(request->m_state == 0)
            {
                if(request->read())
                {
                    request->improv = 1;
                    connectionRAII mysqlconn(&request->mysql, m_connPool);
                    request->process();
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else 
            {
                if(request->write())
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else//proactor
        {
            connectionRAII mysqlconn(&request->mysql, m_connPool);
            request->process();
        }
    }
}
#endif