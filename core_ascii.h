#ifndef CORE_ASCII_H_
#define CORE_ASCII_H_

#include "core_mem.h"

static int is_letter(int c)
{
    return c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g'
            || c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n'
            || c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u'
            || c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z' || c == 'A' || c == 'B'
            || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I'
            || c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' || c == 'O' || c == 'P'
            || c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W'
            || c == 'X' || c == 'Y' || c == 'Z' || c == '_';
}

static int is_dec(int c)
{
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6'
            || c == '7' || c == '8' || c == '9';
}

static int is_hex(int c)
{
    return is_dec(c)
            || (c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'a'
                    || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f');
}

static int is_oct(int c)
{
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6'
            || c == '7';
}

static int is_bin(int c)
{
    return c == '0' || c == '1';
}

static int is_op_start(int c)
{
    return c == '>' || c == '<' || c == '-' || c == '|' || c == '+' || c == '&' || c == '#'
            || c == '^' || c == '=' || c == '%' || c == '/' || c == '!' || c == '*' || c == '.'
            || c == '~' || c == '}' || c == '{' || c == ')' || c == '(' || c == ']' || c == '?'
            || c == ':' || c == ';' || c == ',' || c == '[';
}

static int char_correct_for_base(int C, int base)
{
    if (base == 16 && is_hex(C)) {
        return 1;
    }
    if (base == 10 && is_dec(C)) {
        return 1;
    }
    if (base == 8 && is_oct(C)) {
        return 1;
    }
    if (base == 2 && is_bin(C)) {
        return 1;
    }
    return 0;
}

static int char_value(int base, int c)
{

    int base_in_range = (base == 2) || (base == 8) || (base == 10) || (base == 16);
    if (!base_in_range) {
        cc_fatal("error eval base = %d for char = %c\n", base, c);
    }

    if (base == 2) {
        switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        default:
            cc_fatal("error eval base = %d for char = %c\n", base, c);
        }
    }

    if (base == 8) {
        switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        default:
            cc_fatal("error eval base = %d for char = %c\n", base, c);
        }
    }

    if (base == 10) {
        switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        default:
            cc_fatal("error eval base = %d for char = %c\n", base, c);
        }
    }

    if (base == 16) {
        switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            cc_fatal("error eval base = %d for char = %c\n", base, c);
        }
    }

    cc_fatal("error eval base = %d for char = %c\n", base, c);
    return 0;
}

#endif /* CORE_ASCII_H_ */
