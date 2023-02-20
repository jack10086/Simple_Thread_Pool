# pragma once
#include <mutex>
#include <queue>
using namespace std;

using callback = void (*)(void *);
class Task
{
public:
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void *arg)
    {
        function = f;
        this->arg = arg;
    }
    callback function;
    void *arg;
};

// 任务队列
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    // 添加任务
    void addTask(Task &task);
    void addTask(callback func, void *arg);

    // 取出一个任务
    Task takeTask();

    // 获取当前队列中的任务个数
    inline size_t taskNumber()
    {
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex; // 互斥锁
    queue<Task> m_queue;     // 任务队列
};
