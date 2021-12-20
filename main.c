#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#include "core_list.h"
#include "core_buf.h"
#include "hashmap.h"
#include "core_strutil.h"
#include "core_file.h"
#include "core_mem.h"
#include "core_array.h"
#include "uuid4gen.h"

#define axTEST(expr) do {\
  if( !(expr) ) {\
    fprintf(stderr, "test fail: (%s:%s():%d) : [%s]\n" \
    , __FILE__, __func__, __LINE__, #expr);\
    exit(128);\
  }\
}while(0)

void test_strstarts_1()
{
    char *what = "1";
    char *with = "1";
    axTEST(strstarts(what, with));
}

void test_strstarts_2()
{
    char *what = "";
    char *with = " ";
    axTEST(!strstarts(what, with));
}

void test_strstarts_3()
{
    char *what = "12345";
    char *with = "1234.";
    axTEST(!strstarts(what, with));
}

void test_strstarts_4()
{
    char *what = "12345 ";
    char *with = "12345 ";
    axTEST(strstarts(what, with));
}

void test_strstarts_5()
{
    axTEST(strstarts("usr/local/include/", "usr"));
    axTEST(strstarts("usr/local/include/", "usr/local"));
    axTEST(strstarts("usr/local/include/", "usr/local/"));
}

void test_strends_1()
{
    char *what = "12345";
    char *with = "45";
    axTEST(strends(what, with));
}

void test_strends_2()
{
    char *what = "12345";
    char *with = " 45";
    axTEST(!strends(what, with));
}

void test_strends_3()
{
    char *what = "12345";
    char *with = "12345";
    axTEST(strends(what, with));
}

void test_strends_4()
{
    char *what = "test/include";
    char *with = "include";
    axTEST(strends(what, with));
}

void test_strends_5()
{
    axTEST(strends("test/include", "include"));
    axTEST(strends("test/include", "/include"));
    axTEST(strends("test/include", "t/include"));
}

int streq(char *s1, char *s2)
{
    return strcmp(s1, s2) == 0;
}

void test_strleft_0()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "1024");

    struct strbuilder *res = sb_left(buf, 2);
    axTEST(streq(res->str, "10"));
}

void test_strleft_1()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "1024");

    struct strbuilder *res = sb_left(buf, 200);
    axTEST(streq(res->str, "1024"));
}

void test_strleft_2()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "");

    struct strbuilder *res = sb_left(buf, 0);
    axTEST(streq(res->str, ""));
}

void test_strright_0()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "1024");

    struct strbuilder *res = sb_right(buf, 2);
    axTEST(streq(res->str, "24"));
}

void test_strright_1()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "1024");

    struct strbuilder *res = sb_right(buf, 2000);
    axTEST(streq(res->str, "1024"));
}

void test_strmid_0()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_mid(buf, 0, 3);
    axTEST(streq(res->str, "str"));
}

void test_strmid_1()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_mid(buf, 3, 300);
    axTEST(streq(res->str, "ing_1024"));
}

void test_strmid_2()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_mid(buf, 3, 3);
    axTEST(streq(res->str, "ing"));
}

void test_strmid_3()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_mid(buf, 3, 8);
    axTEST(streq(res->str, "ing_1024"));
}

void test_strmid_4()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_mid(buf, 3, 9);
    axTEST(streq(res->str, "ing_1024"));
}

void test_strmid_5()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_mid(buf, 3, 7);
    axTEST(streq(res->str, "ing_102"));
}

void test_strtrim_0()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "\n\n  string_1024 \n\n  \n\n\t \r\n\n ");

    struct strbuilder *res = sb_trim(buf);
    axTEST(streq(res->str, "string_1024"));
}

void test_strtrim_1()
{
    struct strbuilder *buf = sb_new();
    sb_adds(buf, "string_1024");

    struct strbuilder *res = sb_trim(buf);
    axTEST(streq(res->str, "string_1024"));
    //printf("[%s]\n", res->str);
}

void test_buf_0()
{
    Cbuffer * buf = ccbuf_new("");
    axTEST(buf->size == 0);

    int c = nextc(buf);
    axTEST(c == HC_FEOF);
}

void test_buf_1()
{
    Cbuffer * buf = ccbuf_new("abc");
    axTEST(buf->size == 3);

    axTEST(nextc(buf) == 'a');
    axTEST(nextc(buf) == 'b');
    axTEST(nextc(buf) == 'c');
    axTEST(nextc(buf) == HC_FEOF);
}

void test_buf_2()
{
    Cbuffer * buf = ccbuf_new("a\\\nb\\\nc");

    axTEST(nextc(buf) == 'a');
    axTEST(nextc(buf) == 'b');
    axTEST(nextc(buf) == 'c');
    axTEST(nextc(buf) == HC_FEOF);
}

