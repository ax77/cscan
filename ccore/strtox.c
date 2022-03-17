#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "ascii.h"
#include "buf.h"
#include "str.h"
#include "xmem.h"

//////////////////////////////////////////////////////////////////////
// TODO: rust suffixes https://doc.rust-lang.org/reference/tokens.html:
//
// Lexer
// INTEGER_LITERAL :
//    ( DEC_LITERAL | BIN_LITERAL | OCT_LITERAL | HEX_LITERAL ) INTEGER_SUFFIX?
//
// DEC_LITERAL :
//    DEC_DIGIT (DEC_DIGIT|_)*
//
// BIN_LITERAL :
//    0b (BIN_DIGIT|_)* BIN_DIGIT (BIN_DIGIT|_)*
//
// OCT_LITERAL :
//    0o (OCT_DIGIT|_)* OCT_DIGIT (OCT_DIGIT|_)*
//
// HEX_LITERAL :
//    0x (HEX_DIGIT|_)* HEX_DIGIT (HEX_DIGIT|_)*
//
// BIN_DIGIT : [0-1]
//
// OCT_DIGIT : [0-7]
//
// DEC_DIGIT : [0-9]
//
// HEX_DIGIT : [0-9 a-f A-F]
//
// INTEGER_SUFFIX :
//       u8 | u16 | u32 | u64 | u128 | usize
//    | i8 | i16 | i32 | i64 | i128 | isize
//
// 123;                               // type i32
// 123i32;                            // type i32
// 123u32;                            // type u32
// 123_u32;                           // type u32
// let a: u64 = 123;                  // type u64
//
// 0xff;                              // type i32
// 0xff_u8;                           // type u8
//
// 0o70;                              // type i32
// 0o70_i16;                          // type i16
//
// 0b1111_1111_1001_0000;             // type i32
// 0b1111_1111_1001_0000i64;          // type i64
// 0b________1;                       // type i32
//
// 0usize;                            // type usize

//////////////////////////////////////////////////////////////////////
// TODO: python literals https://docs.python.org/3/reference/lexical_analysis.html:
//
// integer      ::=  decinteger | bininteger | octinteger | hexinteger
// decinteger   ::=  nonzerodigit (["_"] digit)* | "0"+ (["_"] "0")*
// bininteger   ::=  "0" ("b" | "B") (["_"] bindigit)+
// octinteger   ::=  "0" ("o" | "O") (["_"] octdigit)+
// hexinteger   ::=  "0" ("x" | "X") (["_"] hexdigit)+
// nonzerodigit ::=  "1"..."9"
// digit        ::=  "0"..."9"
// bindigit     ::=  "0" | "1"
// octdigit     ::=  "0"..."7"
// hexdigit     ::=  digit | "a"..."f" | "A"..."F"
//
// floatnumber   ::=  pointfloat | exponentfloat
// pointfloat    ::=  [digitpart] fraction | digitpart "."
// exponentfloat ::=  (digitpart | pointfloat) exponent
// digitpart     ::=  digit (["_"] digit)*
// fraction      ::=  "." digitpart
// exponent      ::=  ("e" | "E") ["+" | "-"] digitpart

