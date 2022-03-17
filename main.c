#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#include "ccore/buf.h"
#include "ccore/map.h"
#include "ccore/str.h"
#include "ccore/utest.h"
#include "ccore/vec.h"
#include "ccore/xmem.h"
#include "ccore/list.h"
#include "ccore/uuid4.h"
#include "ccore/fdesc.h"

void test_strstarts_1()
{
    char *what = "1";
    char *with = "1";
    assert_true(strstarts(what, with));
}

void test_strstarts_2()
{
    char *what = "";
    char *with = " ";
    assert_true(!strstarts(what, with));
}

void test_strstarts_3()
{
    char *what = "12345";
    char *with = "1234.";
    assert_true(!strstarts(what, with));
}

void test_strstarts_4()
{
    char *what = "12345 ";
    char *with = "12345 ";
    assert_true(strstarts(what, with));
}

void test_strstarts_5()
{
    assert_true(strstarts("usr/local/include/", "usr"));
    assert_true(strstarts("usr/local/include/", "usr/local"));
    assert_true(strstarts("usr/local/include/", "usr/local/"));
}

void test_strends_1()
{
    char *what = "12345";
    char *with = "45";
    assert_true(strends(what, with));
}

void test_strends_2()
{
    char *what = "12345";
    char *with = " 45";
    assert_true(!strends(what, with));
}

void test_strends_3()
{
    char *what = "12345";
    char *with = "12345";
    assert_true(strends(what, with));
}

void test_strends_4()
{
    char *what = "test/include";
    char *with = "include";
    assert_true(strends(what, with));
}

void test_strends_5()
{
    assert_true(strends("test/include", "include"));
    assert_true(strends("test/include", "/include"));
    assert_true(strends("test/include", "t/include"));
}

int streq(char *s1, char *s2)
{
    return strcmp(s1, s2) == 0;
}

void test_buf_0()
{
    CharBuf * buf = charbuf_new("");
    assert_true(buf->size == 0);

    int c = nextc(buf);
    assert_true(c == HC_FEOF);
}

void test_buf_1()
{
    CharBuf * buf = charbuf_new("abc");
    assert_true(buf->size == 3);

    assert_true(nextc(buf) == 'a');
    assert_true(nextc(buf) == 'b');
    assert_true(nextc(buf) == 'c');
    assert_true(nextc(buf) == HC_FEOF);
}

void test_buf_2()
{
    CharBuf * buf = charbuf_new("a\\\nb\\\nc");

    assert_true(nextc(buf) == 'a');
    assert_true(nextc(buf) == 'b');
    assert_true(nextc(buf) == 'c');
    assert_true(nextc(buf) == HC_FEOF);
}

void test_buf_3()
{
    size_t s = 0;
    char * source = hb_readfile("main.c", &s);
    CharBuf * buf = charbuf_new(source);
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
    assert_true(streq(sb_mid(("3f420a10-5465-"), 4, 9), "0a10-5465"));
    assert_true(streq(sb_mid(("f8535dba-8cfa-"), 5, 9), "dba-8cfa-"));
    assert_true(streq(sb_mid(("0b9edb1c-b2bb-"), 0, 6), "0b9edb"));
    assert_true(streq(sb_mid(("f34285f2-5ffb-"), 3, 14), "285f2-5ffb-"));
    assert_true(streq(sb_mid(("bff4aa0f-2727-"), 6, 11), "0f-2727-"));
    assert_true(streq(sb_mid(("209a689c-527c-"), 10, 6), "27c-"));
    assert_true(streq(sb_mid(("269da2ec-c7df-"), 0, 7), "269da2e"));
    assert_true(streq(sb_mid(("22e0b7cb-21ec-"), 5, 4), "7cb-"));
    assert_true(streq(sb_mid(("ad2b4db3-d89e-"), 6, 1), "b"));
    assert_true(streq(sb_mid(("3833781d-b109-"), 3, 14), "3781d-b109-"));
    assert_true(streq(sb_mid(("8e132b56-3832-"), 10, 9), "832-"));
    assert_true(streq(sb_mid(("163e950d-3908-"), 7, 11), "d-3908-"));
    assert_true(streq(sb_mid(("1d5f3c7b-ab4d-"), 7, 13), "b-ab4d-"));
    assert_true(streq(sb_mid(("9e0a1634-9632-"), 0, 8), "9e0a1634"));
    assert_true(streq(sb_mid(("7508749d-eabc-"), 11, 4), "bc-"));
    assert_true(streq(sb_mid(("6c0e523b-1272-"), 9, 7), "1272-"));
    assert_true(streq(sb_mid(("15635e34-69ac-"), 1, 10), "5635e34-69"));
    assert_true(streq(sb_mid(("8f41d3d7-b717-"), 8, 7), "-b717-"));
    assert_true(streq(sb_mid(("bf84b542-ba2b-"), 0, 11), "bf84b542-ba"));
    assert_true(streq(sb_mid(("58ae2e21-4d2d-"), 12, 3), "d-"));
    assert_true(streq(sb_mid(("fd255310-91d4-"), 4, 7), "5310-91"));
    assert_true(streq(sb_mid(("ea840d0e-19de-"), 12, 8), "e-"));
    assert_true(streq(sb_mid(("e5306b54-665d-"), 4, 6), "6b54-6"));
    assert_true(streq(sb_mid(("7befdd16-cd27-"), 13, 3), "-"));
    assert_true(streq(sb_mid(("de808b5f-9ebe-"), 0, 0), ""));
    assert_true(streq(sb_mid(("34ecbaff-25a5-"), 2, 2), "ec"));
    assert_true(streq(sb_mid(("4be184fd-573f-"), 0, 8), "4be184fd"));
    assert_true(streq(sb_mid(("ffcf774c-e3c5-"), 0, 3), "ffc"));
    assert_true(streq(sb_mid(("bf06c07c-240f-"), 4, 11), "c07c-240f-"));
    assert_true(streq(sb_mid(("d4362892-993d-"), 13, 14), "-"));
    assert_true(streq(sb_mid(("fc36edf4-647c-"), 6, 1), "f"));
    assert_true(streq(sb_mid(("d39f9cbe-d1a0-"), 4, 3), "9cb"));
    assert_true(streq(sb_mid(("56ed5dc4-d6cc-"), 9, 13), "d6cc-"));
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

int token_simple_equal(void *a, void *b)
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

    HashMap_put(map, a1, "a1");
    HashMap_put(map, a1, "a2");
    HashMap_put(map, a1, "a3");

    char * res = (char*) HashMap_get(map, a1);
    assert_true(res);
    assert_true(streq(res, "a3"));
}

