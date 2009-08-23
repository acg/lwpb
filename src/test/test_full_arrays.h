
#include <lwpb/arch/cc.h>

/* data included from the c++ packed-data generator,
   and from the c test code. */

#define THOUSAND     1000
#define MILLION      1000000
#define BILLION      1000000000
#define TRILLION     1000000000000LL
#define QUADRILLION  1000000000000000LL
#define QUINTILLION  1000000000000000000LL

s32_t int32_arr0[2] = { -1, 1 };
s32_t int32_arr1[5] = { 42, 666, -1123123, 0, 47 };
s32_t int32_arr_min_max[2] = { S32_MIN, S32_MAX };

u32_t uint32_roundnumbers[4] = { BILLION, MILLION, 1, 0 };
u32_t uint32_0_max[2] = { U32_MIN, U32_MAX };

s64_t int64_roundnumbers[15] = { -QUINTILLION, -QUADRILLION, -TRILLION,
                                 -BILLION, -MILLION, -THOUSAND,
                                 1,
                                 THOUSAND, MILLION, BILLION,
                                 TRILLION, QUADRILLION, QUINTILLION };
s64_t int64_min_max[2] = { S64_MIN, S64_MAX };

u64_t uint64_roundnumbers[9] = { 1,
                                 THOUSAND, MILLION, BILLION,
                                 TRILLION, QUADRILLION, QUINTILLION };
u64_t uint64_0_1_max[3] = { U64_MIN, 1, U64_MAX };
u64_t uint64_random[] = { 0,
                          666,
                          4200000000ULL,
                          16ULL * (u64_t) QUINTILLION + 33 };

#define FLOATING_POINT_RANDOM \
-1000.0, -100.0, -42.0, 0, 666, 131313
float float_random[] = { FLOATING_POINT_RANDOM };
double double_random[] = { FLOATING_POINT_RANDOM };

protobuf_c_boolean boolean_0[]  = {0 };
protobuf_c_boolean boolean_1[]  = {1 };
protobuf_c_boolean boolean_random[] = {0,1,1,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,1,1,0,1,1,0 };

TEST_ENUM_SMALL_TYPE_NAME enum_small_0[] = { TEST_ENUM_SMALL(VALUE) };
TEST_ENUM_SMALL_TYPE_NAME enum_small_1[] = { TEST_ENUM_SMALL(OTHER_VALUE) };
#define T(v) (TEST_ENUM_SMALL_TYPE_NAME)(v)
TEST_ENUM_SMALL_TYPE_NAME enum_small_random[] = {T(0),T(1),T(1),T(0),T(0),T(1),T(1),T(1),T(0),T(0),T(0),T(0),T(0),T(1),T(1),T(1),T(1),T(1),T(1),T(0),T(1),T(1),T(0),T(1),T(1),T(0) };
#undef T

#define T(v) (TEST_ENUM_TYPE_NAME)(v)
TEST_ENUM_TYPE_NAME enum_0[] = { T(0) };
TEST_ENUM_TYPE_NAME enum_1[] = { T(1) };
TEST_ENUM_TYPE_NAME enum_random[] = { 
   T(0), T(268435455), T(127), T(16384), T(16383),
   T(2097152), T(2097151), T(128), T(268435456),
   T(0), T(2097152), T(268435455), T(127), T(16383), T(16384) };
#undef T

const char *repeated_strings_0[] = { "onestring" };
const char *repeated_strings_1[] = { "two", "string" };
const char *repeated_strings_2[] = { "many", "tiny", "little", "strings", "should", "be", "handled" };
const char *repeated_strings_3[] = { "one very long strings XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" };