// TODO: golang literals https://go.dev/ref/spec#Integer_literals:
// int_lit        = decimal_lit | binary_lit | octal_lit | hex_lit .
// decimal_lit    = "0" | ( "1" … "9" ) [ [ "_" ] decimal_digits ] .
// binary_lit     = "0" ( "b" | "B" ) [ "_" ] binary_digits .
// octal_lit      = "0" [ "o" | "O" ] [ "_" ] octal_digits .
// hex_lit        = "0" ( "x" | "X" ) [ "_" ] hex_digits .
//
// decimal_digits = decimal_digit { [ "_" ] decimal_digit } .
// binary_digits  = binary_digit { [ "_" ] binary_digit } .
// octal_digits   = octal_digit { [ "_" ] octal_digit } .
// hex_digits     = hex_digit { [ "_" ] hex_digit } .
// 42
// 4_2
// 0600
// 0_600
// 0o600
// 0O600       // second character is capital letter 'O'
// 0xBadFace
// 0xBad_Face
// 0x_67_7a_2f_cc_40_c6
// 170141183460469231731687303715884105727
// 170_141183_460469_231731_687303_715884_105727
//
// _42         // an identifier, not an integer literal
// 42_         // invalid: _ must separate successive digits
// 4__2        // invalid: only one _ at a time
// 0_xBadFace  // invalid: _ must separate successive digits
//
//
// float_lit         = decimal_float_lit | hex_float_lit .
//
// decimal_float_lit = decimal_digits "." [ decimal_digits ] [ decimal_exponent ] |
//                     decimal_digits decimal_exponent |
//                     "." decimal_digits [ decimal_exponent ] .
// decimal_exponent  = ( "e" | "E" ) [ "+" | "-" ] decimal_digits .
//
// hex_float_lit     = "0" ( "x" | "X" ) hex_mantissa hex_exponent .
// hex_mantissa      = [ "_" ] hex_digits "." [ hex_digits ] |
//                     [ "_" ] hex_digits |
//                     "." hex_digits .
// hex_exponent      = ( "p" | "P" ) [ "+" | "-" ] decimal_digits .
// 0.
// 72.40
// 072.40       // == 72.40
// 2.71828
// 1.e+0
// 6.67428e-11
// 1E6
// .25
// .12345E+5
// 1_5.         // == 15.0
// 0.15e+0_2    // == 15.0
//
// 0x1p-2       // == 0.25
// 0x2.p10      // == 2048.0
// 0x1.Fp+0     // == 1.9375
// 0X.8p-0      // == 0.5
// 0X_1FFFP-16  // == 0.1249847412109375
// 0x15e-2      // == 0x15e - 2 (integer subtraction)
//
// 0x.p1        // invalid: mantissa has no digits
// 1p-2         // invalid: p exponent requires hexadecimal mantissa
// 0x1.5e-2     // invalid: hexadecimal mantissa requires p exponent
// 1_.5         // invalid: _ must separate successive digits
// 1._5         // invalid: _ must separate successive digits
// 1.5_e1       // invalid: _ must separate successive digits
// 1.5e_1       // invalid: _ must separate successive digits
// 1.5e1_       // invalid: _ must separate successive digits

/// the main routine, here we need to parse the buffer,
/// get all the information from it, and interpret the result.

struct cstrtox_parse_data {

    char *input;

    // how we should interpret the result.
    enum {
        E_ERROR = -1, E_BIN, E_OCT, E_DEC, E_HEX, E_HEX_FLOAT, E_DEC_FLOAT
    } eval_flag;

    // if we found a suffix, we may interpret the number as a C-typed-constant
    enum {
        N_ERROR = -1,
        N_INT,
        N_UINT,
        N_LONG,
        N_ULONG,
        N_LONG_LONG,
        N_ULONG_LONG,
        N_FLOAT,
        N_DOUBLE,
        N_LONG_DOUBLE
    } num_type;

    // store all the information we have from ANY numeric constant.
    // we support so far:
    // 1) binary form: 0b0001
    // 2) octal form: 001
    // 3) decimal: 1024
    // 4) hex form: 0xabcde
    // 5) floats: .032, 072.40, 3.14, 2.22750742e+4f, 0x1.68fcp+14

    struct strbuilder *dec;
    struct strbuilder *mnt;
    struct strbuilder *exp;
    struct strbuilder *suf; // C/C++ suffix: UL, ULL, etc.
    char sig; // exponent sign [+][-]

    // it's a floating point constant in its exponent form
    // 22275.074219f == 2.22750742e+4f
    int dec_float_exp;

    // it's a hex floating constant, and in its exponent form, enjoy it.
    // 22275.074219f == 0x1.5c0c4cp+14F
    int hex_float_exp;

    // and the result as is, we may store any number we interpreted
    // by using this simple union.
    ptrdiff_t i64;
    size_t u64;
    float f32;
    double f64;

};

static void ev(struct cstrtox_parse_data *data);