void test_hashmap_str_1()
{
    HashMap * map = HashMap_new_str();
    HashMap_put(map, "1", "1");
    HashMap_put(map, "2", "2");
    HashMap_put(map, "3", "3");

    assert_true(HashMap_size(map) == 3);
    assert_true(streq(HashMap_get(map, "1"), "1"));
    assert_true(streq(HashMap_get(map, "2"), "2"));
    assert_true(streq(HashMap_get(map, "3"), "3"));

    HashMap_put(map, "1", "0");
    assert_true(HashMap_size(map) == 3);
    assert_true(streq(HashMap_get(map, "1"), "0"));
}

static void assertEquals(char *s1, char *s2)
{
    assert_true(strequal(s1, s2));
}

void test_normalize()
{
    assertEquals("c:/project/test/header.h", normalize("c:/project//test/./././././././header.h"));
    assertEquals("project/header.h", normalize("./project/test/../header.h"));
    assertEquals("C:/header.h", normalize("C:/project/test/../../../../header.h"));
    assertEquals("../header.h", normalize("../project/test/../../header.h"));
    assertEquals("C:/header.h", normalize("C:/project/../header.h"));
    assertEquals("/usr/header.h", normalize("/usr/include/../header.h"));

    assertEquals("../header.h", normalize("././././../header.h"));
    assertEquals("C:/header.h", normalize("C:\\../../../../../../header.h"));

    assertEquals("header.h", normalize("./header.h"));
    assertEquals("header.h", normalize("header.h"));

    assertEquals("header.h", normalize(".\\//\\//\\//\\//header.h"));
    assertEquals("src/header.h", normalize("src//\\./././header.h"));

    assertEquals("D:/p2/core/cache/binary", normalize("D:\\..\\..\\.\\p2\\core\\cache\\binary"));
    assertEquals("C:/file.c", normalize("C:\\file.c"));
    assertEquals("C:/file.c", normalize("C://file.c"));
    assertEquals("C:/file.c", normalize("C:/file.c"));

    assertEquals("C:/testing.txt", normalize("C:\\temp\\test\\..\\..\\testing.txt"));
    assertEquals("C:/more/testing/test.txt",
            normalize("C:\\temp\\test\\..\\../testing\\..\\more/testing\\test.txt"));

    assertEquals("/usr/local/include/stdio.h", normalize("/usr/local//include//\\stdio.h"));
    assertEquals("", normalize("./"));
    assertEquals("/", normalize("/"));
    assertEquals("../", normalize("../"));
}


void test_str_pop()
{
    Str sb = STR_INIT;
    char *input = "1234567";
    sb_adds_rev(&sb, input);
}

void test_charbuf() {
    CharBuf *b = charbuf_new("a\\\nb");
    assert_true('a' == nextc(b));
    assert_true('b' == nextc(b));
    assert_true(-1 == nextc(b));
}

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
    test_hashmap_pointers_1();
    test_hashmap_str_1();
    list_test0();
    list_test1();
    list_test2();
    list_test3();
    list_test4();
    list_test5();
    list_test6();
    test_normalize();
    test_str_pop();
    test_charbuf();
    test_strmid_again();

    printf("\n:ok:\n");
    return 0;
}

