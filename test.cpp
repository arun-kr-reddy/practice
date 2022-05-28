#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
uint8_t g_arr[10] = {0};
bool g_arr_filled = false;

void *producer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < 10; i++)
        {
            g_arr[i] = i * 10;
        }
        g_arr_filled = true;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (g_arr_filled == false)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        for (int i = 0; i < 10; i++)
        {
            printf("%d\n", g_arr[i]);
        }
        g_arr_filled = false;
        sleep(1);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    pthread_t th[2];

    if (pthread_create(&th[0], NULL, &producer, NULL) != 0)
    {
        perror("Failed to create thread");
    }
    if (pthread_create(&th[1], NULL, &consumer, NULL) != 0)
    {
        perror("Failed to create thread");
    }

    for (int i = 0; i < 2; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }

    return 0;
}