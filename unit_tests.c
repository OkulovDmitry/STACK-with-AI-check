//"editor.tokenColorCustomizations": {
//    "comments": "#1e1e1e" // невидимость для комментариев
//}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stack.c"

#define EXIT_CHECKS_FAILED 67 //если упал CHECK

static uint64_t number_of_checks = 0, incorrect_checks = 0;

#define CHECK(cond, stk)                                                         \
    do                                                                           \
    {                                                                            \
        number_of_checks++;                                                      \
        if(!(cond))                                                              \
        {                                                                        \
            FILE* tests_errors = fopen("tests_errors.txt", "a");                 \
            if (tests_errors == NULL)                                            \
            {                                                                    \
                FILE* file_errors = fopen("errors.txt", "a");                    \
                fprintf(file_errors, "FAIL: test_errors does not open\n");       \
                fclose(file_errors);                                             \
                abort();                                                         \
            }                                                                    \
            incorrect_checks++;                                                  \
            fprintf(tests_errors, "FAIL\n");                                     \
            dump_to(tests_errors, (stk), NULL, __LINE__, __func__, __FILE__);    \
            fclose(tests_errors);                                                \
        }                                                                        \
    } while (0)

static void t_ctor_and_dtor_base_check(void)
{
    stack_t cd_base_stk = {};
    uint64_t size = 5;
    CHECK(stack_ctor(&cd_base_stk, size STACK_PLACE_OUT("cd_base_stk")) == STACK_OK,        &cd_base_stk);
    CHECK(cd_base_stk.capacity == 5 && cd_base_stk.size == 0 && cd_base_stk.data != 0,      &cd_base_stk);
    CHECK(stack_dtor(&cd_base_stk) == STACK_OK,                                             &cd_base_stk);
    CHECK(cd_base_stk.data == NULL && cd_base_stk.capacity == 0 && cd_base_stk.size == 0,   &cd_base_stk);
}

static void t_pushpoptop_check(void)
{
    stack_t pushpoptop_base_stk = {};
    uint64_t size = 5;
    CHECK(stack_ctor(&pushpoptop_base_stk, size STACK_PLACE_OUT("pushpoptop_base_stk")) == STACK_OK,        &pushpoptop_base_stk);
    for (int i = 0; i < size; i++)
    {
        Elem_t check_value;
        CHECK(stack_push(&pushpoptop_base_stk, i * 10 + 2)  == STACK_OK,    &pushpoptop_base_stk);
        CHECK(stack_top(&pushpoptop_base_stk, &check_value) == STACK_OK,    &pushpoptop_base_stk);
        CHECK(check_value                                   == i * 10 + 2,  &pushpoptop_base_stk);
        CHECK(pushpoptop_base_stk.size                      == i + 1,       &pushpoptop_base_stk);
    }
    CHECK(stack_dtor(&pushpoptop_base_stk)                  == STACK_OK,    &pushpoptop_base_stk);
}

static void t_null_stk_ctor(void)
{
    CHECK(stack_ctor(NULL, 5 STACK_PLACE_OUT("null_stk")) == STACK_NULL_PTR, NULL);
    FILE* file_errors = fopen("errors.txt", "r");
    if (file_errors == NULL)
    { 
        FILE* tests_errors = fopen("tests_errors.txt", "a");
        fprintf(tests_errors, "FAIL: file_errors does not open\n");
        fclose(tests_errors);
        abort();
    }
    char* check_buffer;
    fgets(check_buffer, 200, file_errors);
    CHECK(!strcmp(check_buffer, "WARNING: 1\n"), NULL);
    fclose(file_errors);
}

typedef enum 
{
    SURVIVE, 
    CRASH
} expect_t;
#ifdef STK_ABORT_ON_CORRUPT
    #define IF_ERR_ABORT CRASH
#else
    #define IF_ERR_ABORT SURVIVE
#endif

typedef struct
{
    const char* name;
    void(*func)(void);
    expect_t expect;
    const char* log; //то что должно записаться в errors.txt
} test_t;

#define TEST(n, exp, log) {#n, t_##n, exp, log}

static const test_t TESTS[] = 
{
// Тесты на базовое исполнение функций
    TEST(ctor_and_dtor_base_check, SURVIVE, NULL),
    TEST(pushpoptop_check,         SURVIVE, NULL),
    TEST(null_stk_ctor,            SURVIVE, "WARNING: 1\n")
// Тесты на отлов ошибок

};

#define NUMBER_OF_TESTS (sizeof(TESTS) / sizeof(TESTS[0]))

static const test_t* find_test(const char* name)
{
    for (uint64_t i = 0; i < NUMBER_OF_TESTS; i++) if (strcmp(TESTS[i].name, name) == 0) return &TESTS[i];
    return NULL;
}

static int run_one(const char* name)
{
    const test_t* t = find_test(name);
    if (t == NULL) { printf("NO THAT TEST: %s\n", name); return 2; }
    t->func(); //t->func тупо адрес блин, ставь скобки
    return incorrect_checks ? EXIT_CHECKS_FAILED : 0;
}

static void list_tests(void)
{
    for (uint64_t i = 0; i < NUMBER_OF_TESTS; i++)
    {
        printf("TEST %llu: %s: %s", i,
                                      TESTS[i].name, 
                                      TESTS[i].expect == SURVIVE ? "should survive " : "MAST crash ");
        if (TESTS[i].log != NULL) printf("with log: %s", TESTS[i].log);
        printf("\n");
    }   
}

int main(int argc, char* argv[])
{
    if (argc >= 3 && !strcmp(argv[1], "--one"))                  return run_one(argv[2]); //эта хрень возвращает не true/false, а 0 если одинаковы а иначе что то другое
    if (argc >= 2 && !strcmp(argv[1], "--list")) { list_tests(); return 0; }

    return 0;
}