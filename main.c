#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include "core_buf.h"
#include "token.h"
#include "hashmap.h"
#include "ident_hash.h"
#include "core_strutil.h"

HashMap *VALID_COMBINATIONS;

char* readfile(const char *filename, size_t *szout) {
	FILE *fp = NULL;
	size_t n, sz;

	char *data = NULL;
	fp = fopen(filename, "rb");
	assert(fp && "file does not exists.");

	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);

	*szout = sz;

	data = malloc(sz + 1);
	assert(data && "malloc fail");

	data[sz] = '\0';
	n = fread(data, 1, sz, fp);
	if (n != sz) {
		goto fail;
	}

	fclose(fp);
	return data;

	fail: if (fp) {
		fclose(fp);
	}
	if (data) {
		free(data);
	}

	assert(0);
	return NULL;
}

#define axTEST(expr) do {\
  if( !(expr) ) {\
    fprintf(stderr, "test fail: (%s:%s():%d) : [%s]\n" \
    , __FILE__, __func__, __LINE__, #expr);\
    exit(128);\
  }\
}while(0)

void test_strstarts_1() {
	char *what = "1";
	char *with = "1";
	axTEST(strstarts(what, with));
}

void test_strstarts_2() {
	char *what = "";
	char *with = " ";
	axTEST(!strstarts(what, with));
}

void test_strstarts_3() {
	char *what = "12345";
	char *with = "1234.";
	axTEST(!strstarts(what, with));
}

void test_strstarts_4() {
	char *what = "12345 ";
	char *with = "12345 ";
	axTEST(strstarts(what, with));
}

void test_strstarts_5() {
	axTEST(strstarts("usr/local/include/", "usr"));
	axTEST(strstarts("usr/local/include/", "usr/local"));
	axTEST(strstarts("usr/local/include/", "usr/local/"));
}

void test_strends_1() {
	char *what = "12345";
	char *with = "45";
	axTEST(strends(what, with));
}

void test_strends_2() {
	char *what = "12345";
	char *with = " 45";
	axTEST(!strends(what, with));
}

void test_strends_3() {
	char *what = "12345";
	char *with = "12345";
	axTEST(strends(what, with));
}

void test_strends_4() {
	char *what = "test/include";
	char *with = "include";
	axTEST(strends(what, with));
}

void test_strends_5() {
	axTEST(strends("test/include", "include"));
	axTEST(strends("test/include", "/include"));
	axTEST(strends("test/include", "t/include"));
}

int main(void) {

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

	size_t s= 0;
	char *source = readfile("token.c", &s);

	struct ccbuf *buf = ccbuf_new(source);
	for(int c = nextc(buf); c != HC_FEOF; c = nextc(buf)) {
		//printf("%c", c);
	}

//	VALID_COMBINATIONS = HashMap_new_str();
//
//	init_identifiers_bt();
//	init_tokens_bt();
//
//	HashMap_add(VALID_COMBINATIONS, "->", T_ARROW);
//	HashMap_add(VALID_COMBINATIONS, "--", T_MINUS_MINUS);
//	HashMap_add(VALID_COMBINATIONS, "-=", T_MINUS_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "!=", T_NE);
//	HashMap_add(VALID_COMBINATIONS, "..", T_DOT_DOT);
//	HashMap_add(VALID_COMBINATIONS, "*=", T_TIMES_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "/=", T_DIVIDE_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "&=", T_AND_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "&&", T_AND_AND);
//	HashMap_add(VALID_COMBINATIONS, "##", T_SHARP_SHARP);
//	HashMap_add(VALID_COMBINATIONS, "%=", T_PERCENT_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "^=", T_XOR_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "++", T_PLUS_PLUS);
//	HashMap_add(VALID_COMBINATIONS, "+=", T_PLUS_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "<=", T_LE);
//	HashMap_add(VALID_COMBINATIONS, "<<", T_LSHIFT);
//	HashMap_add(VALID_COMBINATIONS, "==", T_EQ);
//	HashMap_add(VALID_COMBINATIONS, ">=", T_GE);
//	HashMap_add(VALID_COMBINATIONS, ">>", T_RSHIFT);
//	HashMap_add(VALID_COMBINATIONS, "||", T_OR_OR);
//	HashMap_add(VALID_COMBINATIONS, "|=", T_OR_EQUAL);
//	HashMap_add(VALID_COMBINATIONS, "<:", T_LEFT_BRACKET);
//	HashMap_add(VALID_COMBINATIONS, ":>", T_RIGHT_BRACKET);
//	HashMap_add(VALID_COMBINATIONS, "<%", T_LEFT_BRACE);
//	HashMap_add(VALID_COMBINATIONS, "%>", T_RIGHT_BRACE);
//	HashMap_add(VALID_COMBINATIONS, "%:", T_SHARP);
//
//	struct ident *id = get_hashed_ident("inline", 0);
//	printf("%s\n", id->name);
//	assert(id == inline_ident);

	printf("\n:ok:\n");
	return 0;
}

