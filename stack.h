#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <io.h> // Для _open, _read, _close
#include <fcntl.h>    // Для флагов открытия (типа _O_RDONLY, _O_BINARY)
#include <sys/stat.h> // для макросов прав доступа

#define STKDEBUG

#ifdef STKDEBUG
#define STACK_PLACE_OUT(...) , __VA_ARGS__, __func__, __FILE__, __LINE__
#endif
#ifdef STKDEBUG
#define STACK_PLACE_IN , const char* name, const char* function, const char* file, int line
#endif
#ifndef STKDEBUG
#define STACK_PLACE_OUT /*NOT_STKDEBUG*/
#endif
#ifndef STKDEBUG
#define STACK_PLACE_IN /*NOT_STKDEBUG*/
#endif

typedef enum
{
    STACK_OK = 0,
    STACK_NULL_PTR = 1,
    STACK_OUT_OF_MEMORY = 2, //malloc or realloc fail
    STACK_UNDERFLOW = 3,
    STACK_OVERFLOW = 4,
    STACK_CORRUPTED = 5, //НАРУШЕН КЭШ ИЛИ КАНАРЕЙКИ
    NULL_PTR = 6
} err_t;

typedef double Elem_t;
#define ELEM_FMT "%f" //format string

typedef struct 
{
#ifdef STKDEBUG
    const char* name;
    const char* function;
    const char* file;
    int line;
#endif
    Elem_t* data;
    size_t capacity;
    size_t size;

} stack_t;

err_t stack_ctor(stack_t* stk, size_t initial_capacity STACK_PLACE_IN);
err_t stack_dtor(stack_t* stk);

err_t stack_push(stack_t* stk, Elem_t value);
err_t stack_pop(stack_t* stk, Elem_t* out_value);
err_t stack_top(const stack_t* stk, Elem_t* out_value);

err_t resize_up(stack_t* stk); //static
err_t resize_down(stack_t* stk); //static

bool stack_is_empty(const stack_t* stk);
size_t stack_get_size(const stack_t* stk); //подумать насчет как ловить в них ошибки и выводить

void stack_dump(const char* out, const stack_t* stk, const char* name, int line, const char* functionm, const char* file_name);
err_t stack_verify(const stack_t* stk);

const char* my_strerror(err_t err);
void my_perror(const char* prefix, err_t err);

#endif