char* ev_flag_tos(int f)
{
    switch (f) {
    case E_ERROR:
        return "E_ERROR";
    case E_BIN:
        return "E_BIN";
    case E_OCT:
        return "E_OCT";
    case E_DEC:
        return "E_DEC";
    case E_HEX:
        return "E_HEX";
    case E_HEX_FLOAT:
        return "E_HEX_FLOAT";
    case E_DEC_FLOAT:
        return "E_DEC_FLOAT";
    default:
        break;
    }
    return "";
}

struct cstrtox_parse_data* cstrtox_parse_data_new(char *input)
{
    assert(input);

    struct cstrtox_parse_data *rv = cc_malloc(sizeof(struct cstrtox_parse_data));

    rv->input = input;
    rv->eval_flag = E_ERROR;
    rv->num_type = N_ERROR;

    rv->dec = sb_new();
    rv->mnt = sb_new();
    rv->exp = sb_new();
    rv->suf = sb_new();
    rv->sig = '+';

    rv->dec_float_exp = 0;
    rv->hex_float_exp = 0;

    rv->i64 = 0;
    rv->u64 = 0;
    rv->f32 = 0;
    rv->f64 = 0;
    return rv;
}

void cstrtox_print_data(struct cstrtox_parse_data *data)
{
    assert(data);

    printf("inp=%s\n", data->input);
    printf("dec=%s\n", data->dec->str);
    printf("mnt=%s\n", data->mnt->str);
    printf("exp=%s\n", data->exp->str);
    printf("suf=%s\n", data->suf->str);
    printf("sig=%c\n", data->sig);
    printf("EVF=%s\n", ev_flag_tos(data->eval_flag));

    printf("DFE=%d\n", data->dec_float_exp);
    printf("HFE=%d\n", data->hex_float_exp);

    printf("i64=%zd\n", data->i64);
    printf("u64=%zu\n", data->u64);
    printf("f32=%f\n", data->f32);
    printf("f64=%f\n\n", data->f64);
}

static int is_int_f(int flag)
{
    return (flag == E_BIN) || (flag == E_OCT) || (flag == E_DEC) || (flag == E_HEX);
}

static int is_float_f(int flag)
{
    return (flag == E_DEC_FLOAT) || (flag == E_HEX_FLOAT);
}

static void evalhexfloat(struct cstrtox_parse_data *data);
static void evaldecfloat(struct cstrtox_parse_data *data);

static void ev(struct cstrtox_parse_data *data)
{
    assert(data);

    // validate(data)
    // checkSuffix(data);
    // checkExponent(data);

    int flag = data->eval_flag;

    if (is_int_f(flag)) {

        struct strbuilder *buf = data->dec;
        int base = 10;
        if (flag == E_BIN) {
            base = 2;
        }
        if (flag == E_OCT) {
            base = 8;
        }
        if (flag == E_HEX) {
            base = 16;
        }

        int c = sb_nextc(buf);

        long retval = 0;
        while (c != HC_FEOF && char_correct_for_base(c, base)) {
            retval = retval * base + char_value(base, c);
            c = sb_nextc(buf);
        }

        //TODO: normal routine with overflow, etc.
        data->i64 = retval;

//      if (!suf.isEmpty()) {
//        setNumtype(INT_SUFFIX_TABLE.get(suf));
//      } else {
//        setNumtype(NumType.N_INT);
//      }
    }

    if (flag == E_HEX_FLOAT) {
        evalhexfloat(data);
    }

    if (flag == E_DEC_FLOAT) {
        evaldecfloat(data);
    }

}

