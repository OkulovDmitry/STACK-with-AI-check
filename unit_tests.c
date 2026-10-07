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

#define EXIT_CHECKS_FAILED 67 //если упал CHECK - будущий код завершения ребёнка, кароче если incorrect_checks > 0

//количество тестов в функции теста и количество тестов, которые нашли ошибку в стеке
static uint64_t number_of_checks = 0, 
                incorrect_checks = 0;
//===================================================================================

//================================ Макрос проверки ==============================
// принимает cond - проверяемое условие, stk - стек, для которого это условие записано, можно передавать NULL, если проверяется не стек
#define CHECK(cond, stk)                                                         \
    do                                                                           \
    {                                                                            \
        number_of_checks++;                                                      \
        if(!(cond))                                                              \
        {                                                                        \
            FILE* tests_errors = fopen("tests_errors.txt", "a");                 \
            if (  tests_errors == NULL)                                          \
            {                                                                    \
                FILE*   file_errors = fopen("errors.log", "a");                  \
                fprintf(file_errors, "FAIL: test_errors does not open\n");       \
                fclose( file_errors);                                            \
                abort();                                                         \
            }                                                                    \
            incorrect_checks++;                                                  \
            fprintf(tests_errors, "FAIL: Test find error in this stack:\n");     \
            dump_to(tests_errors, (stk), NULL, __LINE__, __func__, __FILE__);    \
            fclose( tests_errors);                                               \
        }                                                                        \
    } while (0)
//================================================================================

//===================== Функция проверки открылся ли файл =============
static void check_file_ptr(FILE* file_errors)
{
    if (file_errors == NULL)
    { 
        FILE* tests_errors = fopen("tests_errors.txt", "a");
        if (tests_errors)
        {
            fprintf(tests_errors, "FAIL: file_errors does not open\n");
            fclose( tests_errors);
        }
        abort();
    }
}
//=====================================================================

//============= Функция проверки правильно ли записался лог ошибки в тесте ===============
static void check_log_buffer(FILE* file_errors, const char* expected_log, my_stack_t* stk)
{
    char buffer[200] = {0};
    fgets(buffer, 200, file_errors);
    CHECK(!strcmp(buffer, expected_log), stk);
}
//=========================================================================================

//====================== Функция проверки на базовую работу ctor и dtor стека ============================
static void t_ctor_and_dtor_base_check(void)
{
    my_stack_t cd_base_stk = {};
    uint64_t size = 5;
    CHECK(stack_ctor(&cd_base_stk, size STACK_PLACE_OUT("cd_base_stk")) == STACK_OK,        &cd_base_stk);

    CHECK(cd_base_stk.capacity == 5 && 
          cd_base_stk.size     == 0 && 
          cd_base_stk.data     != 0,                                                        &cd_base_stk);

    CHECK(stack_dtor(&cd_base_stk) == STACK_OK,                                             &cd_base_stk);

    CHECK(cd_base_stk.data     == NULL && 
          cd_base_stk.capacity == 0    && 
          cd_base_stk.size     == 0,                                                        &cd_base_stk);
}
//=========================================================================================================

//================================ Функция проверки на базовую работу push и pop т top стека =====================================
static void t_pushpoptop_check(void)
{
    my_stack_t pushpoptop_base_stk = {};
    uint64_t   size                = 5;

    CHECK(stack_ctor(&pushpoptop_base_stk, size STACK_PLACE_OUT("pushpoptop_base_stk")) == STACK_OK,        &pushpoptop_base_stk);

    for (uint64_t i = 0; i < size; i++)
    {
        Elem_t check_value;
        CHECK(stack_push(&pushpoptop_base_stk, i * 10 + 2)  == STACK_OK,    &pushpoptop_base_stk);
        CHECK(stack_top(&pushpoptop_base_stk, &check_value) == STACK_OK,    &pushpoptop_base_stk);
        CHECK(check_value                                   == i * 10 + 2,  &pushpoptop_base_stk);
        CHECK(pushpoptop_base_stk.size                      == i + 1,       &pushpoptop_base_stk);
    }
    CHECK(stack_dtor(&pushpoptop_base_stk)                  == STACK_OK,    &pushpoptop_base_stk);
}
//================================================================================================================================


