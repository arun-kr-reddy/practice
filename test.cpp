#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t g_counter = 0;

void *thread1Func(void *ptr)
{
    while (1)
    {
        sleep(0.5);

        // int pthread_mutex_lock(pthread_mutex_t *mutex);
        // int pthread_mutex_unlock(pthread_mutex_t *mutex);
        pthread_mutex_lock(&g_mutex);
        g_counter++;
        printf("1 %d\n", g_counter);
        pthread_mutex_unlock(&g_mutex);

        if (g_counter >= 5)
        {
            // void pthread_exit(void *retval);
            pthread_exit(NULL);
        }
    }
}

void *thread2Func(void *ptr)
{
    while (1)
    {
        sleep(0.5);

        pthread_mutex_lock(&g_mutex);
        g_counter++;
        printf("2 %d\n", g_counter);
        pthread_mutex_unlock(&g_mutex);

        if (g_counter >= 10)
        {
            pthread_exit(NULL);
        }
    }
}

int main()
{
    pthread_t thread1;
    pthread_t thread2;
    int ret1, ret2;

    // int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void
    // *(*start_routine)(void *), void *restrict arg);
    // if (attr == NULL), default params used
    ret1 = pthread_create(&thread1, NULL, thread1Func, NULL);
    assert(ret1 == 0);
    ret2 = pthread_create(&thread2, NULL, thread2Func, NULL);
    assert(ret2 == 0);

    // int pthread_join(pthread_t thread, void **retval);
    // retval should not be local scope
    pthread_join(thread1, NULL);
    printf("thread1 returns: %d\n", ret1);

    pthread_join(thread2, NULL);
    printf("thread2 returns: %d\n", ret2);

    return 0;
}