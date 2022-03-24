#include "ccore/vec.h"
#include "ccore/utest.h"

void test_vec0() {
    vec(i8) buf = VEC_INIT(i8);
    vec_push_back(&buf, 'a');
    assert_true(vec_size(&buf) == 1);

    char c = vec_pop_back(&buf);
    assert_true(c == 'a');
    assert_true(vec_size(&buf) == 0);
}

void test_vec1() {
    vec(i8) buf = VEC_INIT(i8);
    vec_push_back(&buf, 'a');
    assert_true(vec_size(&buf) == 1);

    char c = vec_set(&buf, 0, 'b');
    assert_true(c == 'a');
    assert_true(vec_size(&buf) == 1);
    assert_true(vec_get(&buf, 0) == 'b');
}
