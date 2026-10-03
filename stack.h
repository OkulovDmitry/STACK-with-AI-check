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
//#define STK_CANARY
//#define STK_HASH

#ifdef STKDEBUG
    #define STACK_PLACE_OUT(...) , __VA_ARGS__, __func__, __FILE__, __LINE__
    #define STACK_PLACE_IN , const char* name, const char* function, const char* file, int line
#else 
    #define STACK_PLACE_OUT(...)
    #define STACK_PLACE_IN
#endif

typedef enum
{
    STACK_OK = 0,
//WARNING: 1...99 - стек цел, живем дальше
    STACK_NULL_PTR = 1,
    STACK_OUT_OF_MEMORY = 2, //malloc or realloc fail
    STACK_UNDERFLOW = 3,
    STACK_OVERFLOW = 4,
    STACK_NULL_OUT_PTR = 5,
//ERROR: 100+ - стек испорчен и работать с ним дальше нельзя
    STACK_ERROR_BASE = 100,
    STACK_CORRUPTED = 101, //НАРУШЕН КЭШ ИЛИ КАНАРЕЙКИ
    STACK_CANARY_STK_DEAD   = 102,  /* затёрта канарейка структуры */
    STACK_CANARY_DATA_DEAD  = 103,  /* затёрта канарейка буфера данных */
    STACK_HASH_MISMATCH     = 104   /* хеш структуры или данных не сошёлся */
} err_t;

#define STACK_IS_WARNING(_e) ((_e) != STACK_OK && (_e) < STACK_ERROR_BASE)
#define STACK_IS_ERROR(_e) ((_e) >= STACK_ERROR_BASE)

typedef double Elem_t;
#define ELEM_FMT "%f" //format string
#define ELEM_POISON ((Elem_t)0xDEADBEEF)

typedef uint64_t canary_t;
#define CANARY_VALUE ((canary_t)0xBADC0FFEE0DDF00DULL)

typedef struct 
{
#ifdef STK_CANARY
    canary_t left_canary;
#endif
#ifdef STKDEBUG
    const char* name;
    const char* function;
    const char* file;
    int line;
#endif
    Elem_t* data;
    uint64_t capacity;
    uint64_t size;
#ifdef STK_HASH
    uint64_t hash_data;
    uint64_t hash_stk;
#endif
#ifdef STK_CANARY
    canary_t right_canary;
#endif
} stack_t;

err_t stack_ctor(stack_t* stk, uint64_t initial_capacity STACK_PLACE_IN);
err_t stack_dtor(stack_t* stk);

err_t stack_push(stack_t* stk, Elem_t value);
err_t stack_pop(stack_t* stk, Elem_t* out_value);
err_t stack_top(const stack_t* stk, Elem_t* out_value);

bool stack_is_empty(const stack_t* stk);
size_t stack_get_size(const stack_t* stk); //подумать насчет как ловить в них ошибки и выводить

void stack_dump(const char* out, const stack_t* stk, const char* name, int line, const char* functionm, const char* file_name);
err_t stack_verify(const stack_t* stk);

const char* my_strerror(err_t err);
void my_perror(const char* prefix, err_t err);

#define STACK_CTOR(stk, cap)   stack_ctor(&(stk), (cap) STACK_PLACE_OUT(#stk))
#define STACK_DUMP(file, stk)  stack_dump((file), &(stk), #stk, __LINE__, __func__, __FILE__)
//додумать эти обертки

#endif