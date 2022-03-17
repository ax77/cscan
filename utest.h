/*
 * core_unittests.h
 *
 *  Created on: 20 дек. 2021 г.
 *      Author: Alexey
 */

#ifndef UTEST_H_
#define UTEST_H_

#include <stdio.h>
#include <stdlib.h>

#define assert_true(expr) do {\
  if( !(expr) ) {\
    fprintf(stderr, "test fail: (%s:%s():%d) : [%s]\n" \
    , __FILE__, __func__, __LINE__, #expr);\
    exit(128);\
  }\
}while(0)

#endif /* UTEST_H_ */
