#include "ascii.h"
#include "hdrs.h"

size_t eval_integer(char *dec, unsigned base) {
    assert(dec);

    size_t len = strlen(dec);
    assert(len);

    size_t retval = 0;

    for (size_t i = 0; i < len; i++) {
        int c = dec[i];
        assert(char_correct_for_base(c, base));
        retval = retval * base + char_value(base, c);
    }

    return retval;

}

double eval_float_16(char *dec, char *mnt, char *exp, char sig) {

    assert(dec);
    assert(mnt);
    assert(exp);

    size_t declen = strlen(dec);
    size_t mntlen = strlen(mnt);
    size_t explen = strlen(exp);

    double realval = 0.0;

    for (size_t i = 0; i < declen; i++) {
        const int c = dec[i];
        realval = realval * 16.0 + (double) char_value(16, c);
    }

    if (mntlen > 0) {
        double m = 0.0625;
        for (size_t i = 0; i < mntlen; i++) {
            const int c = mnt[i];
            realval = realval + (double) char_value(16, c) * m;
            m *= 0.0625;
        }
    }

    int div = 0;
    if (sig == '-') {
        div = 1;
    }

    if (explen > 0) {

        int pow = 0;
        for (size_t i = 0; i < explen; i++) {
            const int c = exp[i];
            pow = pow * 10 + (int) char_value(10, c);
        }
        double m = 1.0;
        for (int i = 0; i < pow; i++) {
            m *= 2.0;
        }
        if (div) {
            realval /= m;
        } else {
            realval *= m;
        }

    }

    return realval;

}

double eval_float_10(char *dec, char *mnt, char *exp, char sig) {

    assert(dec);
    assert(mnt);
    assert(exp);

    size_t declen = strlen(dec);
    size_t mntlen = strlen(mnt);
    size_t explen = strlen(exp);

    double realval = 0.0;

    for (size_t i = 0; i < declen; i++) {
        const int c = dec[i];
        realval = realval * 10.0 + (double) char_value(10, c);
    }

    if (mntlen > 0) {
        double m = 0.1;
        for (size_t i = 0; i < mntlen; i++) {
            const int c = mnt[i];
            realval = realval + (double) char_value(10, c) * m;
            m *= 0.1;
        }
    }

    int div = 0;
    if (sig == '-') {
        div = 1;
    }

    if (explen > 0) {

        int pow = 0;
        for (size_t i = 0; i < explen; i++) {
            const int c = exp[i];
            pow = pow * 10 + (int) char_value(10, c);
        }
        double m = 1.0;
        for (int i = 0; i < pow; i++) {
            m *= 10.0;
        }
        if (div) {
            realval /= m;
        } else {
            realval *= m;
        }

    }

    return realval;
}
