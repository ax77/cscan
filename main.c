#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#include "core_list.h"
#include "core_buf.h"
#include "token.h"
#include "hashmap.h"
#include "ident_hash.h"
#include "core_strutil.h"
#include "core_file.h"
#include "core_mem.h"

HashMap *VALID_COMBINATIONS;

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

int streq(char *s1, char *s2) {
	return strcmp(s1, s2) == 0;
}

void test_pathnormalize_1() {
	struct strbuilder *s = pathnormalize("usr//local//include");
	//printf("%s\n", s->str);
	axTEST(streq(s->str, "usr/local/include"));
}

void test_pathnormalize_2() {
	struct strbuilder *s1 = pathnormalize(
			"//\\usr//\\//\\local//\\//\\include\\//\\//");
	char *s2 = "/usr/local/include/";
	axTEST(strlen(s1->str) == strlen(s2));
}

void test_pathnormalize_3() {
	struct strbuilder *s1 = pathnormalize(
			"//\\usr//\\//\\local//\\//\\include\\//\\//string.h");
	char *s2 = "/usr/local/include/string.h";
	axTEST(strlen(s1->str) == strlen(s2));
}

void test_pathnormalize_4() {
	struct strbuilder *s1 = pathnormalize("");
	char *s2 = "./";
	axTEST(strlen(s1->str) == strlen(s2));
}

void test_strleft_0() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "1024");

	struct strbuilder *res = sb_left(buf, 2);
	axTEST(streq(res->str, "10"));
}

void test_strleft_1() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "1024");

	struct strbuilder *res = sb_left(buf, 200);
	axTEST(streq(res->str, "1024"));
}

void test_strleft_2() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "");

	struct strbuilder *res = sb_left(buf, 0);
	axTEST(streq(res->str, ""));
}

void test_strright_0() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "1024");

	struct strbuilder *res = sb_right(buf, 2);
	axTEST(streq(res->str, "24"));
}

void test_strright_1() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "1024");

	struct strbuilder *res = sb_right(buf, 2000);
	axTEST(streq(res->str, "1024"));
}

void test_strmid_0() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_mid(buf, 0, 3);
	axTEST(streq(res->str, "str"));
}

void test_strmid_1() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_mid(buf, 3, 300);
	axTEST(streq(res->str, "ing_1024"));
}

void test_strmid_2() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_mid(buf, 3, 3);
	axTEST(streq(res->str, "ing"));
}

void test_strmid_3() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_mid(buf, 3, 8);
	axTEST(streq(res->str, "ing_1024"));
}

void test_strmid_4() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_mid(buf, 3, 9);
	axTEST(streq(res->str, "ing_1024"));
}

void test_strmid_5() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_mid(buf, 3, 7);
	axTEST(streq(res->str, "ing_102"));
}

void test_strtrim_0() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "\n\n  string_1024 \n\n  \n\n\t \r\n\n ");

	struct strbuilder *res = sb_trim(buf);
	axTEST(streq(res->str, "string_1024"));
}

void test_strtrim_1() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_trim(buf);
	axTEST(streq(res->str, "string_1024"));
	//printf("[%s]\n", res->str);
}

void test_replace_0() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "string_1024");

	struct strbuilder *res = sb_replace(buf, "0", "abc");
	printf("%s\n", res->str);
}

void test_replace_1() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "a b c d e f");

	struct strbuilder *res = sb_replace(buf, " ", "/");
	printf("%s\n", res->str);
}

void test_replace_2() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "this is the string...this is another string...");

	struct strbuilder *res = sb_replace(buf, "s ", "@ ");
	printf("%s\n", res->str);
}

void test_replace_3() {
	struct strbuilder *buf = sb_new();
	sb_adds(buf, "test replace all a");

	struct strbuilder *res = sb_replace(buf, "a", "/");
	printf("%s\n", res->str);
}

int main(void) {
	test_replace_0();
	test_replace_1();
	test_replace_2();
	test_replace_3();

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
	test_pathnormalize_1();
	test_pathnormalize_2();
	test_pathnormalize_3();
	test_pathnormalize_4();
	test_strleft_0();
	test_strleft_1();
	test_strleft_2();
	test_strright_0();
	test_strright_1();
	test_strmid_0();
	test_strmid_1();
	test_strmid_2();
	test_strmid_3();
	test_strmid_4();
	test_strmid_5();
	test_strtrim_0();
	test_strtrim_1();

	printf("\n:ok:\n");
	return 0;
}

