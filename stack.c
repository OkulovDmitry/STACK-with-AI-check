#include "stack.h"

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* 1. Конфигурация и макросы                                                  */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define STR(s) ((s) ? (s) : "?")    /* чтобы printf не падал на NULL */

#define ERRORS_FILE        "errors.txt"
#define STACK_MIN_CAPACITY 4
#define DUMP_MAX_ELEMS     1000
#define SEP "====================================================================================================\n"

#ifdef STK_CANARY
    #define CANARY_SIZE sizeof(canary_t)
#else
    #define CANARY_SIZE ((size_t)0)
#endif

//========================== ABORT делать или нет ===================================
#ifdef STK_ABORT_ON_CORRUPT
    #define ABORT_IF_ERROR(_e) do { if (STACK_IS_ERROR(_e)) abort(); } while (0)
#else
    #define ABORT_IF_ERROR(_e) ((void)0)
#endif
//===================================================================================

//======================================== Запись ошибок ============================================
#define ERROR_LOGGING(_e, stk)                                                                      \
    do {                                                                                            \
        FILE* errors_file = fopen(ERRORS_FILE, "a");                                                \
        if (errors_file != NULL)                                                                    \
        {                                                                                           \
            fprintf(errors_file, "%s: %i\n", STACK_IS_ERROR(_e) ? "ERROR" : "WARNING", _e);         \
            dump_to(errors_file, (stk), NULL, __LINE__, __func__, __FILE__);                        \
            fclose(errors_file);                                                                    \
        }                                                                                           \
        ABORT_IF_ERROR(_e);                                                                         \
        return _e;                                                                                  \
    } while (0)
//===================================================================================================

//============================= Проверка стека на валидность ============================
#define STACK_CHECK(stk)                                                                \
    do {                                                                                \
        err_t _e = stack_verify(stk);                                                   \
        if (_e != STACK_OK)                                                             \
        {                                                                               \
            ERROR_LOGGING(_e, stk);                                                     \
        }                                                                               \
    } while (0)                                                                         \
//=======================================================================================

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* 2. Внутренние вспомогательные функции                                      */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

/* Прототипы внутренних функций: видны только в этом файле */
static err_t resize_up(stack_t* stk);
static err_t resize_down(stack_t* stk);

//======= Обнуляет часть буфера от индекса from до конца буфера ========
static void fill_poison(stack_t* stk, size_t from)
{
    for (size_t i = from; i < stk->capacity; i++) stk->data[i] = ELEM_POISON;
}
//======================================================================

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* 3. Защита                                                                  */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

err_t stack_verify(const stack_t* stk)
{
    if (stk == NULL)               return STACK_NULL_PTR;
    if (stk->data == NULL)         return STACK_CORRUPTED;
    if (stk->size > stk->capacity) return STACK_CORRUPTED;
#ifdef STKDEBUG
    for (uint64_t i = stk->size; i < stk->capacity; i++)
        if (stk->data[i] != ELEM_POISON) return STACK_CORRUPTED;
#endif
    return STACK_OK;
}

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* 4. Диагностика                                                             */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

//========================= Печатает подробное состояние стека в произвольный поток ================================
void dump_to(FILE* file, const stack_t* stk, const char* name, int line, const char* function, const char* file_name)
{
    fprintf(file, SEP);

    if (stk == NULL)
    {
        fprintf(file, "stk = NULL\n");
        fprintf(file, SEP);
        return;
    }
#ifdef STKDEBUG
    fprintf(file, "stack_t %s created by %s() at %s: %i\n", STR(stk->name), STR(stk->function), STR(stk->file), stk->line);
#endif
    fprintf(file, "stack_t %s dumped from %s() at %s: %i\n", STR(name), STR(function), STR(file_name), line);
    fprintf(file, "capacity = %llu\n", stk->capacity);
    fprintf(file, "size = %llu\n", stk->size);
    fprintf(file, "%s [%p]\n", STR(name), (void*)stk->data);
    if (stk->data != NULL)
    {
        for (size_t i = 0; i < stk->capacity; i++)
        {
            fprintf(file, "%c[%llu] = " ELEM_FMT "\n", i < stk->size ? '*' : ' ', i, stk->data[i]);
        }
    }
    else fprintf(file, "data = NULL\n");

    fprintf(file, SEP);
}
//====================================================================================================================

//=============================== Обёртка над dump_to - дописывает dump в файл с именем out ================================
void stack_dump(const char* out, const stack_t* stk, const char* name, int line, const char* function, const char* file_name)
{
    FILE* file = fopen(out, "a");
    if (file == NULL) {
        perror("Error of open file");
        return;
    }

    dump_to(file, stk, name, line, function, file_name);
    fclose(file);
}
//============================================================================================================================

