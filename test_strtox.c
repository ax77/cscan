#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <math.h>
#include "ccore/strtox.h"

#define str(x) #x

static int dbl_cmp(double a, double b, double epsilon)
{
    if (fabsl(a - b) < epsilon) {
        return 1;
    }
    return 0;
}

//static void test_floating()
//{
//    char *endptr = "";
//
//#   define data(FLT, DBL, HEX, EXP) { .expect = FLT, .f = str(FLT), .d = str(DBL), .h = str(HEX), .e = str(EXP) }
//    struct floating {
//        double expect;
//        char *f;
//        char *d;
//        char *h;
//        char *e;
//    } fdata[] = {
//#       include "idata/fdata.data"
//            };
//#   undef data
//    const size_t len = sizeof(fdata) / sizeof(fdata[0]);
//
//    for (int i = 0; i < len; i++) {
//        struct floating x = fdata[i];
//
//        double flt = strtold(x.f, &endptr);
//        double hex = strtold(x.h, &endptr);
//        double exp = strtold(x.e, &endptr);
//
//        if (!dbl_cmp(x.expect, flt, 0.0001f)) {
//            printf("F fail: expect: %f, but actual: %f\n", x.expect, flt);
//        }
//        if (!dbl_cmp(x.expect, hex, 0.0001f)) {
//            printf("H fail: expect: %f, but actual: %f\n", x.expect, hex);
//        }
//        if (!dbl_cmp(x.expect, exp, 0.1f)) {
//            printf("E fail: expect: %f, but actual: %f\n", x.expect, exp);
//        }
//    }
//}
//
//static void test_integers(void)
//{
//
//    char *endptr = "";
//
//#   define data(DEC, BIN, OCT, HEX) { .expect = DEC, .b = str(BIN), .o = str(OCT), .h = str(HEX) }
//    struct integers {
//        ptrdiff_t expect;
//        char *b;
//        char *o;
//        char *h;
//    } idata[] = {
//#   include "idata/idata.data"
//            };
//#   undef data
//    const size_t len = sizeof(idata) / sizeof(idata[0]);
//
//    for (int i = 0; i < len; i++) {
//        struct integers x = idata[i];
//
//        ptrdiff_t bin = strtoll(x.b + 2, &endptr, 2); // +2 - because we have to skip 0b prefix, waiting a new -std=c22 :)
//        ptrdiff_t oct = strtoll(x.o, &endptr, 8);
//        ptrdiff_t hex = strtoll(x.h, &endptr, 16);
//
//        if (x.expect != bin) {
//            printf("B fail: expect: %ld, but actual: %ld\n", x.expect, bin);
//        }
//        if (x.expect != oct) {
//            printf("O fail: expect: %ld, but actual: %ld\n", x.expect, oct);
//        }
//        if (x.expect != hex) {
//            printf("H fail: expect: %ld, but actual: %ld\n", x.expect, hex);
//        }
//    }
//}
//
//static void test_unsigned(void)
//{
//
//    char *endptr = "";
//
//#   define data(DEC, BIN, OCT, HEX) { .expect = DEC, .b = str(BIN), .o = str(OCT), .h = str(HEX) }
//    struct integers {
//        size_t expect;
//        char *b;
//        char *o;
//        char *h;
//    } idata[] = {
//#   include "idata/udata.data"
//            };
//#   undef data
//    const size_t len = sizeof(idata) / sizeof(idata[0]);
//
//    for (int i = 0; i < len; i++) {
//        struct integers x = idata[i];
//
//        size_t bin = strtoull(x.b + 2, &endptr, 2); // +2 - because we have to skip 0b prefix, waiting a new -std=c22 :)
//        size_t oct = strtoull(x.o, &endptr, 8);
//        size_t hex = strtoull(x.h, &endptr, 16);
//
//        if (x.expect != bin) {
//            printf("B fail: expect: %ld, but actual: %ld\n", x.expect, bin);
//        }
//        if (x.expect != oct) {
//            printf("O fail: expect: %ld, but actual: %ld\n", x.expect, oct);
//        }
//        if (x.expect != hex) {
//            printf("H fail: expect: %ld, but actual: %ld\n", x.expect, hex);
//        }
//    }
//}

static ptrdiff_t eval_i64(char *s)
{
    Strtox *x = parse_number(s);
    assert(strtox_is_integer(x));
    return x->i64;
}

static double eval_float(char *s)
{
    Strtox *x = parse_number(s);
    assert(strtox_is_floating(x));
    return x->f64;
}

static void test_eval_i64()
{
#   define data(DEC, BIN, OCT, HEX) { .expect = DEC, .b = str(BIN), .o = str(OCT), .h = str(HEX) }
    struct integers {
        ptrdiff_t expect;
        char *b;
        char *o;
        char *h;
    } idata[] = {
#   include "idata/idata.data"
            };
#   undef data
    const size_t len = sizeof(idata) / sizeof(idata[0]);

    for (int i = 0; i < len; i++) {
        struct integers x = idata[i];

        ptrdiff_t bin = eval_i64(x.b);
        ptrdiff_t oct = eval_i64(x.o);
        ptrdiff_t hex = eval_i64(x.h);

        if (x.expect != bin) {
            printf("B fail: expect: %ld, but actual: %ld\n", x.expect, bin);
        }
        if (x.expect != oct) {
            printf("O fail: expect: %ld, but actual: %ld\n", x.expect, oct);
        }
        if (x.expect != hex) {
            printf("H fail: expect: %ld, but actual: %ld\n", x.expect, hex);
        }

    }
}

static void test_eval_f64()
{

#   define data(FLT, DBL, HEX, EXP) { .expect = FLT, .f = str(FLT), .d = str(DBL), .h = str(HEX), .e = str(EXP) }
    struct floating {
        double expect;
        char *f;
        char *d;
        char *h;
        char *e;
    } fdata[] = {
#       include "idata/fdata.data"
            };
#   undef data
    const size_t len = sizeof(fdata) / sizeof(fdata[0]);

    // data(0.053010, 0.0530104033445833, 0x1.b242df96cee27p-5, 5.301040e-02),

    for (int i = 0; i < len; i++) {
        struct floating x = fdata[i];

        double flt = eval_float(x.f);
        double dbl = eval_float(x.d);
        double hex = eval_float(x.h);
        double exp = eval_float(x.e);

        if (!dbl_cmp(x.expect, flt, 0.0001f)) {
            printf("F fail: expect: %f, but actual: %f\n", x.expect, flt);
        }
        if (!dbl_cmp(x.expect, dbl, 0.0001f)) {
            printf("D fail: expect: %f, but actual: %f\n", x.expect, dbl);
        }
        if (!dbl_cmp(x.expect, hex, 0.0001f)) {
            printf("H fail: expect: %f, but actual: %f\n", x.expect, hex);
        }
        if (!dbl_cmp(x.expect, exp, 0.1f)) {
            printf("E fail: expect: %f, but actual: %f\n", x.expect, exp);
        }

    }
}

void test_strtox_stdlib()
{
    // test_floating();
    // test_integers();
    // test_unsigned();

    test_eval_i64();
    test_eval_f64();
}

