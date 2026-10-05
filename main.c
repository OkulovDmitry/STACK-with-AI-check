//"editor.tokenColorCustomizations": {
//    "comments": "#1e1e1e" // невидимость для комментариев
//}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stack.h"

int main()
{
    stack_t stk1 = {};
    int err = stack_ctor(&stk1, 5 STACK_PLACE_OUT("stk1"));
    if (err) my_perror("Error in stack_ctor of stk1:", err);
    
    for (int i = 0; i < 6; i++)
    {
        err = stack_push(&stk1, 10*i+1);
        if (err) my_perror("Error in stack_push", err);
    }
    stack_dump("stack_info.txt", &stk1, "stk1", 25, "main", "main.c");

    for (int i = 0; i < 7; i++)
    {
        double x;
        err = stack_pop(&stk1, &x);
        //printf("size of stack: %zu\n", stk1.capacity);
        if (err) my_perror("Error in stack_pop", err);
        //printf("I pop: %f\n", x);
    }
    stack_dump("stack_info.txt", &stk1, "stk1", 35, "main", "main.c");

    for (int i = 0; i < 23; i++)
    {
        err = stack_push(&stk1, 10*i+1);
        if (err) my_perror("Error in stack_push", err);
        stack_dump("stack_info.txt", &stk1, "stk1", 41, "main", "main.c");
    }

    return 0;
}