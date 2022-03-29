#include "ccore/vec.h"
#include "ccore/utest.h"

void test_vec0()
{
    vec(i8) buf = VEC_INIT(i8);
    vec_push_back(&buf, 'a');
    assert_true(vec_size(&buf) == 1);

    char c = vec_pop_back(&buf);
    assert_true(c == 'a');
    assert_true(vec_size(&buf) == 0);
}

void test_vec1()
{
    vec(i8) buf = VEC_INIT(i8);
    vec_push_back(&buf, 'a');
    assert_true(vec_size(&buf) == 1);

    char c = vec_set(&buf, 0, 'b');
    assert_true(c == 'a');
    assert_true(vec_size(&buf) == 1);
    assert_true(vec_get(&buf, 0) == 'b');
}

void test_vec2()
{
    vec(i8) buf = VEC_INIT(i8);
    for (int i = 33; i < CHAR_MAX; i++) {
        vec_push_back(&buf, (char ) i);
    }
    char *content =
            "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    assert_true(strcmp(content, buf.data) == 0);
    assert_true(strlen(content) == buf.size);
    assert_true(vec_get(&buf, 0) == '!');
    assert_true(vec_get(&buf, buf.size-1) == '~');
}

static int xxx_streq(char *a, char *b)
{
    return strcmp(a, b) == 0;
}

void test_vec3()
{
    vec(str) v = VEC_INIT(str);
    vec_push_back(&v, "1");
    vec_push_back(&v, "2");
    vec_push_back(&v, "3");

    assert_true(vec_index_of(&v, "1", xxx_streq) == 0);
    assert_true(vec_index_of(&v, "2", xxx_streq) == 1);
    assert_true(vec_index_of(&v, "3", xxx_streq) == 2);
    assert_true(vec_index_of(&v, " ", xxx_streq) == -1);
}

void test_vec4()
{
    //1
    vec(u32) v = VEC_INIT(u32);
    vec_push_back(&v, 1);
    vec_push_back(&v, 2);
    vec_push_back(&v, 3);
    vec_push_back(&v, 4);
    vec_push_back(&v, 5);

    assert_true(vec_remove(&v, 0) == 1);
    assert_true(vec_size(&v) == 4);

    assert_true(vec_remove(&v, 0) == 2);
    assert_true(vec_size(&v) == 3);

    assert_true(vec_remove(&v, 0) == 3);
    assert_true(vec_size(&v) == 2);

    assert_true(vec_remove(&v, vec_size(&v)-1) == 5);
    assert_true(vec_size(&v) == 1);

    assert_true(vec_remove(&v, vec_size(&v)-1) == 4);
    assert_true(vec_size(&v) == 0);

    //2
    vec(i8) str = VEC_INIT(i8);
    vec_push_back(&str, 'a');
    vec_push_back(&str, 'b');
    vec_push_back(&str, 'c');
    vec_push_back(&str, 'd');
    vec_push_back(&str, 'e');

    assert_true(xxx_streq(str.data, "abcde"));
    vec_remove(&str, 2);
    assert_true(xxx_streq(str.data, "abde"));

    //3
    vec(i8) buf = VEC_INIT(i8);
    for (int i = 33; i < CHAR_MAX; i++) {
        vec_push_back(&buf, (char ) i);
    }
    char *content =
            "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    size_t len = strlen(content);
    for (size_t i = 0; i < len; i++) {
        assert_true(vec_remove(&buf,0 ) == content[i]);
    }
}

static int i32_cmpfunc(const void *a, const void *b)
{
    return (*(int*) a - *(int*) b);
}

static int char_ptr_cmpfunc(const void *str1, const void *str2)
{
    char *const*pp1 = str1;
    char *const*pp2 = str2;
    return strcmp(*pp1, *pp2);
}

void test_vec5()
{
    vec(u32) v = VEC_INIT(u32);

    vec_insert(&v, 0, 32);
    assert_true(vec_get(&v, 0) == 32);
    assert_true(vec_size(&v) == 1);

    vec_insert(&v, 0, 64);
    assert_true(vec_get(&v, 0) == 64);
    assert_true(vec_get(&v, 1) == 32);
    assert_true(vec_size(&v) == 2);

    //64,128,32
    vec_insert(&v, 1, 128);
    assert_true(vec_get(&v, 0) == 64);
    assert_true(vec_get(&v, 1) == 128);
    assert_true(vec_get(&v, 2) == 32);
    assert_true(vec_size(&v) == 3);

    vec_sort(&v, &i32_cmpfunc);
    assert_true(vec_get(&v, 0) == 32);
    assert_true(vec_get(&v, 1) == 64);
    assert_true(vec_get(&v, 2) == 128);
    assert_true(vec_size(&v) == 3);

    vec_clear(&v);
    assert_true(vec_size(&v) == 0);
}

void test_vec7()
{
    vec(str) v = VEC_INIT(str);
    vec_push_back(&v, "100");
    vec_push_back(&v, "010");
    vec_push_back(&v, "011");
    vec_push_back(&v, "001");
    vec_push_back(&v, "000");
    vec_push_back(&v, "101");
    vec_push_back(&v, "111");
    vec_push_back(&v, "110");

    vec_sort(&v, &char_ptr_cmpfunc);
    assert_true(vec_size(&v) == 8);
    assert_true(xxx_streq("000", vec_get(&v,0)));
    assert_true(xxx_streq("001", vec_get(&v,1)));
    assert_true(xxx_streq("010", vec_get(&v,2)));
    assert_true(xxx_streq("011", vec_get(&v,3)));
    assert_true(xxx_streq("100", vec_get(&v,4)));
    assert_true(xxx_streq("101", vec_get(&v,5)));
    assert_true(xxx_streq("110", vec_get(&v,6)));
    assert_true(xxx_streq("111", vec_get(&v,7)));

}