void test_buf_3()
{
    size_t s = 0;
    char * source = readfile("main.c", &s);
    Cbuffer * buf = ccbuf_new(source);
    for (;;) {
        int c = nextc(buf);
        if (c == HC_FEOF) {
            break;
        }
        // printf("%c", c);
    }
}

void test_strmid_again()
{
    axTEST(streq(sb_mid(sb_news("3f420a10-5465-"), 4, 9)->str, "0a10-5465"));
    axTEST(streq(sb_mid(sb_news("f8535dba-8cfa-"), 5, 9)->str, "dba-8cfa-"));
    axTEST(streq(sb_mid(sb_news("0b9edb1c-b2bb-"), 0, 6)->str, "0b9edb"));
    axTEST(streq(sb_mid(sb_news("f34285f2-5ffb-"), 3, 14)->str, "285f2-5ffb-"));
    axTEST(streq(sb_mid(sb_news("bff4aa0f-2727-"), 6, 11)->str, "0f-2727-"));
    axTEST(streq(sb_mid(sb_news("209a689c-527c-"), 10, 6)->str, "27c-"));
    axTEST(streq(sb_mid(sb_news("269da2ec-c7df-"), 0, 7)->str, "269da2e"));
    axTEST(streq(sb_mid(sb_news("22e0b7cb-21ec-"), 5, 4)->str, "7cb-"));
    axTEST(streq(sb_mid(sb_news("ad2b4db3-d89e-"), 6, 1)->str, "b"));
    axTEST(streq(sb_mid(sb_news("3833781d-b109-"), 3, 14)->str, "3781d-b109-"));
    axTEST(streq(sb_mid(sb_news("8e132b56-3832-"), 10, 9)->str, "832-"));
    axTEST(streq(sb_mid(sb_news("163e950d-3908-"), 7, 11)->str, "d-3908-"));
    axTEST(streq(sb_mid(sb_news("1d5f3c7b-ab4d-"), 7, 13)->str, "b-ab4d-"));
    axTEST(streq(sb_mid(sb_news("9e0a1634-9632-"), 0, 8)->str, "9e0a1634"));
    axTEST(streq(sb_mid(sb_news("7508749d-eabc-"), 11, 4)->str, "bc-"));
    axTEST(streq(sb_mid(sb_news("6c0e523b-1272-"), 9, 7)->str, "1272-"));
    axTEST(streq(sb_mid(sb_news("15635e34-69ac-"), 1, 10)->str, "5635e34-69"));
    axTEST(streq(sb_mid(sb_news("8f41d3d7-b717-"), 8, 7)->str, "-b717-"));
    axTEST(streq(sb_mid(sb_news("bf84b542-ba2b-"), 0, 11)->str, "bf84b542-ba"));
    axTEST(streq(sb_mid(sb_news("58ae2e21-4d2d-"), 12, 3)->str, "d-"));
    axTEST(streq(sb_mid(sb_news("fd255310-91d4-"), 4, 7)->str, "5310-91"));
    axTEST(streq(sb_mid(sb_news("ea840d0e-19de-"), 12, 8)->str, "e-"));
    axTEST(streq(sb_mid(sb_news("e5306b54-665d-"), 4, 6)->str, "6b54-6"));
    axTEST(streq(sb_mid(sb_news("7befdd16-cd27-"), 13, 3)->str, "-"));
    axTEST(streq(sb_mid(sb_news("de808b5f-9ebe-"), 0, 0)->str, ""));
    axTEST(streq(sb_mid(sb_news("34ecbaff-25a5-"), 2, 2)->str, "ec"));
    axTEST(streq(sb_mid(sb_news("4be184fd-573f-"), 0, 8)->str, "4be184fd"));
    axTEST(streq(sb_mid(sb_news("ffcf774c-e3c5-"), 0, 3)->str, "ffc"));
    axTEST(streq(sb_mid(sb_news("bf06c07c-240f-"), 4, 11)->str, "c07c-240f-"));
    axTEST(streq(sb_mid(sb_news("d4362892-993d-"), 13, 14)->str, "-"));
    axTEST(streq(sb_mid(sb_news("fc36edf4-647c-"), 6, 1)->str, "f"));
    axTEST(streq(sb_mid(sb_news("d39f9cbe-d1a0-"), 4, 3)->str, "9cb"));
    axTEST(streq(sb_mid(sb_news("56ed5dc4-d6cc-"), 9, 13)->str, "d6cc-"));
}

struct token_simple {
    char *value;
    int type;
    int flag;
};

struct token_simple * token_simple_new(char *name, int type, int flag)
{
    struct token_simple * rv = cc_malloc(sizeof(struct token_simple));
    rv->value = cc_strdup(name);
    rv->type = type;
    rv->flag = flag;
    return rv;
}

