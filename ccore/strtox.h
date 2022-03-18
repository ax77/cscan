/*
 * strtox.h
 *
 *  Created on: 18 мар. 2022 г.
 *      Author: Alexey
 */

#ifndef CCORE_STRTOX_H_
#define CCORE_STRTOX_H_

#include <stddef.h>

/// NOTE:
/// We assume during parsing that all integers are unsigned,
/// and we treat them as u64. Then - we look at the main_sign,
/// and if the sign is present, and it's negative - we will
/// check whether the parsed and evaluated integer suits to
/// all of the limits. For simplicity, because checking these
/// limits during evaluation not so simple as it may appears.

typedef struct strtox Strtox;
struct strtox {

    // correct:
    // FLOATING+B10, FLOATING+B16
    // INTEGER+B2, INTEGER+B8, INTEGER+B10, INTEGER+B16

    // 0x1.c0f8663623b19p+14
    // evaltype = FLOATING
    // evalbase = B16
    enum {
        EVALTYPE_ERROR = -1, EVALTYPE_FLOATING, EVALTYPE_INTEGER,
    } evaltype;
    enum {
        EVALBASE_ERROR = -1, EVALBASE_2 = 2, EVALBASE_8 = 8, EVALBASE_10 = 10, EVALBASE_16 = 16,
    } evalbase;

    // We keep the whole parsed data only because
    // we need to test whether the parser have recognized all
    // things correctly. And also - we may interfere the
    // type according to the suffix we parsed.
    //
    // 0x1.c0f8663623b19p+14
    // main_sign = '+'
    // dec       = "1"
    // mnt       = "c0f8663623b19"
    // exp       = "14"
    // exp_sign  = '+'
    // suf       = ""

    char main_sign;
    char *dec;
    char *mnt;
    char *exp;
    char exp_sign;
    char *suf;

    union {
        ptrdiff_t i64;
        size_t u64;
        float f32;
        double f64;
    };
};

Strtox *parse_number(char *n);

#endif /* CCORE_STRTOX_H_ */
