#include "hdrs.h"
#include "str.h"
#include "ascii.h"
#include "strtox.h"

//  private static void cutForBase(LinkedList<Character> buffer, StringBuilder out, int base) {
//
//    final boolean baseIsCorrect = base == 2 || base == 8 || base == 10 || base == 16;
//    if (!baseIsCorrect) {
//      throw new RuntimeException("it is not a correct base: " + base);
//    }
//
//    for (; !buffer.isEmpty();) {
//      final Character peek = buffer.peekFirst();
//      final boolean needBreak = (base == 2 && !isBin(peek)) || (base == 8 && !isOct(peek))
//          || (base == 10 && !isDec(peek)) || (base == 16 && !isHex(peek));
//      if (needBreak) {
//        break;
//      }
//
//      char c = buffer.removeFirst();
//      out.append(c);
//    }
//  }

static void cut_for_base(Str *buffer, Str *out, unsigned base)
{
    assert(buffer);
    assert(out);

    int base_ok = base == 2 || base == 8 || base == 10 || base == 16;
    if (!base_ok) {
        cc_fatal("it is not a correct base: %d\n", base);
    }

    while (!sb_is_empty(buffer)) {
        char peek = sb_peek_last(buffer);
        int need_break = (base == 2 && !is_bin(peek)) || (base == 8 && !is_oct(peek))
                || (base == 10 && !is_dec(peek)) || (base == 16 && !is_hex(peek));
        if (need_break) {
            break;
        }

        char c = sb_pop(buffer);
        sb_addc(out, c);
    }
}

//  private static char cutMntExp(LinkedList<Character> buffer, StringBuilder mnt, StringBuilder exp, int mntBase) {
//
//    final boolean baseIsCorrect = mntBase == 10 || mntBase == 16;
//    if (!baseIsCorrect) {
//      throw new RuntimeException("it is not a correct base for a mantissa: " + mntBase);
//    }
//
//    char exp_sign = '+';
//    if (!buffer.isEmpty() && buffer.peekFirst() == '.') {
//      buffer.removeFirst();
//      cutForBase(buffer, mnt, mntBase);
//    }
//
//    if (!buffer.isEmpty()) {
//
//      final boolean isHexExp = buffer.peekFirst() == 'p' || buffer.peekFirst() == 'P';
//      final boolean isDecExp = buffer.peekFirst() == 'e' || buffer.peekFirst() == 'E';
//
//      if (isHexExp && mntBase != 16) {
//        throw new RuntimeException("a hex exponent part in a floating constant.");
//      }
//
//      if (isHexExp || isDecExp) {
//        buffer.removeFirst();
//        if (buffer.peekFirst() == '-' || buffer.peekFirst() == '+') {
//          exp_sign = buffer.removeFirst();
//        }
//        cutForBase(buffer, exp, 10);
//      }
//    }
//    return exp_sign;
//  }

static char cut_mnt_exp(Str *buffer, Str *mnt, Str *exp, unsigned mnt_base)
{

    assert(buffer);
    assert(mnt);
    assert(exp);

    int base_ok = mnt_base == 10 || mnt_base == 16;
    if (!base_ok) {
        cc_fatal("it is not a correct base for a mantissa: %d\n", mnt_base);
    }

    char exp_sign = '+';
    if (!sb_is_empty(buffer) && sb_peek_last(buffer) == '.') {
        sb_pop(buffer);
        cut_for_base(buffer, mnt, mnt_base);
    }

    if (!sb_is_empty(buffer)) {

        int peek = sb_peek_last(buffer);
        int is_hex_exp = peek == 'p' || peek == 'P';
        int is_dec_exp = peek == 'e' || peek == 'E';

        if (is_hex_exp && mnt_base != 16) {
            cc_fatal("a hex exponent part in a floating constant.");
        }

        if (is_hex_exp || is_dec_exp) {
            sb_pop(buffer);
            peek = sb_peek_last(buffer);
            if (peek == '-' || peek == '+') {
                exp_sign = sb_pop(buffer);
            }
            cut_for_base(buffer, exp, 10);
        }
    }

    return exp_sign;
}

