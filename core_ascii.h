#ifndef CORE_ASCII_H_
#define CORE_ASCII_H_

static int is_letter(int c) {
    return c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g'
            || c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n'
            || c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u'
            || c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z' || c == 'A' || c == 'B'
            || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I'
            || c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' || c == 'O' || c == 'P'
            || c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W'
            || c == 'X' || c == 'Y' || c == 'Z' || c == '_';
}

static int is_dec(int c) {
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6'
            || c == '7' || c == '8' || c == '9';
}

static int is_hex(int c) {
    return isDec(c)
            || (c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'a'
                    || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f');
}

static int is_oct(int c) {
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6'
            || c == '7';
}

static int is_bin(int c) {
    return c == '0' || c == '1';
}

static int is_op_start(int c) {
    return c == '>' || c == '<' || c == '-' || c == '|' || c == '+' || c == '&' || c == '#'
            || c == '^' || c == '=' || c == '%' || c == '/' || c == '!' || c == '*' || c == '.'
            || c == '~' || c == '}' || c == '{' || c == ')' || c == '(' || c == ']' || c == '?'
            || c == ':' || c == ';' || c == ',' || c == '[';
}

#endif /* CORE_ASCII_H_ */
