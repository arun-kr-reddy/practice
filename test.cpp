#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct linked_list
{
    uint8_t value;
    linked_list *next;
} linked_list;

void printLL(linked_list *head)
{
    linked_list *current = head;

    while (current != NULL)
    {
        printf("%d\n", current->value);
        current = current->next;
    }
}

void deleteNode(linked_list *head, uint8_t idx)
{
    printf("idx %u\n", idx);
    head = NULL; // head->next;
    return;

    if (idx == 0)
    {
        printf("inside");
        head = NULL; // head->next;
        return;
    }

    uint8_t i = 0;
    linked_list *previous = NULL;
    linked_list *current = head;
    while (current != NULL)
    {
        previous = current;
        current = current->next;
        ++i;
        if (i == idx)
        {
            previous->next = current->next;
            return;
        }
    }
}

static void swap(uint8_t *a, uint8_t *b)
{
    uint8_t temp = *a;
    *a = *b;
    *b = temp;

    return;
}

void bubbleSort(uint8_t *array, uint8_t n)
{
    for (int i = 0; i < (n - 1); ++i)
    {
        for (int j = 0; j < (n - i - 1); ++j)
        {
            if(array[j] > array[j+1])
            {
                swap(&array[j], &array[j+1]);
            }
        }
    }

    return;
}

int main(void)
{
    clock_t t;
    t = clock();

    linked_list head, middle, tail;

    head.value = 10;
    head.next = &middle;

    middle.value = 20;
    middle.next = &tail;

    tail.value = 30;
    tail.next = NULL;

    deleteNode(&head, 2);
    printf("head %u\n", head.value);
    printLL(&head);

    printf("%d\n", atoi("12"));

    t = clock() - t;

    double time_taken = ((double)t) / CLOCKS_PER_SEC;

    printf("run in %lf sec\n", time_taken);

    uint8_t arr[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    bubbleSort(arr, 10);
    for(int i=0; i<10; i++)
    {
        printf("%u ", arr[i]);
    }
    printf("\n");

    return 0;
}