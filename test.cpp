#include "threadPool.h"
#include <iostream>
using namespace std;

void taskFunc(void *arg)
{
    int num = *(int *)arg;
    printf("thread %ld is workig, number = %d\n", pthread_self(), num);
    sleep(1);
}
int main()
{
    ThreadPool pool(3, 10);
    for (int i = 0; i < 100; i++)
    {
        int *num = new int(i + 100);
        Task task(taskFunc, num);
        pool.addTask(task);
    }
    sleep(20);

    return 0;
}