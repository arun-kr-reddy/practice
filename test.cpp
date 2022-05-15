#include <assert.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

void *thread1Func(void *ptr)
{ //...
}

void *thread2Func(void *ptr)
{ //...
}

int main()
{
    pthread_t thread1;
    pthread_t thread2;
    int ret1, ret2;

    ret1 = pthread_create(&thread1, NULL, thread1Func, NULL);
    assert(ret1 == 0);
    ret2 = pthread_create(&thread2, NULL, thread2Func, NULL);
    assert(ret2 == 0);

    pthread_join(thread1, NULL);
    printf("thread1 returns: %d\n", ret1);

    pthread_join(thread2, NULL);
    printf("thread2 returns: %d\n", ret2);

    return 0;
}