static int evaltype_by_mnt_exp(Str *mnt, Str *exp)
{
    if (mnt->len || exp->len) {
        return EVALTYPE_FLOATING;
    }
    return EVALTYPE_INTEGER;
}

Strtox *parse_number(char *n)
{
    assert(n);
    size_t len = strlen(n);
    assert(len && "an empty input data");

    Str input = STR_INIT;
    sb_adds_rev(&input, n);

    Str dec = STR_INIT;
    Str mnt = STR_INIT;
    Str exp = STR_INIT;
    Str suf = STR_INIT;

    char main_sign = '+';
    if (sb_peek_last(&input) == '+' || sb_peek_last(&input) == '-') {
        main_sign = sb_pop(&input);
    }

    char exp_sign = '+';
    int isBin = 0;
    int isOct = 0;
    int isHex = 0;

    int evaltype = EVALTYPE_ERROR;

    // NOTE: we're dealing with reversed string
    if (input.len > 2) {
        // 0x12 : 21x0
        char c1 = sb_char_at(&input, input.len - 1); // 0
        char c2 = sb_char_at(&input, input.len - 2); // x
        if (c1 == '0' && (c2 == 'b' || c2 == 'B')) {
            isBin = 1;
        }
        if ((c1 == '0' && (c2 == 'o' || c2 == 'O')) || (c1 == '0' && is_oct(c2))) {
            isOct = 1;
        }
        if (c1 == '0' && (c2 == 'x' || c2 == 'X')) {
            isHex = 1;
        }
    }

    if (isBin || isOct || isHex) {
        if (isOct) {
            sb_pop(&input);
            if (sb_peek_last(&input) == 'o' || sb_peek_last(&input) == 'O') {
                sb_pop(&input);
            }
        } else {
            sb_pop(&input);
            sb_pop(&input);
        }

        // we'he checked that the buffer.size() > 2,
        // and now we know, that there's something here.

        if (isBin) {
            cut_for_base(&input, &dec, 2);
        }

        if (isOct) {
            cut_for_base(&input, &dec, 8);
        }

        if (isHex) {
            cut_for_base(&input, &dec, 16);
            exp_sign = cut_mnt_exp(&input, &mnt, &exp, 16);
            evaltype = evaltype_by_mnt_exp(&mnt, &exp);
        }

    }

    else {

        if (sb_peek_last(&input) == '.') {
            exp_sign = cut_mnt_exp(&input, &mnt, &exp, 10);
            evaltype = evaltype_by_mnt_exp(&mnt, &exp);
        }

        else {

            // parse decimal|floating|floating_exponent

            if (!is_dec(sb_peek_last(&input))) {
                cc_fatal("not a number: %s\n", n);
            }

            cut_for_base(&input, &dec, 10);
            exp_sign = cut_mnt_exp(&input, &mnt, &exp, 10);
            evaltype = evaltype_by_mnt_exp(&mnt, &exp);
        }

    }

    while (!sb_is_empty(&input)) {
        sb_addc(&suf, sb_pop(&input));
    }

    int evalbase = EVALBASE_ERROR;
    if (isBin) {
        evalbase = EVALBASE_2;
    } else if (isOct) {
        evalbase = EVALBASE_8;
    } else if (isHex) {
        evalbase = EVALBASE_16;
    } else {
        evalbase = EVALBASE_10;
    }

    assert(evaltype != EVALTYPE_ERROR);

    Strtox *result = cc_malloc(sizeof(struct strtox));
    result->evalbase = evalbase;
    result->evaltype = evaltype;

    result->main_sign = main_sign;
    result->dec = sb_buf_or_empty(&dec);
    result->mnt = sb_buf_or_empty(&mnt);
    result->exp = sb_buf_or_empty(&exp);
    result->exp_sign = exp_sign;
    result->suf = sb_buf_or_empty(&suf);
    return result;
}
