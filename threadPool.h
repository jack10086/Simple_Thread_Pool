# pragma once
#include "TaskQueue.h"
#include<unistd.h>

class ThreadPool
{
public:
    ThreadPool(int minNum, int maxNum);
    ~ThreadPool();

    // 添加任务
    void addTask(Task &task);
    // 获取忙的线程个数
    int getBusyNum();
    // 获取活着的线程个数
    int getAliveNum();

private:
    // 消费者线程函数（工作线程）
    static void *worker(void *arg);

    // 管理者线程
    static void *manager(void *arg);

    // 单个线程退出
    void threadExit();

private:
    pthread_mutex_t m_lock;  //
    pthread_cond_t notEmpty; // 任务队列是否为空
    TaskQueue *taskQ;        // 任务队列
    pthread_t managerID;     // 管理者线程ID
    pthread_t *threadIDS;    // 工作的线程ID
    int minNum;              // 最小线程数量
    int maxNum;              // 最大线程数量
    int busyNum;             // 忙的线程数
    int aliveNum;            // 存活的线程数
    int exitNum;             // 要销毁的线程数
    bool shutdown = false;   // 是否销毁线程池
};