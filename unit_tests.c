//"editor.tokenColorCustomizations": {
//    "comments": "#1e1e1e" // невидимость для комментариев
//}
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>     // Для fork()
#include <sys/types.h>  // Для типа данных pid_t
#include <sys/wait.h>   // Для wait() и всех макросов анализа (WIFEXITED, WEXITSTATUS и др.)
#include "stack.c"

#define EXIT_CHECKS_FAILED 6767 //если упал CHECK - будущий код завершения ребёнка

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

static void check_file_ptr(FILE* file_errors)
{
    if (file_errors == NULL)
    { 
        FILE* tests_errors = fopen("tests_errors.txt", "a");
        if (tests_errors)
        {
            fprintf(tests_errors, "FAIL: file_errors does not open\n");
            fclose(tests_errors);
        }
        //printf("БЕГИТЕ Я КОНЧЕНЫЙ\n");
        abort();
    }
    //printf("I NORMAL\n");
}

static void check_log_buffer(FILE* file_errors, const char* expected_log, my_stack_t* stk)
{
    char buffer[200] = {0};
    fgets(buffer, 200, file_errors);
    CHECK(!strcmp(buffer, expected_log), stk);
}

static void t_ctor_and_dtor_base_check(void)
{
    my_stack_t cd_base_stk = {};
    uint64_t size = 5;
    CHECK(stack_ctor(&cd_base_stk, size STACK_PLACE_OUT("cd_base_stk")) == STACK_OK,        &cd_base_stk);
    CHECK(cd_base_stk.capacity == 5 && cd_base_stk.size == 0 && cd_base_stk.data != 0,      &cd_base_stk);
    CHECK(stack_dtor(&cd_base_stk) == STACK_OK,                                             &cd_base_stk);
    CHECK(cd_base_stk.data == NULL && cd_base_stk.capacity == 0 && cd_base_stk.size == 0,   &cd_base_stk);
}

static void t_pushpoptop_check(void)
{
    my_stack_t pushpoptop_base_stk = {};
    uint64_t size = 5;
    CHECK(stack_ctor(&pushpoptop_base_stk, size STACK_PLACE_OUT("pushpoptop_base_stk")) == STACK_OK,        &pushpoptop_base_stk);
    for (uint64_t i = 0; i < size; i++)
    {
        Elem_t check_value;
        CHECK(stack_push(&pushpoptop_base_stk, i * 10 + 2)  == STACK_OK,    &pushpoptop_base_stk);
        CHECK(my_stack_top(&pushpoptop_base_stk, &check_value) == STACK_OK,    &pushpoptop_base_stk);
        CHECK(check_value                                   == i * 10 + 2,  &pushpoptop_base_stk);
        CHECK(pushpoptop_base_stk.size                      == i + 1,       &pushpoptop_base_stk);
    }
    CHECK(stack_dtor(&pushpoptop_base_stk)                  == STACK_OK,    &pushpoptop_base_stk);
}

