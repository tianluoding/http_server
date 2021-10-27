#ifndef _UTILS_H_
#define _UTILS_H_
#include "timer_wheel.h"
//#include "timer_wheel.h"

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    int setnonblocking(int fd);

    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    static void sig_handler(int sig);

    void addsig(int sig, void(handler)(int), bool restart = true);

    void timer_handler();

    void show_error(int connfd, const char*info);

public:
    static int *u_pipefd;
    //sort_timer_lst m_timer_lst;
    time_wheel m_time_wheel;
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data*user_data);
#endif