//====================================== Превращает код ошибки в понятную строку =============================================
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
        case STACK_NULL_OUT_PTR: return "null pointer pased";
        case STACK_CANARY_STK_DEAD: return "Attack on structure canary";  /* затёрта канарейка структуры */
        case STACK_CANARY_DATA_DEAD: return "Attack on bufer canary"; /* затёрта канарейка буфера данных */
        case STACK_HASH_MISMATCH: return "Hash changed";
        default: return "unknown error";
    }
}
//==============================================================================================================================


//======== Печатает в stderr "prefix: описание ошибки" ========
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
//=============================================================

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* 5. Публичный API                                                           */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */


//======================================== Инициализация стека ============================================
err_t stack_ctor(stack_t* stk, uint64_t initial_capacity STACK_PLACE_IN)
{
    if (stk == NULL) ERROR_LOGGING(STACK_NULL_PTR, stk);
#ifdef STKDEBUG
    stk->name = name;
    stk->function = function;
    stk->file = file;
    stk->line = line;
#endif
    if (initial_capacity < 0) ERROR_LOGGING(STACK_CORRUPTED, stk);
    Elem_t* temp;
    if ((temp = calloc(initial_capacity, sizeof(Elem_t))) == NULL) ERROR_LOGGING(STACK_OUT_OF_MEMORY, stk);
    stk->data = temp;
    stk->capacity = initial_capacity;
    stk->size = 0;
    fill_poison(stk, stk->size);
    STACK_CHECK(stk);
    return STACK_OK;
}
//===========================================================================================================


//============== Push и resize_up - кладут один элемент в стек и увеличивают если он переполнен =============
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

static err_t resize_up(stack_t* stk)
{
    //printf("I am in resize_up");
    STACK_CHECK(stk);
    uint64_t new_cap = 2 * stk->capacity + 1;
    Elem_t* temp = realloc(stk->data, new_cap * sizeof(Elem_t));
    if (temp == NULL) ERROR_LOGGING(STACK_OUT_OF_MEMORY, stk);
    stk->data = temp;
    //printf("I did realloc");
    stk->capacity = new_cap;
    fill_poison(stk, stk->size);
    STACK_CHECK(stk);
    return STACK_OK;
}
//==============================================================================================================

//============== Pop и resize_down - удаляют один элемент из стека и уменьшают если он слишком велик для хранения текущего количества элементов =============
err_t stack_pop(stack_t* stk, Elem_t* out_value)
{
    STACK_CHECK(stk);
    if (stk->size == 0) ERROR_LOGGING(STACK_UNDERFLOW, stk);
    if (out_value == NULL) return STACK_NULL_OUT_PTR;
    *out_value = stk->data[stk->size - 1];
    stk->data[stk->size - 1] = ELEM_POISON;
    stk->size--;
    //printf("I pop " ELEM_FMT "\n", *out_value);
    if (stk->size <= stk->capacity / 4 && stk->capacity > STACK_MIN_CAPACITY)
    {
        //printf("I want resize down stack");
        err_t err = resize_down(stk);
        if (err) return err;
    }
    STACK_CHECK(stk);
    return STACK_OK;
}

static err_t resize_down(stack_t* stk)
{
    STACK_CHECK(stk);
    uint64_t new_cap = max(stk->capacity / 2, STACK_MIN_CAPACITY);
    Elem_t* temp = realloc(stk->data, sizeof(Elem_t) * new_cap);
    if (temp == NULL) ERROR_LOGGING(STACK_OUT_OF_MEMORY, stk);
    stk->data = temp;
    stk->capacity = new_cap;
    //printf("I resize down\n");
    STACK_CHECK(stk);
    return STACK_OK;
}
//==========================================================================================================================================================

//========== Читает верхний элемент не удаляя его ============
err_t stack_top(const stack_t* stk, Elem_t* out_value)
{
    STACK_CHECK(stk);
    if (out_value == NULL) return STACK_NULL_OUT_PTR;
    if (stk->size == 0) ERROR_LOGGING(STACK_UNDERFLOW, stk);
    *out_value = stk->data[stk->size - 1];
    //printf("I top " ELEM_FMT "\n", *out_value);
    STACK_CHECK(stk);
    return STACK_OK;
}
//============================================================

//===== Освобождает память стека и сбрасывает поля в нули =====
err_t stack_dtor(stack_t* stk)
{
    if (stk == NULL) ERROR_LOGGING(STACK_NULL_PTR, stk);
    free(stk->data);
    stk->data = NULL;
    stk->size = 0;
    stk->capacity = 0;
    return STACK_OK;
}
//=============================================================

/* stack_is_empty - true, если стек пуст; NULL считается пустым стеком */
bool stack_is_empty(const stack_t* stk)
{
    return stk == NULL || stk->size == 0;
}

/* stack_get_size - текущее число элементов; для NULL возвращает 0 */
size_t stack_get_size(const stack_t* stk)
{
    return stk == NULL ? 0 : stk->size;
}