bool token_simple_equal(void *a, void *b)
{

    struct token_simple * first = (struct token_simple*) a;
    struct token_simple * second = (struct token_simple*) b;

    if (!streq(first->value, second->value)) {
        return false;
    }
    if (first->type != second->type) {
        return false;
    }
    if (first->flag != second->flag) {
        return false;
    }
    return true;
}

size_t token_simple_hash(void * elem)
{
    size_t ptr_hash_size = *((size_t*) elem);
    return ptr_hash_size;
}

void token_simple_print(struct token_simple * elem, char * val)
{
    printf("value=%s, type=%d, flag=%d; valmap=%s\n", elem->value, elem->type, elem->flag, val);
}

void test_hashmap_pointers_1()
{
    HashMap * map = HashMap_new(&token_simple_hash, &token_simple_equal);

    struct token_simple * a1 = token_simple_new("a", 0, 1);

    HashMap_add(map, a1, "a1");
    HashMap_add(map, a1, "a2");
    HashMap_add(map, a1, "a3");

    char * res = (char*) HashMap_get(map, a1);
    axTEST(res);
    axTEST(streq(res, "a3"));

//    for (size_t i = 0; i < map->capacity; i++) {
//        Entry* e = map->table[i];
//        if (e == NULL) {
//            continue;
//        }
//        printf("%4lu: ", i);
//        for (; e; e = e->next) {
//            token_simple_print(e->key, e->val);
//        }
//    }
}

void test_hashmap_str_1()
{
    HashMap * map = HashMap_new_str();
    HashMap_add(map, "1", "1");
    HashMap_add(map, "2", "2");
    HashMap_add(map, "3", "3");

    axTEST(HashMap_size(map) == 3);
    axTEST(streq(HashMap_get(map, "1"), "1"));
    axTEST(streq(HashMap_get(map, "2"), "2"));
    axTEST(streq(HashMap_get(map, "3"), "3"));

    HashMap_add(map, "1", "0");
    axTEST(HashMap_size(map) == 3);
    axTEST(streq(HashMap_get(map, "1"), "0"));
}

void test_split_char_1()
{
    StringBuilder *input = sb_news("src/test/inlude/my/folder/to/std/headers/");
    LinkedList * lines = sb_split_char(input, '/', false);
    for (ListNode * node = lines->first; node; node = node->next) {
        char *str = (char*) node->item;
        assert(str);
    }
}

void test_replace_0()
{
    StringBuilder * where = sb_news("illidiance");
    char * olds = "illi";
    char * news = ".";
    axTEST(streq(sb_replace(where, olds, news)->str, ".diance"));
}

void test_array_0()
{
    ArrayList *arr = array_new();
    for (int i = 0; i < 10; i++) {
        char buf[32] = { '\0' };
        sprintf(buf, "%05d", i);
        array_add(arr, cc_strdup(buf));
    }
    for (size_t i = 0; i < arr->size; i++) {
        //printf("%s\n", (char*) array_get(arr, i));
    }
}

static bool strequal(void *a, void *b)
{
    char *str_1 = (char*) a;
    char *str_2 = (char*) b;
    return strcmp(str_1, str_2) == 0;
}

void test_list_remove_0()
{
    LinkedList *list = list_new(strequal);
    char *a = "a";
    char *b = "b";
    char *c = "c";

    list_push_back(list, a);
    list_push_back(list, b);
    list_push_back(list, c);

    list_remove(list, b);

    for (ListNode *x = list->first; x; x = x->next) {
        char *str = (char*) x->item;
        printf("%s\n", str);
    }
}

#if 0 // work with strtoX
int main(void)
{

    test_buf_0();
    test_buf_1();
    test_buf_2();
    test_buf_3();

    test_strstarts_1();
    test_strstarts_2();
    test_strstarts_3();
    test_strstarts_4();
    test_strstarts_5();
    test_strends_1();
    test_strends_2();
    test_strends_3();
    test_strends_4();
    test_strends_5();
    test_strleft_0();
    test_strleft_1();
    test_strleft_2();
    test_strright_0();
    test_strright_1();
    test_strmid_0();
    test_strmid_1();
    test_strmid_2();
    test_strmid_3();
    test_strmid_4();
    test_strmid_5();
    test_strtrim_0();
    test_strtrim_1();
    test_strmid_again();

    test_hashmap_pointers_1();
    test_hashmap_str_1();

    test_split_char_1();
    test_replace_0();

    test_array_0();
    test_list_remove_0();

    char buf[64];
    for (int i = 0; i < 10; i += 1) {
        uuid4gen(buf);
        printf("%s\n", buf);
    }

    printf("\n:ok:\n");
    return 0;
}
#endif

