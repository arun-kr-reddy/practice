#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
uint8_t g_counter = 0;
bool g_count_updated = false;

void *producer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&g_mutex);
        while(g_count_updated == true)
        {
            pthread_cond_wait(&g_cond, &g_mutex);
        }
        ++g_counter;
        printf("%u produced\n", g_counter);
        g_count_updated = true;
        pthread_mutex_unlock(&g_mutex);
        pthread_cond_signal(&g_cond);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&g_mutex);
        while(g_count_updated == false)
        {
            pthread_cond_wait(&g_cond, &g_mutex);
        }
        printf("%u received\n", g_counter);
        g_count_updated = false;
        pthread_mutex_unlock(&g_mutex);
        pthread_cond_signal(&g_cond);
    }
}
enum temp {e_a=10 , e_b, e_c};
int main()
{
    pthread_t thread1, thread2;

    // assert(pthread_create(&thread1, NULL, producer, NULL) == 0);
    // assert(pthread_create(&thread2, NULL, consumer, NULL) == 0);

    // assert(pthread_join(thread1, NULL) == 0);
    // assert(pthread_join(thread2, NULL) == 0);

    int a = 10;
    int b = 20;
    int *c = (int *)malloc(4);
    int *d = (int *)malloc(4);
    printf("%p %p %p %p\n", &a, &b, c, d);
    printf("%d %d %d\n", e_a, e_b, e_c);

    return 0;
}