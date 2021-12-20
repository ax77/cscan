/*
 * core_unittests.h
 *
 *  Created on: 20 дек. 2021 г.
 *      Author: Alexey
 */

#ifndef CORE_UNITTESTS_H_
#define CORE_UNITTESTS_H_

#include <stdio.h>
#include <stdlib.h>

#define cc_assert_true(expr) do {\
  if( !(expr) ) {\
    fprintf(stderr, "test fail: (%s:%s():%d) : [%s]\n" \
    , __FILE__, __func__, __LINE__, #expr);\
    exit(128);\
  }\
}while(0)

#endif /* CORE_UNITTESTS_H_ */
