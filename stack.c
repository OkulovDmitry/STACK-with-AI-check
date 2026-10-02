#include "stack.h"

#define STACK_CHECK(stk)                                                                \
    do {                                                                                \
        err_t _e = stack_verify(stk);                                                   \
        if (_e == STACK_CORRUPTED)                                                      \
        {                                                                               \
            FILE* errors_file = fopen("errors.txt", "a");                               \
            stack_dump("errors.txt", (stk), #stk, __LINE__, __func__, __FILE__);        \
            /*abort без затирания*/                                                     \
        }                                                                               \
        if (_e != STACK_OK) {                                                           \
            FILE* errors_file = fopen("errors.txt", "a");                               \
            fprintf(errors_file, "WARNING: %i\n", _e);                                  \
            stack_dump("errors.txt", (stk), #stk, __LINE__, __func__, __FILE__);        \
            fclose(errors_file);                                                        \
            return _e;                                                                  \
        }                                                                               \
    } while (0)

#define ERROR_LOGGING(_e, stk)                                                          \
    do                                                                                  \
    {                                                                                   \
        FILE* errors_file = fopen("errors.txt", "a");                               \
        fprintf(errors_file, "WARNING: %i\n", _e);                                  \
        stack_dump("errors.txt", (stk), #stk, __LINE__, __func__, __FILE__);        \
        fclose(errors_file);                                                        \
        return _e;                                                                  \
    } while (0)

err_t stack_ctor(stack_t* stk, size_t initial_capacity STACK_PLACE_IN)
{
#ifdef STKDEBUG
    stk->name = name;
    stk->function = function;
    stk->file = file;
    stk->line = line;
#endif
    if (stk == NULL) ERROR_LOGGING(STACK_NULL_PTR, stk);
    if (initial_capacity < 0) ERROR_LOGGING(STACK_CORRUPTED, stk);
    Elem_t* temp;
    if ((temp = calloc(initial_capacity, sizeof(Elem_t))) == NULL) ERROR_LOGGING(STACK_OUT_OF_MEMORY, stk);
    stk->data = temp;
    stk->capacity = initial_capacity;
    stk->size = 0;
    STACK_CHECK(stk);
    return STACK_OK;
}

err_t stack_push(stack_t* stk, Elem_t value)
{
    STACK_CHECK(stk);

    if (stk->size == stk->capacity)
    {
        //printf("I want resize up stack\n");
        err_t err = resize_up(stk);
        if (err) return err;
    }
    stk->data[stk->size++] = value;
    //printf("I push " ELEM_FMT "\n", value);
    STACK_CHECK(stk);
    return STACK_OK;
}

err_t resize_up(stack_t* stk)
{
    //printf("I am in resize_up");
    STACK_CHECK(stk);

    Elem_t* temp = realloc(stk->data, 2 * stk->capacity * sizeof(Elem_t) + sizeof(Elem_t));
    if (temp == NULL) ERROR_LOGGING(STACK_OUT_OF_MEMORY, stk);
    stk->data = temp;
    //printf("I did realloc");
    stk->capacity += stk->capacity + 1;
    memset((Elem_t*)((size_t)stk->data + stk->size*sizeof(Elem_t)), 0, (stk->capacity - stk->size)*sizeof(Elem_t));
    STACK_CHECK(stk);
    return STACK_OK;
}

err_t stack_pop(stack_t* stk, Elem_t* out_value)
{
    STACK_CHECK(stk);
    if (stk->size == 0) ERROR_LOGGING(STACK_UNDERFLOW, stk);
    if (out_value == NULL) return NULL_PTR;
    *out_value = stk->data[stk->size - 1];
    stk->data[stk->size - 1] = 0;
    stk->size--;
    //printf("I pop " ELEM_FMT "\n", *out_value);
    if (stk->size == stk->capacity / 4 && stk->capacity != 0)
    {
        //printf("I want resize down stack");
        err_t err = resize_down(stk);
        if (err) return err;
    }
    STACK_CHECK(stk);
    return STACK_OK;
}

err_t resize_down(stack_t* stk)
{
    STACK_CHECK(stk);
    Elem_t* temp = realloc(stk->data, stk->capacity * sizeof(Elem_t) / 2);
    if (temp == NULL) ERROR_LOGGING(STACK_OUT_OF_MEMORY, stk);
    stk->data = temp;
    stk->capacity /= 2;
    //printf("I resize down\n");
    STACK_CHECK(stk);
    return STACK_OK;
}

err_t stack_top(const stack_t* stk, Elem_t* out_value)
{
    STACK_CHECK(stk);
    if (out_value == NULL) return NULL_PTR;
    if (stk->size == 0) ERROR_LOGGING(STACK_UNDERFLOW, stk);
    if (out_value == NULL) return NULL_PTR;
    *out_value = stk->data[stk->size - 1];
    printf("I top " ELEM_FMT "\n", *out_value);
    STACK_CHECK(stk);
    return STACK_OK;
}

err_t stack_dtor(stack_t* stk)
{
    if (stk == NULL) ERROR_LOGGING(STACK_NULL_PTR, stk);
    free(stk->data);
    stk->data = NULL;
    stk->size = 0;
    stk->capacity = 0;
    return STACK_OK;
}

bool stack_is_empty(const stack_t* stk)
{
    return stk == NULL || stk->size == 0;
}

size_t stack_get_size(const stack_t* stk)
{
    return stk == NULL ? 0 : stk->size;
}

err_t stack_verify(const stack_t* stk)
{
    if (stk == NULL)               return STACK_NULL_PTR;
    if (stk->data == NULL)         return STACK_CORRUPTED;
    if (stk->size > stk->capacity) return STACK_CORRUPTED;
    return STACK_OK;
}

void stack_dump(const char* out, const stack_t* stk, const char* name, int line, const char* function, const char* file_name)
{
    FILE* file = fopen(out, "a");
    if (file == NULL) {
        perror("Error of open file");
        return;
    }

    fprintf(file, "====================================================================================================\n");

#ifdef STKDEBUG
    fprintf(file, "stack_t %s created by %s() at %s: %i\n", stk->name, stk->function, stk->file, stk->line);
#endif
#ifndef STKDEBUG
    fprintf(file, "stack_t %s dumped from %s at %s: %i\n", name, function, file_name, line);
#endif
    fprintf(file, "capacity = %zu\n", stk->capacity);
    fprintf(file, "size = %zu\n", stk->size);
    fprintf(file, "%s [%p]\n", name, stk->data);
    if (stk->data != NULL)
    {
        for (size_t i = 0; i < stk->capacity; i++)
        {
            fprintf(file, "*[%zu] = " ELEM_FMT "\n", i, stk->data[i]);
        }
        fclose(file);
    }

    fprintf(file, "====================================================================================================\n");
}

const char* my_strerror(err_t err)
{
    switch(err)
    {
        case STACK_OK: return "OK";
        case STACK_NULL_PTR: return "null stack pointer pased";
        case STACK_OUT_OF_MEMORY: return "out of memory";
        case STACK_UNDERFLOW: return "stack underflow (stack is empty)";
        case STACK_OVERFLOW: return "stack overflow";
        case STACK_CORRUPTED: return "stack structure is corrupted";
        case NULL_PTR: return "null pointer pased";
        default: return "unknown error";
    }
}

void my_perror(const char* prefix, err_t err)
{
    if (prefix != NULL && *prefix != '\0')
    {
        fprintf(stderr, "%s: %s\n", prefix, my_strerror(err));
    }
    else
    {
        fprintf(stderr, "%s\n", my_strerror(err));
    }
}