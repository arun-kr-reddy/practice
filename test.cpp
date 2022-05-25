#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const uint8_t MAX_SIZE = 10;

typedef struct node
{
    uint8_t data;
    node *next;
} node;

void printLL(node *head)
{
    node *current = head;

    while (current != NULL)
    {
        printf("%d\n", current->data);
        current = current->next;
    }
}

void deleteNode(node *head, uint8_t idx)
{
    printf("idx %u\n", idx);
    head = NULL; // head->next;
    return;

    if (idx == 0)
    {
        printf("inside");
        head = head->next;
        return;
    }

    uint8_t i = 0;
    node *previous = NULL;
    node *current = head;
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

void changePtr(int *num1, int *num2)
{
    num1 = num2;
    return;
}

int main(void)
{
    node *head = (node *)malloc(sizeof(node));
    node *one = (node *)malloc(sizeof(node));
    node *two = (node *)malloc(sizeof(node));
    node *three = (node *)malloc(sizeof(node));

    one->data = 10;
    one->next = two;

    two->data = 20;
    two->next = three;

    three->data = 30;
    three->next = NULL;

    head = one;

    printLL(head);
    deleteNode(head, 0);
    printf("head %u\n", head->data);

    int temp1 = 10, temp2 = 20;
    changePtr(&temp1, &temp2);
    printf("%d\n", temp1, temp2);

    return 0;
}