//========================== Функция на подачу NULL в ctor ============================
static void t_null_stk_ctor(void)
{
    CHECK(stack_ctor(NULL, 5 STACK_PLACE_OUT("null_stk")) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.log", "r");
    check_file_ptr(file_errors);

    check_log_buffer(file_errors, "WARNING: 1\n", NULL);

    fclose(file_errors);
}
//=================================================================================


//========================== Функция на подачу NULL в dtor =========================
static void t_null_stk_dtor(void)
{
    CHECK(stack_dtor(NULL) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.log", "r");
    check_file_ptr(file_errors);

    check_log_buffer(file_errors, "WARNING: 1\n", NULL);

    fclose(file_errors);
}
//==================================================================================

//============= Функция на подачу NULL в push ================
static void t_null_stk_push(void)
{
    CHECK(stack_push(NULL, 52) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.log", "r");
    check_file_ptr(file_errors);
    check_log_buffer(file_errors, "WARNING: 1\n", NULL);
    fclose(file_errors);
}
//============================================================

//================= Функция на подачу NULL в pop ==================
static void t_null_stk_pop(void)
{
    Elem_t val = 0;
    CHECK(stack_pop(NULL, &val) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.log", "r");
    check_file_ptr(file_errors);

    check_log_buffer(file_errors, "WARNING: 1\n", NULL);

    fclose(file_errors);
}
//=================================================================

//============ Функция на подачу NULL в top ================
static void t_null_stk_top(void)
{
    Elem_t val = 0;
    CHECK(stack_top(NULL, &val) == STACK_NULL_PTR, NULL);

    FILE* file_errors = fopen("errors.log", "r");
    check_file_ptr(file_errors);

    check_log_buffer(file_errors, "WARNING: 1\n", NULL);

    fclose(file_errors);
}
//==========================================================

//====================================== POP из пустого стека ========================================
static void t_pop_from_empty_stk(void)
{
    my_stack_t stk_underflow = {};
    CHECK(stack_ctor(&stk_underflow, 4 STACK_PLACE_OUT("stk_underflow")) == STACK_OK, &stk_underflow);

    Elem_t val = 0;
    CHECK(stack_pop(&stk_underflow, &val) == STACK_UNDERFLOW, &stk_underflow);

    FILE* file_errors = fopen("errors.log", "r");
    check_file_ptr(file_errors);

    check_log_buffer(file_errors, "WARNING: 3\n", NULL);

    fclose(file_errors);

    CHECK(stack_dtor(&stk_underflow) == STACK_OK, &stk_underflow);
}
//====================================================================================================

#ifdef STK_CANARY
//=============================== Функция проверки на изменение левой канарейки стека ===================================
static void t_left_stk_canary_corrypted(void)
{
    my_stack_t stk_left_stk_canary = {};
    CHECK(stack_ctor(&stk_left_stk_canary, 4 STACK_PLACE_OUT("stk_left_stk_canary")) == STACK_OK, &stk_left_stk_canary);

    stk_left_stk_canary.left_canary ^= (canary_t)1 << 52;

    Elem_t val = 0;
    stack_pop(&stk_left_stk_canary, &val);
}
//=========================================================================================================================


//=============================== Функция проверки на изменение правой канарейки стека ===================================
static void t_right_stk_canary_corrypted(void)
{
    my_stack_t stk_right_stk_canary = {};
    CHECK(stack_ctor(&stk_right_stk_canary, 4 STACK_PLACE_OUT("stk_right_stk_canary")) == STACK_OK, &stk_right_stk_canary);

    stk_right_stk_canary.right_canary ^= (canary_t)1 << 52;

    Elem_t val = 0;
    stack_pop(&stk_right_stk_canary, &val);
}
//=========================================================================================================================

//================================= Функция проверки на изменение левой канарейки data ===================================
static void t_left_data_canary_corrypted(void)
{
    my_stack_t stk_left_data_canary = {};
    CHECK(stack_ctor(&stk_left_data_canary, 4 STACK_PLACE_OUT("stk_left_data_canary")) == STACK_OK, &stk_left_data_canary);

    unsigned char* ptr = (unsigned char*)stk_left_data_canary.data - 1;
    *ptr = 3;

    Elem_t val = 0;
    stack_pop(&stk_left_data_canary, &val);
}
//=========================================================================================================================

//================================= Функция проверки на изменение правой канарейки data ===================================
static void t_right_data_canary_corrypted(void)
{
    my_stack_t stk_right_data_canary = {};
    CHECK(stack_ctor(&stk_right_data_canary, 4 STACK_PLACE_OUT("stk_right_data_canary")) == STACK_OK, &stk_right_data_canary);

    canary_t* right_data_canary = right_canary_addr(&stk_right_data_canary);
    *right_data_canary = 676952;

    Elem_t val = 0;
    stack_pop(&stk_right_data_canary, &val);
}
//=========================================================================================================================
#endif

#ifdef STK_HASH
// =================================== Функция проверки на изменение data хэша =====================================================
static void t_data_hash_corrypted(void)
{
    my_stack_t stk_data_hash_corrypted = {};
    CHECK(stack_ctor(&stk_data_hash_corrypted, 15 STACK_PLACE_OUT("stk_data_hash_corrypted")) == STACK_OK, &stk_data_hash_corrypted);

    for (int i = 0; i < 16; i++)
    {
        CHECK(stack_push(&stk_data_hash_corrypted, i * 10 + 3) == STACK_OK, &stk_data_hash_corrypted);
    }

    stk_data_hash_corrypted.data[6] = 233234;

    Elem_t val = 0;
    CHECK(stack_pop(&stk_data_hash_corrypted, &val) == STACK_DATA_HASH_MISMATCH, &stk_data_hash_corrypted);
}
//===================================================================================================================================

// =================================== Функция проверки на изменение стек хэша =====================================================
static void t_stack_hash_corrypted(void)
{
    my_stack_t stk_stack_hash_corrypted = {};
    CHECK(stack_ctor(&stk_stack_hash_corrypted, 15 STACK_PLACE_OUT("stk_stack_hash_corrypted")) == STACK_OK, &stk_stack_hash_corrypted);

    for (int i = 0; i < 16; i++)
    {
        CHECK(stack_push(&stk_stack_hash_corrypted, i * 10 + 3) == STACK_OK, &stk_stack_hash_corrypted);
    }

    stk_stack_hash_corrypted.size = 233234;

    Elem_t val = 0;
    CHECK(stack_pop(&stk_stack_hash_corrypted, &val) == STACK_STK_HASH_MISMATCH, &stk_stack_hash_corrypted);
}
//===================================================================================================================================
#endif

//== Ожидаемый способ выхода из процесса ==
typedef enum 
{
    SURVIVE, 
    CRASH
} expect_t;
//=========================================

//===== Режим определяющий наличие abort =====
#ifdef STK_ABORT_ON_CORRUPT
    #define IF_ERR_ABORT CRASH
#else
    #define IF_ERR_ABORT SURVIVE
#endif
//============================================

//===================== Структура теста =====================
typedef struct
{
    const char* name;  // имя теста
    void(*func)(void); // функция тестирующая стек
    expect_t expect;   // Ожидаемый тип выхода стека
    const char* log;   // то что должно записаться в errors.log
} test_t;
//============================================================

//========== Макрос, создающий тест ===========
#define TEST(n, exp, log) {#n, t_##n, exp, log}
//=============================================

//====================== Массив тестов ========================
static const test_t TESTS[] = 
{
// Тесты на базовое исполнение функций
    TEST(ctor_and_dtor_base_check,     SURVIVE, "\n"),
    TEST(pushpoptop_check,             SURVIVE, "\n"),
// Тесты на отлов ошибок WARNING
    TEST(null_stk_ctor,                SURVIVE, "WARNING: 1\n"),
    TEST(null_stk_dtor,                SURVIVE, "WARNING: 1\n"),
    TEST(null_stk_push,                SURVIVE, "WARNING: 1\n"),
    TEST(null_stk_pop,                 SURVIVE, "WARNING: 1\n"),
    TEST(null_stk_top,                 SURVIVE, "WARNING: 1\n"),
    TEST(pop_from_empty_stk,           SURVIVE, "WARNING: 3\n"),
    //TEST(push_to_stack_overflow,     SURVIVE, "WARNING: 4\n"),
// Тесты на отлов ошибок CRASH
#ifdef STK_CANARY
    TEST(left_stk_canary_corrypted,    CRASH,   "ERROR: 102\n"),
    TEST(right_stk_canary_corrypted,   CRASH,   "ERROR: 103\n"),
    TEST(left_data_canary_corrypted,   CRASH,   "ERROR: 104\n"),
    TEST(right_data_canary_corrypted,  CRASH,   "ERROR: 105\n"),
#endif
#ifdef STK_HASH
    TEST(data_hash_corrypted,          CRASH,   "ERROR: 106\n"),
    TEST(stack_hash_corrypted,         CRASH,   "ERROR: 107\n")
#endif
};
//===============================================================

//=============================== Макрос определяющий количество тестов =================================
#define NUMBER_OF_TESTS (sizeof(TESTS) / sizeof(TESTS[0]))
//=======================================================================================================

// ======================== Функция пробешается по TESTS и ищет тест по названию ==========================
static const test_t* find_test(const char* name)
{
    for (uint64_t i = 0; i < NUMBER_OF_TESTS; i++) if (strcmp(TESTS[i].name, name) == 0) return &TESTS[i];
    return NULL;
}
//=========================================================================================================

//==== Функция создающая дочерний процесс, в котором запускается тест, и анализирующая его выход и запись в error.txt ====
static int run_isolated(const test_t* t)
{
    fflush(stdout);     //от двойных printf ов

    pid_t pid = fork(); //сохраняем PID созданного процесса
    if (pid == -1) 
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        // Код дочернего процесса
        t->func();
        exit(incorrect_checks ? EXIT_CHECKS_FAILED : 0);
    }

    int status = 0;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (t->expect == CRASH)
    {
        if (!(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT))
        {
            printf("%s: ожидали abort, а процесс завершился иначе\n", t->name);
            return 1;
        }
    }
    else
    {
        if (!(WIFEXITED(status) && (WEXITSTATUS(status) == 0 || WEXITSTATUS(status) == 67)))
        {
            printf("%s: ожидали нормальный выход\n", t->name);
            return 1;
        }
    }

    if (t->expect == 1 && t->log != NULL)
    {
        FILE* file_errors = fopen("errors.log", "r");
        check_file_ptr(file_errors);
        check_log_buffer(file_errors, t->log, NULL);
        fclose(file_errors);
    }

    return 0; //если приостановлен или продолжен
}
//=====================================================================================================================================

//======= Функция вызывает поиск теста и еси найден, то зачищает файлики для анализа и вызывает run_isolated =======
static int run_one(const char* name)
{
    const test_t* t = find_test(name);
    if (t == NULL) { printf("NO THAT TEST: %s\n", name); return 2; }

    remove("errors.log");
    remove("tests_errors.txt");

    return run_isolated(t);
}
//==================================================================================================================

//============================== Выводит список всех доступных тестов ================================
static void list_tests(void)
{
    for (uint64_t i = 0; i < NUMBER_OF_TESTS; i++)
    {
        printf("TEST %" PRIu64 ": %s: %s", i,
                                      TESTS[i].name, 
                                      TESTS[i].expect == SURVIVE ? "should survive " : "MAST crash ");
        if (strcmp(TESTS[i].log, "\n") != 0) { printf("with log: %s", TESTS[i].log); }
        else                                 { printf("%s",           TESTS[i].log); }
    }   
}
//=====================================================================================================

int main(int argc, char* argv[])
{
    if (argc >= 3 && !strcmp(argv[1], "--one"))                  return run_one(argv[2]);
    if (argc >= 2 && !strcmp(argv[1], "--list")) { list_tests(); return 0; }

    return 0;
}