static void evalhexfloat(struct cstrtox_parse_data *data)
{

    struct strbuilder *decbuf = data->dec;

    double realval = 0.0;
    for (int c = sb_nextc(decbuf); c != HC_FEOF; c = sb_nextc(decbuf)) {
        realval = realval * 16.0 + (double) char_value(16, c);
    }

    if (data->mnt->len > 0) {
        struct strbuilder *mntbuf = data->mnt;

        double m = 0.0625;
        for (int c = sb_nextc(mntbuf); c != HC_FEOF; c = sb_nextc(mntbuf)) {
            realval = realval + (double) char_value(16, c) * m;
            m *= 0.0625;
        }
    }

    int div = 0;
    if (data->sig == '-') {
        div = 1;
    }

    if (data->exp->len > 0) {

        struct strbuilder *expbuf = data->exp;

        //
        int pow = 0;
        for (int c = sb_nextc(expbuf); c != HC_FEOF; c = sb_nextc(expbuf)) {
            pow = pow * 10 + (int) char_value(10, c);
        }
        //
        double m = 1.0;
        for (int i = 0; i < pow; i++) {
            m *= 2.0;
        }
        //
        if (div) {
            realval /= m;
        } else {
            realval *= m;
        }

    }

    data->f32 = (float) realval;
    data->f64 = (double) realval;

}

