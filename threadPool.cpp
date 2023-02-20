
#include "threadPool.h"
#include <iostream>
#include <cstring>

ThreadPool::ThreadPool(int minN, int maxN)
{
    // 实例化任务队列
    taskQ = new TaskQueue;

    // 初始化线程池
    minNum = minN;
    maxNum = maxN;
    busyNum = 0;
    aliveNum = minNum;

    // 根据线程池上限给线程数组分配内存
    threadIDS = new pthread_t[maxNum];
    if (threadIDS == nullptr)
    {
        cout << "new pthread_t[] failed----" << endl;
        return;
    }

    // 初始化
    memset(threadIDS, 0, sizeof threadIDS);

    // 初始化互斥锁，条件变量
    if (pthread_mutex_init(&m_lock, NULL) != 0 || pthread_cond_init(&notEmpty, NULL) != 0)
    {
        cout << "init mutex or condition failed----" << endl;
        return;
    }

    // 创建最小线程个数
    for (int i = 0; i < minNum; i++)
    {
        pthread_create(&threadIDS[i], NULL, worker, this);
        cout << "init Sub-threads, ID: " << to_string(threadIDS[i]) << endl;
    }

    // 创建管理者线程，1个
    pthread_create(&managerID, NULL, manager, this);
}
ThreadPool::~ThreadPool()
{
    shutdown = true;

    // 销毁管理者线程
    pthread_join(managerID, NULL);

    // 唤醒所有消费者线程
    for (int i = 0; i < aliveNum; i++)
        pthread_cond_signal(&notEmpty);

    if (taskQ)
        delete (taskQ);
    if (threadIDS)
        delete (threadIDS);
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&notEmpty);
}

// 添加任务
void ThreadPool::addTask(Task &task)
{
    if (shutdown)
        return;

    // 添加任务，不需要加锁，任务队列中有锁
    taskQ->addTask(task);
    // 唤醒工作的线程
    pthread_cond_signal(&notEmpty);
}

// 获取忙的线程个数
int ThreadPool::getBusyNum()
{
    pthread_mutex_lock(&m_lock);
    int threadNum = busyNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

// 获取活着的线程个数
int ThreadPool::getAliveNum()
{
    pthread_mutex_lock(&m_lock);
    int threadNum = aliveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

void *ThreadPool::manager(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);

    // 如果线程池没有关闭，就一直检测
    while (!pool->shutdown)
    {
        // 每5秒检测一次
        sleep(5);

        // 取出线程池中的相关数据
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->taskQ->taskNumber();
        int liveNum = pool->aliveNum;
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        // 新增或销毁的线程数
        const int NUMBER = 2;

        // 当前任务个数>存活的线程数 && 存活的线程数<最大线程个数
        if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            // 给线程池添加线程时，也需要对线程池加锁
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->maxNum, num < NUMBER, pool->aliveNum < pool->maxNum; i++)
                if (pool->threadIDS[i] == 0)
                {
                    pthread_create(&pool->threadIDS[i], NULL, worker, pool);
                    num++;
                    pool->aliveNum++;
                }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // 销毁线程: 忙的线程数<存活的线程数目 && 存活的线程数目>最小线程数
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->m_lock);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            for (int i = 0; i < NUMBER; ++i)
                pthread_cond_signal(&pool->notEmpty);
        }
    }
    return nullptr;
}

void *ThreadPool::worker(void *arg)
{
    // 线程池是一个共享资源
    ThreadPool *pool = static_cast<ThreadPool *>(arg);

    // 一直读
    while (true)
    {
        // 开始操作线程池，访问任务队列，需要加锁
        pthread_mutex_lock(&pool->m_lock);

        // 任务队列为空，且线程池未关闭时，工作线程阻塞
        while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)
        {
            cout << "thread " << to_string(pthread_self()) << " waiting----" << endl;

            // 阻塞线程
            pthread_cond_wait(&pool->notEmpty, &pool->m_lock);

            // 解除阻塞后，判断是否要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->aliveNum > pool->minNum)
                {
                    pool->aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }

        // 如果线程池被关闭了，线程也要退出
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // 开始处理任务
        // 从队列中取出一个任务
        Task task = pool->taskQ->takeTask();

        // 工作线程+1
        pool->busyNum++;

        // 对线程池的操作结束了，线程池解锁
        pthread_mutex_unlock(&pool->m_lock);

        // 执行任务
        cout << "thread " << to_string(pthread_self()) << " start working----" << endl;
        task.function(task.arg);

        // 任务执行结束
        cout << "thread " << to_string(pthread_self()) << " over working----" << endl;
        pthread_mutex_lock(&pool->m_lock);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }
    return nullptr;
}

// 单个线程退出
void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < maxNum; i++)
        if (threadIDS[i] == tid)
        {
            cout << "threadExit() function: thread " << to_string(pthread_self()) << " exiting----" << endl;
            threadIDS[i] = 0;
            break;
        }

    pthread_exit(NULL);
}