static void t_null_stk_ctor(void)
{
    CHECK(stack_ctor(NULL, 5 STACK_PLACE_OUT("null_stk")) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 1\n", NULL);
    fclose(file_errors);
}

static void t_null_stk_dtor(void)
{
    CHECK(stack_dtor(NULL) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 1\n", NULL);
    fclose(file_errors);
}

static void t_null_stk_push(void)
{
    CHECK(stack_push(NULL, 52) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 1\n", NULL);
    fclose(file_errors);
}

static void t_null_stk_pop(void)
{
    Elem_t val = 0;
    CHECK(stack_pop(NULL, &val) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 1\n", NULL);
    fclose(file_errors);
}

static void t_null_stk_top(void)
{
    Elem_t val = 0;
    CHECK(my_stack_top(NULL, &val) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 1\n", NULL);
    fclose(file_errors);
}

static void t_pop_from_empty_stk(void)
{
    my_stack_t stk_underflow = {};
    CHECK(stack_ctor(&stk_underflow, 4 STACK_PLACE_OUT("stk_underflow")) == STACK_OK, &stk_underflow);

    Elem_t val = 0;
    CHECK(stack_pop(&stk_underflow, &val) == STACK_UNDERFLOW, &stk_underflow);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 3\n", NULL);
    fclose(file_errors);

    CHECK(stack_dtor(&stk_underflow) == STACK_OK, &stk_underflow);
}

/*static void t_push_to_stack_overflow(void)
{
    my_stack_t stk_overflow = {};
    CHECK(stack_ctor(&stk_overflow, 4 STACK_PLACE_OUT("stk_overflow")) == STACK_OK, &stk_overflow);

    while(sizeof(stk_overflow) < MAX_MEMORY_ON_STACK - sizeof(Elem_t))
    {
        CHECK(stack_push(&stk_overflow, 432) == STACK_OK, &stk_overflow);
    }
    CHECK(stack_push(&stk_overflow, 432) == STACK_OVERFLOW, &stk_overflow);

    FILE* file_errors = fopen("errors.txt", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 4\n", NULL);
    fclose(file_errors);

    CHECK(stack_dtor(&stk_overflow) == STACK_OK, &stk_overflow);
}*/

static int isolated_check(my_stack_t stk)
{

}

static void t_left_stk_canary_corrypted(void)
{
    my_stack_t stk_left_stk_canary = {};
    CHECK(stack_ctor(&stk_left_stk_canary, 4, STACK_PLACE_OUT("stk_left_stk_canary")) == STACK_OK, &stk_left_stk_canary);

    stk_left_stk_canary.left_canary ^= (canaty_t)1 << 52;
    CHECK(stack_verify(&stk_left_stk_canary) == LEFT_STACK_CANARY_STK_DEAD, &stk_left_stk_canary);
    isolated_check();
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
// Тесты на отлов ошибок WARNING
    TEST(null_stk_ctor,            SURVIVE, "WARNING: 1"),
    TEST(null_stk_dtor,            SURVIVE, "WARNING: 1"),
    TEST(null_stk_push,            SURVIVE, "WARNING: 1"),
    TEST(null_stk_pop,             SURVIVE, "WARNING: 1"),
    TEST(null_stk_top,             SURVIVE, "WARNING: 1"),
    TEST(pop_from_empty_stk,       SURVIVE, "WARNING: 3"),
    //TEST(push_to_stack_overflow,   SURVIVE, "WARNING: 4")
// Тесты на отлов ошибок CRASH
};

#define NUMBER_OF_TESTS (sizeof(TESTS) / sizeof(TESTS[0]))

static const test_t* find_test(const char* name)
{
    for (uint64_t i = 0; i < NUMBER_OF_TESTS; i++) if (strcmp(TESTS[i].name, name) == 0) return &TESTS[i];
    return NULL;
}

static int run_isolated(const test_t* t)
{
    fflush(stdout); //от двойных printf ов
    pid_t pid = fork(); //сохраняем PID созданного процесса
    if (pid == -1) 
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        // Код дочернего процесса
        t->func(); //t->func тупо адрес блин, ставь скобки
        exit(incorrect_checks ? EXIT_CHECKS_FAILED : 0);
    }
    int status = 0;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status))
    { 
        printf("Убит сигналом = %d%s\n", WTERMSIG(status), WCOREDUMP(status) ? " (дамп ядра)" : "");
        return 1;
    }

    return -2; //если приостановлен или продолжен
}

static int run_one(const char* name)
{
    const test_t* t = find_test(name);
    if (t == NULL) { printf("NO THAT TEST: %s\n", name); return 2; }

    remove("errors.txt");
    remove("tests_errors.txt");

    return run_isolated(t);
}

static void list_tests(void)
{
    for (uint64_t i = 0; i < NUMBER_OF_TESTS; i++)
    {
        printf("TEST %" PRIu64 ": %s: %s", i,
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