static void evaldecfloat(struct cstrtox_parse_data *data)
{

    struct strbuilder *decbuf = data->dec;

    double realval = 0.0;

    for (int c = sb_nextc(decbuf); c != HC_FEOF; c = sb_nextc(decbuf)) {
        realval = realval * 10.0 + (double) char_value(10, c);
    }

    if (data->mnt->len > 0) {
        struct strbuilder *mntbuf = data->mnt;

        double m = 0.1;
        for (int c = sb_nextc(mntbuf); c != HC_FEOF; c = sb_nextc(mntbuf)) {
            realval = realval + (double) char_value(10, c) * m;
            m *= 0.1;
        }
    }

    int div = 0;
    if (data->sig == '-') {
        div = 1;
    }

    if (data->exp->len > 0) {

        struct strbuilder *expbuf = data->exp;

        int pow = 0;
        for (int c = sb_nextc(expbuf); c != HC_FEOF; c = sb_nextc(expbuf)) {
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

    data->f32 = (float) realval;
    data->f64 = (double) realval;
}

static void addc_ignore_underscore(struct strbuilder *buf, char c)
{
    assert(buf && buf->str);
    if (c == '_') {
        return;
    }
    sb_addc(buf, c);
}

static int oct_only(char *where)
{
    assert(where);
    for (size_t i = 0; where[i]; i += 1) {
        if (!is_oct(where[i])) {
            return 0;
        }
    }
    return 1;
}

static int cut_exponent(int c, struct cstrtox_parse_data *data, struct strbuilder *buf)
{
    int is_dec_exp = (c == 'e' || c == 'E');
    int is_hex_exp = (c == 'p' || c == 'P');

    if (is_dec_exp || is_hex_exp) {

        if (is_dec_exp) {
            data->dec_float_exp = 1;
            data->eval_flag = E_DEC_FLOAT;
        }
        if (is_hex_exp) {
            data->hex_float_exp = 1;
            data->eval_flag = E_HEX_FLOAT;
        }

        c = sb_nextc(buf); // skip [eEpP]

        // exponent sign is optional
        if (c == '-') {
            c = sb_nextc(buf);
            data->sig = '-';
        } else if (c == '+') {
            c = sb_nextc(buf);
            data->sig = '+';
        } else {
            data->sig = '+';
        }

        // exponent digits are NOT optional

        if (!is_dec(c)) {
            cc_fatal("exponent require decimal part: %s\n", data->input);
        }

        for (; c != HC_FEOF && is_dec(c); c = sb_nextc(buf)) {
            sb_addc(data->exp, (char) c);
        }
    }

    return c;
}

struct cstrtox_parse_data* parse_again(char *what)
{
    assert(what && strlen(what));

    struct strbuilder *buf = sb_news(what);
    struct cstrtox_parse_data *rv = cstrtox_parse_data_new(what);

    int c = sb_nextc(buf);
    int nextchar = sb_peekc(buf);

    // 2
    if (c == '0' && (nextchar == 'b' || nextchar == 'B')) {

        rv->eval_flag = E_BIN;
        c = sb_nextc(buf); // skip [bB]

        for (c = sb_nextc(buf); is_bin(c) || c == '_'; c = sb_nextc(buf)) {
            addc_ignore_underscore(rv->dec, (char) c);
        }

        for (; c != HC_FEOF; c = sb_nextc(buf)) {
            sb_addc(rv->suf, (char) c);
        }

    }

    // 8
    else if (c == '0' && (nextchar == 'o' || nextchar == 'O')) {

        rv->eval_flag = E_OCT;
        c = sb_nextc(buf); // skip [oO]

        for (c = sb_nextc(buf); is_oct(c) || c == '_'; c = sb_nextc(buf)) {
            addc_ignore_underscore(rv->dec, (char) c);
        }

        for (; c != HC_FEOF; c = sb_nextc(buf)) {
            sb_addc(rv->suf, (char) c);
        }

    }

    // hex, floating hex
    // 0xABCDEF
    // 0x1.5c0c4cp+14F
    // 0X1FFFP-16
    else if (c == '0' && (nextchar == 'x' || nextchar == 'X')) {

        rv->eval_flag = E_HEX;
        c = sb_nextc(buf); // skip [xX]

        for (c = sb_nextc(buf); is_hex(c) || c == '_'; c = sb_nextc(buf)) {
            addc_ignore_underscore(rv->dec, (char) c);
        }

        // here we cut all the hex digits from the input
        // and now - we may check whether it's a hexadecimal floating literal
        // it may be with or without dot
        //

        if (c == '.') {
            rv->eval_flag = E_HEX_FLOAT;
            for (c = sb_nextc(buf); is_hex(c); c = sb_nextc(buf)) {
                sb_addc(rv->mnt, (char) c);
            }
        }

        if (c == 'p' || c == 'P') {
            c = cut_exponent(c, rv, buf);
        }

        for (; c != HC_FEOF; c = sb_nextc(buf)) {
            sb_addc(rv->suf, (char) c);
        }

    }

    // floating || exponent
    // .25
    // .12345E+5
    else if (c == '.') {

        rv->eval_flag = E_DEC_FLOAT;

        for (c = sb_nextc(buf); is_dec(c); c = sb_nextc(buf)) {
            sb_addc(rv->mnt, (char) c);
        }

        if (c == 'e' || c == 'E') {
            c = cut_exponent(c, rv, buf);
        }

        for (; c != HC_FEOF; c = sb_nextc(buf)) {
            sb_addc(rv->suf, (char) c);
        }

    }

    // octal || decimal || floating || exponent
    // 013154
    // 5740
    // 3.14
    // 072.40
    // 2.22750742e+4f
    // 0.
    // 1.f
    else if (is_dec(c)) {

        // our string does not begin with any valid prefix.
        // it must be a decimal digit then.
        // here a corner case: when it starts with a zero,
        // we will know whether it is an octal literal or floating point
        // only at the end of the stage.
        // 013154 and 072.40 are valid literals in C, for example...

        rv->eval_flag = E_DEC;
        const int begin_with_zero = (c == '0');

        for (; is_dec(c) || c == '_'; c = sb_nextc(buf)) {
            addc_ignore_underscore(rv->dec, (char) c);
        }

        if (c == '.') {
            rv->eval_flag = E_DEC_FLOAT;

            for (c = sb_nextc(buf); is_dec(c); c = sb_nextc(buf)) {
                sb_addc(rv->mnt, (char) c);
            }
        }

        if (c == 'e' || c == 'E') {
            c = cut_exponent(c, rv, buf);
        }

        for (; c != HC_FEOF; c = sb_nextc(buf)) {
            sb_addc(rv->suf, (char) c);
        }

        // here we will handle our corner case between octal literals and
        // any other we may parse.
        // if our input begins with a zero, and we have octal digits in the decimal buffer,
        // and there wasn't any dot or exponent, we may say it's an octal literal.
        if (begin_with_zero) {

            if (rv->eval_flag == E_DEC && rv->dec->len > 1 && oct_only(rv->dec->str)) {
                rv->eval_flag = E_OCT;
            }
        }

    }

    else {
        cc_fatal("unknown literal: [%s]\n", what);
    }

    ev(rv);
    return rv;

}

// tests

static int dbl_cmp(double a, double b, double epsilon)
{
    if (fabs(a - b) < epsilon) {
        return 1;
    }
    return 0;
}

static void tests()
{
    char *endptr = "";

    char *input = "0x1.22d881f6efcd3p+18";
    assert(dbl_cmp(297826.0306968216, 0x1.22d881f6efcd3p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "2.978260e+05";
    assert(dbl_cmp(297826.0306968216, 2.978260e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.19a89d4acdd43p+19";
    assert(dbl_cmp(576836.91538135, 0x1.19a89d4acdd43p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "5.768369e+05";
    assert(dbl_cmp(576836.91538135, 5.768369e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.e63674fa06bc9p+18";
    assert(dbl_cmp(497881.8277603952, 0x1.e63674fa06bc9p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "4.978818e+05";
    assert(dbl_cmp(497881.8277603952, 4.978818e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.263d9a60b48a3p+15";
    assert(dbl_cmp(37662.80151905238, 0x1.263d9a60b48a3p+15, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "3.766280e+04";
    assert(dbl_cmp(37662.80151905238, 3.766280e+04, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.2ee7cc8bc3025p+19";
    assert(dbl_cmp(620350.392060761, 0x1.2ee7cc8bc3025p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "6.203504e+05";
    assert(dbl_cmp(620350.392060761, 6.203504e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.eae4e6fbcd724p+16";
    assert(dbl_cmp(125668.90227970161, 0x1.eae4e6fbcd724p+16, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "1.256689e+05";
    assert(dbl_cmp(125668.90227970161, 1.256689e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.7abf9c6aebf0dp+18";
    assert(dbl_cmp(387838.4440259792, 0x1.7abf9c6aebf0dp+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "3.878384e+05";
    assert(dbl_cmp(387838.4440259792, 3.878384e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.9143caec3aa23p+15";
    assert(dbl_cmp(51361.896333534525, 0x1.9143caec3aa23p+15, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "5.136190e+04";
    assert(dbl_cmp(51361.896333534525, 5.136190e+04, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.75487b278d2f2p+19";
    assert(dbl_cmp(764483.848578064, 0x1.75487b278d2f2p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "7.644838e+05";
    assert(dbl_cmp(764483.848578064, 7.644838e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.e2082a154a1eap+19";
    assert(dbl_cmp(987201.3150988196, 0x1.e2082a154a1eap+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "9.872013e+05";
    assert(dbl_cmp(987201.3150988196, 9.872013e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.0c1c0939cba94p+18";
    assert(dbl_cmp(274544.1441525605, 0x1.0c1c0939cba94p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "2.745441e+05";
    assert(dbl_cmp(274544.1441525605, 2.745441e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.b9fd5d218da08p+18";
    assert(dbl_cmp(452597.45517292665, 0x1.b9fd5d218da08p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "4.525975e+05";
    assert(dbl_cmp(452597.45517292665, 4.525975e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.13bac0218e209p+19";
    assert(dbl_cmp(564694.0040960918, 0x1.13bac0218e209p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "5.646940e+05";
    assert(dbl_cmp(564694.0040960918, 5.646940e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.2fea7a7121e91p+18";
    assert(dbl_cmp(311209.9131550575, 0x1.2fea7a7121e91p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "3.112099e+05";
    assert(dbl_cmp(311209.9131550575, 3.112099e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.7c884ccd88540p+18";
    assert(dbl_cmp(389665.2000447102, 0x1.7c884ccd88540p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "3.896652e+05";
    assert(dbl_cmp(389665.2000447102, 3.896652e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.d9f9f908ed300p+19";
    assert(dbl_cmp(970703.7823396623, 0x1.d9f9f908ed300p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "9.707038e+05";
    assert(dbl_cmp(970703.7823396623, 9.707038e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.4fc33c36a0ac6p+16";
    assert(dbl_cmp(85955.23520855149, 0x1.4fc33c36a0ac6p+16, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "8.595524e+04";
    assert(dbl_cmp(85955.23520855149, 8.595524e+04, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.c7e5b78941388p+19";
    assert(dbl_cmp(933677.7355047325, 0x1.c7e5b78941388p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "9.336777e+05";
    assert(dbl_cmp(933677.7355047325, 9.336777e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.34b2036cc16d7p+16";
    assert(dbl_cmp(79026.0133782284, 0x1.34b2036cc16d7p+16, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "7.902601e+04";
    assert(dbl_cmp(79026.0133782284, 7.902601e+04, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.93723a7af3719p+17";
    assert(dbl_cmp(206564.45687716527, 0x1.93723a7af3719p+17, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "2.065645e+05";
    assert(dbl_cmp(206564.45687716527, 2.065645e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.13f5fe5f689d8p+19";
    assert(dbl_cmp(565167.9491465641, 0x1.13f5fe5f689d8p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "5.651679e+05";
    assert(dbl_cmp(565167.9491465641, 5.651679e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.bdf101cc2587ep+19";
    assert(dbl_cmp(913288.0561702398, 0x1.bdf101cc2587ep+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "9.132881e+05";
    assert(dbl_cmp(913288.0561702398, 9.132881e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.cc652130d883ap+15";
    assert(dbl_cmp(58930.56482578856, 0x1.cc652130d883ap+15, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "5.893056e+04";
    assert(dbl_cmp(58930.56482578856, 5.893056e+04, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.ae6da240545eep+18";
    assert(dbl_cmp(440758.5351763655, 0x1.ae6da240545eep+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "4.407585e+05";
    assert(dbl_cmp(440758.5351763655, 4.407585e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.6ad83d9ebff73p+12";
    assert(dbl_cmp(5805.5150439737945, 0x1.6ad83d9ebff73p+12, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "5.805515e+03";
    assert(dbl_cmp(5805.5150439737945, 5.805515e+03, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.5cf543ca70bd3p+17";
    assert(dbl_cmp(178666.52961549032, 0x1.5cf543ca70bd3p+17, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "1.786665e+05";
    assert(dbl_cmp(178666.52961549032, 1.786665e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.cd05bc61f9e57p+18";
    assert(dbl_cmp(472086.94347999006, 0x1.cd05bc61f9e57p+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "4.720869e+05";
    assert(dbl_cmp(472086.94347999006, 4.720869e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.d63eb59fe60cap+18";
    assert(dbl_cmp(481530.8378844379, 0x1.d63eb59fe60cap+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "4.815308e+05";
    assert(dbl_cmp(481530.8378844379, 4.815308e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.765399337150bp+19";
    assert(dbl_cmp(766620.7875296188, 0x1.765399337150bp+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "7.666208e+05";
    assert(dbl_cmp(766620.7875296188, 7.666208e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.2b99623129c4dp+19";
    assert(dbl_cmp(613579.0685013622, 0x1.2b99623129c4dp+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "6.135791e+05";
    assert(dbl_cmp(613579.0685013622, 6.135791e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.92a079409358ap+18";
    assert(dbl_cmp(412289.89456638007, 0x1.92a079409358ap+18, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "4.122899e+05";
    assert(dbl_cmp(412289.89456638007, 4.122899e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x1.9ab90a3d17860p+19";
    assert(dbl_cmp(841160.3199575059, 0x1.9ab90a3d17860p+19, 0.0001f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
    input = "8.411603e+05";
    assert(dbl_cmp(841160.3199575059, 8.411603e+05, 0.1f));
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

}

#if 0
int main()
{
    tests();
    char *endptr = "";

    cstrtox_print_data(parse_again("0b1111_1111_1001_0000i64"));
    cstrtox_print_data(parse_again("19122021ull"));
    cstrtox_print_data(parse_again("2.22750742e+4f"));
    cstrtox_print_data(parse_again("0x1.5c0c4cp+14F"));
    cstrtox_print_data(parse_again("0xABCDEF"));
    cstrtox_print_data(parse_again("0b00000000000000000001011001101100"));
    cstrtox_print_data(parse_again("5740"));
    cstrtox_print_data(parse_again("0x166c"));
    cstrtox_print_data(parse_again("0o13154"));
    cstrtox_print_data(parse_again("072.40"));
    cstrtox_print_data(parse_again("0X1FFFP-16"));
    cstrtox_print_data(parse_again("0x1p-2"));
    cstrtox_print_data(parse_again(".12345E+5"));
    cstrtox_print_data(parse_again("013154"));

    char *input = "2.22750742e+4f";
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = ".12345E+5";
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0X1FFFP-16";
    assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));

    input = "0x166c";
    assert(parse_again(input)->i64 == strtol(input, &endptr, 16));

    printf("\n:ok:\n");
    return 0;
}
#endif

//int main_____(void) {
//    FILE *fp;
//    char *line = NULL;
//    size_t len = 0;
//    ssize_t read;
//
//    struct strbuilder *buf = sb_new();
//
//    fp = fopen("main.c", "r");
//    if (fp == NULL)
//        exit(EXIT_FAILURE);
//
//    while ((read = getline(&line, &len, fp)) != -1) {
//
//        cstrtox_buf_adds(buf, line);
//    }
//
//    fclose(fp);
//    if (line)
//        free(line);
//
//    for (int c = cstrtox_buf_nextc(buf); c != -1; c = cstrtox_buf_nextc(buf)) {
//        printf("%c", c);
//    }
//
//    cstrtox_buf_free(&buf);
//
//    exit(EXIT_SUCCESS);
//}

//  private void setFloating(double realval) {
//
//    NumType resulttype = NumType.N_DOUBLE;
//    if (!suf.isEmpty()) {
//      if (suf.equals("f") || suf.equals("F")) {
//        resulttype = NumType.N_FLOAT;
//      } else if (suf.equals("l") || suf.equals("L")) {
//        resulttype = NumType.N_LONG_DOUBLE;
//      }
//    }
//
//    setNumtype(resulttype);
//
//    if (resulttype == NumType.N_LONG_DOUBLE || resulttype == NumType.N_DOUBLE) {
//      setCdouble(realval);
//    } else {
//      setCfloat((float) realval);
//    }
//
//  }
//  private void checkSuffix() {
//    if (isInteger()) {
//      if (!suf.isEmpty()) {
//        if (!INT_SUFFIX_TABLE.containsKey(suf)) {
//          throw new NumExc("error: incorrect suffif for decimal constant: [" + suf + "]");
//        }
//      }
//    }
//
//    if (isFloating()) {
//      if (!suf.isEmpty()) {
//        boolean correctFloatSuffix = suf.equals("f") || suf.equals("F") || suf.equals("l") || suf.equals("L");
//        if (!correctFloatSuffix) {
//          throw new NumExc("error: incorrect suffif for floating constant: [" + suf + "]");
//        }
//      }
//    }
//  }
//
//  private void checkExponent() {
//
//    if (isFloating()) {
//
//      // I)
//      if (hexFloatExp || decFloatExp) {
//        if (exp.isEmpty()) {
//          throw new NumExc("error: exponent has no digits: [" + input + "]");
//        }
//      }
//
//      // II)
//      if (flag == NumFlagEval.E_HEX_FLOAT && !hexFloatExp) {
//        throw new NumExc("error: hexadecimal floating constant require exponent: [" + input + "]");
//      }
//
//    }
//
//  }

//import random
//
//# input = ".12345E+5";
//# assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));
//
//def gen_floats():
//    for x in range(0, 32):
//        x = random.uniform(0.1, 999999.9)
//
//        exp = "{:e}".format(x)
//
//        print("input = \"" + str(float.hex(x)) + "\";")
//        print("assert(dbl_cmp(" + str(x) + ", " + str(float.hex(x)) + ", 0.0001f));")
//        print("assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));")
//
//        print("input = \"" + exp + "\";")
//        print("assert(dbl_cmp(" + str(x) + ", " + exp + ", 0.1f));")
//        print("assert(dbl_cmp(parse_again(input)->f64, strtod(input, &endptr), 0.0001f));")
//
//        print("\n")
//
//def gen_hex():
//    for x in range(0, 32):
//        x = random.randrange(0, 2147483647)
//        print(hex(x))
//
//gen_hex()
