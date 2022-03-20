#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#include "ccore/strtox.h"
#include "ccore/buf.h"
#include "ccore/map.h"
#include "ccore/str.h"
#include "ccore/utest.h"
#include "ccore/vec.h"
#include "ccore/xmem.h"
#include "ccore/list.h"
#include "ccore/uuid4.h"
#include "ccore/fdesc.h"
#include "ccore/eval.h"
#include "tests.h"

int streq(char *s1, char *s2)
{
    return strcmp(s1, s2) == 0;
}

void test_buf_0()
{
    CharBuf *buf = charbuf_new("");
    assert_true(buf->size == 0);

    int c = nextc(buf);
    assert_true(c == HC_FEOF);
}

void test_buf_1()
{
    CharBuf *buf = charbuf_new("abc");
    assert_true(buf->size == 3);

    assert_true(nextc(buf) == 'a');
    assert_true(nextc(buf) == 'b');
    assert_true(nextc(buf) == 'c');
    assert_true(nextc(buf) == HC_FEOF);
}

void test_buf_2()
{
    CharBuf *buf = charbuf_new("a\\\nb\\\nc");

    assert_true(nextc(buf) == 'a');
    assert_true(nextc(buf) == 'b');
    assert_true(nextc(buf) == 'c');
    assert_true(nextc(buf) == HC_FEOF);
}

void test_buf_3()
{
    size_t s = 0;
    char *source = hb_readfile("main.c", &s);
    CharBuf *buf = charbuf_new(source);
    for (;;) {
        int c = nextc(buf);
        if (c == HC_FEOF) {
            break;
        }
        // printf("%c", c);
    }
}

struct token_simple {
    char *value;
    int type;
    int flag;
};

struct token_simple* token_simple_new(char *name, int type, int flag)
{
    struct token_simple *rv = cc_malloc(sizeof(struct token_simple));
    rv->value = cc_strdup(name);
    rv->type = type;
    rv->flag = flag;
    return rv;
}

int token_simple_equal(void *a, void *b)
{

    struct token_simple *first = (struct token_simple*) a;
    struct token_simple *second = (struct token_simple*) b;

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

size_t token_simple_hash(void *elem)
{
    size_t ptr_hash_size = *((size_t*) elem);
    return ptr_hash_size;
}

void token_simple_print(struct token_simple *elem, char *val)
{
    printf("value=%s, type=%d, flag=%d; valmap=%s\n", elem->value, elem->type,
            elem->flag, val);
}

void test_hashmap_pointers_1()
{
    HashMap *map = HashMap_new(&token_simple_hash, &token_simple_equal);

    struct token_simple *a1 = token_simple_new("a", 0, 1);

    HashMap_put(map, a1, "a1");
    HashMap_put(map, a1, "a2");
    HashMap_put(map, a1, "a3");

    char *res = (char*) HashMap_get(map, a1);
    assert_true(res);
    assert_true(streq(res, "a3"));
}

void test_hashmap_str_1()
{
    HashMap *map = HashMap_new_str();
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

void test_str_pop()
{
    Str sb = STR_INIT;
    char *input = "1234567";
    sb_adds_rev(&sb, input);
}

void test_charbuf()
{
    CharBuf *b = charbuf_new("a\\\nb");
    assert_true('a' == nextc(b));
    assert_true('b' == nextc(b));
    assert_true(-1 == nextc(b));
}

void test_eval()
{
    assert_true(1024 == eval_integer("010000000000", 2));
    assert_true(1024 == eval_integer("2000", 8));
    assert_true(1024 == eval_integer("1024", 10));
    assert_true(1024 == eval_integer("400", 16));

    Strtox *data = parse_number("0x1.cd05bc61f9e57p+18");
    assert_true(FLOATING_16 == data->evaltype);
    assert_true('+' == data->main_sign);
    assert_true(strequal("1", data->dec));
    assert_true(strequal("cd05bc61f9e57", data->mnt));
    assert_true(strequal("18", data->exp));
    assert_true('+' == data->exp_sign);
    assert_true(strequal("", data->suf));

    data = parse_number("0x0");
    assert_true(INTEGER_16 == data->evaltype);
    assert_true(strequal("0", data->dec));
    assert_true(strequal("", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number("0");
    assert_true(INTEGER_10 == data->evaltype);
    assert_true(strequal("0", data->dec));
    assert_true(strequal("", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number("01");
    assert_true(INTEGER_8 == data->evaltype);
    assert_true(strequal("1", data->dec));
    assert_true(strequal("", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number("0b0");
    assert_true(INTEGER_2 == data->evaltype);
    assert_true(strequal("0", data->dec));
    assert_true(strequal("", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number("3.14");
    assert_true(FLOATING_10 == data->evaltype);
    assert_true(strequal("3", data->dec));
    assert_true(strequal("14", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number(".14");
    assert_true(FLOATING_10 == data->evaltype);
    assert_true(strequal("", data->dec));
    assert_true(strequal("14", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number("3.830124e+05");
    assert_true(FLOATING_10 == data->evaltype);
    assert_true(strequal("3", data->dec));
    assert_true(strequal("830124", data->mnt));
    assert_true(strequal("05", data->exp));
    assert_true(strequal("", data->suf));

    data = parse_number("383012.4228341295965947L");
    assert_true(FLOATING_10 == data->evaltype);
    assert_true(strequal("383012", data->dec));
    assert_true(strequal("4228341295965947", data->mnt));
    assert_true(strequal("", data->exp));
    assert_true(strequal("L", data->suf));
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
    test_normalize_1();
    test_str_pop();
    test_charbuf();
    test_strmid_1();
    test_eval();
    test_strtox_stdlib();

    test_str_0();

    printf("\n:ok:\n");
    return 0;
}

