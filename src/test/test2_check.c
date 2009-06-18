#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lwpb/lwpb.h>

#include "generated/test2_pb2.h"
#include "generated/test2_vectors.inc"


#define protobuf_c_boolean int
#define TEST_ENUM_SMALL_TYPE_NAME  int
#define TEST_ENUM_SMALL(NAME)      FOO_##NAME
#define TEST_ENUM_TYPE_NAME        int
#define TEST_ENUM(NAME)            FOO_##NAME


#include "test2_test_arrays.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

static int verbose = 0;

/** Checks an lwpb return code. */
#define CHECK_LWPB(err)                                                     \
    do {                                                                    \
        if (verbose)                                                        \
            printf("checking lwpb result\n");                               \
        if ((err) != LWPB_ERR_OK) {                                         \
            printf("%s [%d]: lwpb function returned with: %d (%s)\n",       \
                   __FILE__, __LINE__, err, lwpb_err_text(err));            \
            abort();                                                        \
        }                                                                   \
    } while (0)

/** Returns !0 if buffers are equal. */
static int buf_equal(const uint8_t *buf1, size_t len1,
                     const uint8_t *buf2, size_t len2)
{
    if (len1 != len2)
        return 0;
    
    return memcmp(buf1, buf2, len1) == 0;
}

/** Dumps a buffer. */
static void dump_buf(const uint8_t *buf, size_t len)
{
    size_t i;
    
    for (i = 0; i < len; i++)
        printf(" %02x", buf[i]);
    printf("\n");
}

/** Checks a buffer vs. it's static test vector. */
static void check_buf(const uint8_t *actual_data,
                      size_t actual_len,
                      const uint8_t *expected_data,
                      size_t expected_len,
                      const char *static_buf_name,
                      const char *filename,
                      unsigned lineno)
{
    if (verbose)
        printf("checking buffer\n");
    
    if (buf_equal(actual_data, actual_len, expected_data, expected_len))
        return;
    
    printf("%s [%d]: buffer is not as expected\n", filename, lineno);
    printf("actual (length = %u):\n", actual_len);
    dump_buf(actual_data, actual_len);
    printf("expected (length = %u) (%s):\n", expected_len, static_buf_name);
    dump_buf(expected_data, expected_len);
    abort();
}

#define CHECK_BUF(buf, len, vector) \
    check_buf(buf, len, vector, sizeof(vector), #vector, __FILE__, __LINE__)


static void check_value(uint64_t actual_value,
                        uint64_t expected_value,
                        const char *filename,
                        unsigned lineno)
{
    if (verbose)
        printf("checking value\n");
    
    if (actual_value == expected_value)
        return;
    
    printf("%s [%d]: value not as expected (actual = %lld expected = %lld)\n",
           filename, lineno, actual_value, expected_value);
    abort();
}

#define CHECK_VALUE(actual_value, expected_value) \
    check_value((int64_t) (actual_value), (int64_t) (expected_value), \
                __FILE__, __LINE__)

static void check_fvalue(double actual_value,
                         double expected_value,
                         const char *filename,
                         unsigned lineno)
{
    if (verbose)
        printf("checking floating point value\n");
    
    if (actual_value == expected_value)
        return;
    
    printf("%s [%d]: value not as expected (actual = %f expected = %f)\n",
           filename, lineno, actual_value, expected_value);
    abort();
}

#define CHECK_FVALUE(actual_value, expected_value) \
    check_fvalue((actual_value), (expected_value), __FILE__, __LINE__)


static void check_string(const char *actual_string, size_t actual_len,
                         const char *expected_string,
                         const char *filename, unsigned lineno)
{
    if (verbose)
        printf("checking string\n");
    
    if (actual_len == strlen(expected_string))
        if (memcmp(actual_string, expected_string, actual_len) == 0)
            return;
    
    printf("%s [%d]: string value not as expected\n", filename, lineno);
    printf("actual (length = %u):\n", actual_len);
    while (actual_len--)
        printf("%c", *actual_string++);
    printf("\n");
    printf("expected (length = %u):\n", strlen(expected_string));
    printf("%s\n", expected_string);
    abort();
}

#define CHECK_STRING(actual_string, actual_len, expected_string) \
    check_string(actual_string, actual_len, expected_string, __FILE__, __LINE__)

static void do_assert(const char *msg,
                      const char *filename, unsigned lineno)
{
    printf("%s [%d]: %s\n", filename, lineno, msg);
    abort();
}

#define CHECK_ASSERT(expr, msg)                                             \
    do {                                                                    \
        if (!(expr))                                                        \
            do_assert(msg, __FILE__, __LINE__);                             \
    } while(0)

/** Array holding expected field values when decoding. */
struct testing_fields {
    union {
        struct { lwpb_enum_t test; } TestMessRequiredEnumSmall;
        struct { lwpb_enum_t test; } TestMessRequiredEnum;
        struct { char *test; } TestFieldNo;
        struct { int32_t test; } TestMessRequiredInt32;
        struct { int32_t test; } TestMessRequiredSInt32;
        struct { int32_t test; } TestMessRequiredSFixed32;
        struct { uint32_t test; } TestMessRequiredUInt32;
        struct { uint32_t test; } TestMessRequiredFixed32;
        struct { int64_t test; } TestMessRequiredInt64;
        struct { int64_t test; } TestMessRequiredSInt64;
        struct { int64_t test; } TestMessRequiredSFixed64;
        struct { uint64_t test; } TestMessRequiredUInt64;
        struct { uint64_t test; } TestMessRequiredFixed64;
        struct { float test; } TestMessRequiredFloat;
        struct { double test; } TestMessRequiredDouble;
        struct { lwpb_bool_t test; } TestMessRequiredBool;
        struct { char *test; } TestMessRequiredString;
        struct { uint8_t *test; } TestMessRequiredBytes;
        struct {
            int32_t test_int32;
            int32_t test_sint32;
            int32_t test_sfixed32;
            uint32_t test_uint32;
            uint32_t test_fixed32;
            int64_t test_int64;
            int64_t test_sint64;
            int64_t test_sfixed64;
            uint64_t test_uint64;
            uint64_t test_fixed64;
            float test_float;
            double test_double;
            lwpb_bool_t test_boolean;
            lwpb_enum_t test_enum_small;
            lwpb_enum_t test_enum;
            char *test_string;
            uint8_t *test_bytes;
        } TestMessOptional;
        struct {
            int32_t *test_int32;
            int32_t *test_sint32;
            int32_t *test_sfixed32;
            uint32_t *test_uint32;
            uint32_t *test_fixed32;
            int64_t *test_int64;
            int64_t *test_sint64;
            int64_t *test_sfixed64;
            uint64_t *test_uint64;
            uint64_t *test_fixed64;
            float *test_float;
            double *test_double;
            lwpb_bool_t *test_boolean;
            lwpb_enum_t *test_enum_small;
            lwpb_enum_t *test_enum;
            char **test_string;
            uint8_t *test_bytes;
        } TestMess;
    } u;
};

static void generic_field_handler(struct lwpb_decoder *decoder,
                                  const struct lwpb_msg_desc *msg_desc,
                                  const struct lwpb_field_desc *field_desc,
                                  union lwpb_value *value, void *arg)
{
    struct testing_fields *fields = decoder->arg;
    
    if (msg_desc == foo_TestMessRequiredEnumSmall) {
        if (field_desc == foo_TestMessRequiredEnumSmall_test) {
            CHECK_VALUE(value->enum_, fields->u.TestMessRequiredEnumSmall.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredEnum) {
        if (field_desc == foo_TestMessRequiredEnum_test) {
            CHECK_VALUE(value->enum_, fields->u.TestMessRequiredEnum.test); return;
        }
    } else if (msg_desc == foo_TestFieldNo15 ||
               msg_desc == foo_TestFieldNo16 ||
               msg_desc == foo_TestFieldNo2047 ||
               msg_desc == foo_TestFieldNo2048 ||
               msg_desc == foo_TestFieldNo262143 ||
               msg_desc == foo_TestFieldNo262144 ||
               msg_desc == foo_TestFieldNo33554431 ||
               msg_desc == foo_TestFieldNo33554432) {
        if (field_desc == foo_TestFieldNo15_test ||
            field_desc == foo_TestFieldNo16_test ||
            field_desc == foo_TestFieldNo2047_test ||
            field_desc == foo_TestFieldNo2048_test ||
            field_desc == foo_TestFieldNo262143_test ||
            field_desc == foo_TestFieldNo262144_test ||
            field_desc == foo_TestFieldNo33554431_test ||
            field_desc == foo_TestFieldNo33554432_test) {
            CHECK_STRING(value->string.str, value->string.len, fields->u.TestFieldNo.test);
            return;
        }
    } else if (msg_desc == foo_TestMessRequiredInt32) {
        if (field_desc == foo_TestMessRequiredInt32_test) {
            CHECK_VALUE(value->int32, fields->u.TestMessRequiredInt32.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredSInt32) {
        if (field_desc == foo_TestMessRequiredSInt32_test) {
            CHECK_VALUE(value->int32, fields->u.TestMessRequiredSInt32.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredSFixed32) {
        if (field_desc == foo_TestMessRequiredSFixed32_test) {
            CHECK_VALUE(value->int32, fields->u.TestMessRequiredSFixed32.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredUInt32) {
        if (field_desc == foo_TestMessRequiredUInt32_test) {
            CHECK_VALUE(value->uint32, fields->u.TestMessRequiredUInt32.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredFixed32) {
        if (field_desc == foo_TestMessRequiredFixed32_test) {
            CHECK_VALUE(value->uint32, fields->u.TestMessRequiredFixed32.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredInt64) {
        if (field_desc == foo_TestMessRequiredInt64_test) {
            CHECK_VALUE(value->int64, fields->u.TestMessRequiredInt64.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredSInt64) {
        if (field_desc == foo_TestMessRequiredSInt64_test) {
            CHECK_VALUE(value->int64, fields->u.TestMessRequiredSInt64.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredSFixed64) {
        if (field_desc == foo_TestMessRequiredSFixed64_test) {
            CHECK_VALUE(value->int64, fields->u.TestMessRequiredSFixed64.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredUInt64) {
        if (field_desc == foo_TestMessRequiredUInt64_test) {
            CHECK_VALUE(value->uint64, fields->u.TestMessRequiredUInt64.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredFixed64) {
        if (field_desc == foo_TestMessRequiredFixed64_test) {
            CHECK_VALUE(value->uint64, fields->u.TestMessRequiredFixed64.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredFloat) {
        if (field_desc == foo_TestMessRequiredFloat_test) {
            CHECK_FVALUE(value->float_, fields->u.TestMessRequiredFloat.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredDouble) {
        if (field_desc == foo_TestMessRequiredDouble_test) {
            CHECK_FVALUE(value->double_, fields->u.TestMessRequiredDouble.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredBool) {
        if (field_desc == foo_TestMessRequiredBool_test) {
            CHECK_VALUE(value->bool, fields->u.TestMessRequiredBool.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredString) {
        if (field_desc == foo_TestMessRequiredString_test) {
            CHECK_STRING(value->string.str, value->string.len, fields->u.TestMessRequiredString.test); return;
        }
    } else if (msg_desc == foo_TestMessRequiredBytes) {
        if (field_desc == foo_TestMessRequiredBytes_test) {
            CHECK_STRING(value->string.str, value->string.len, fields->u.TestMessRequiredBytes.test); return;
        }
    } else if (msg_desc == foo_TestMessOptional) {
        if (field_desc == foo_TestMessOptional_test_int32) {
            CHECK_VALUE(value->int32, fields->u.TestMessOptional.test_int32); return;
        } else if (field_desc == foo_TestMessOptional_test_sint32) {
            CHECK_VALUE(value->int32, fields->u.TestMessOptional.test_sint32); return;
        } else if (field_desc == foo_TestMessOptional_test_sfixed32) {
            CHECK_VALUE(value->int32, fields->u.TestMessOptional.test_sfixed32); return;
        } else if (field_desc == foo_TestMessOptional_test_uint32) {
            CHECK_VALUE(value->uint32, fields->u.TestMessOptional.test_uint32); return;
        } else if (field_desc == foo_TestMessOptional_test_fixed32) {
            CHECK_VALUE(value->uint32, fields->u.TestMessOptional.test_fixed32); return;
        } else if (field_desc == foo_TestMessOptional_test_int64) {
            CHECK_VALUE(value->int64, fields->u.TestMessOptional.test_int64); return;
        } else if (field_desc == foo_TestMessOptional_test_sint64) {
            CHECK_VALUE(value->int64, fields->u.TestMessOptional.test_sint64); return;
        } else if (field_desc == foo_TestMessOptional_test_sfixed64) {
            CHECK_VALUE(value->int64, fields->u.TestMessOptional.test_sfixed64); return;
        } else if (field_desc == foo_TestMessOptional_test_uint64) {
            CHECK_VALUE(value->uint64, fields->u.TestMessOptional.test_uint64); return;
        } else if (field_desc == foo_TestMessOptional_test_fixed64) {
            CHECK_VALUE(value->uint64, fields->u.TestMessOptional.test_fixed64); return;
        } else if (field_desc == foo_TestMessOptional_test_float) {
            CHECK_FVALUE(value->float_, fields->u.TestMessOptional.test_float); return;
        } else if (field_desc == foo_TestMessOptional_test_double) {
            CHECK_FVALUE(value->double_, fields->u.TestMessOptional.test_double); return;
        } else if (field_desc == foo_TestMessOptional_test_boolean) {
            CHECK_VALUE(value->bool, fields->u.TestMessOptional.test_boolean); return;
        } else if (field_desc == foo_TestMessOptional_test_enum_small) {
            CHECK_VALUE(value->enum_, fields->u.TestMessOptional.test_enum_small); return;
        } else if (field_desc == foo_TestMessOptional_test_enum) {
            CHECK_VALUE(value->enum_, fields->u.TestMessOptional.test_enum); return;
        } else if (field_desc == foo_TestMessOptional_test_string) {
            CHECK_STRING(value->string.str, value->string.len, fields->u.TestMessOptional.test_string); return;
        }
    } else if (msg_desc == foo_TestMess) {
        if (field_desc == foo_TestMess_test_int32) {
            CHECK_VALUE(value->int32, *fields->u.TestMess.test_int32++); return;
        } else if (field_desc == foo_TestMess_test_sint32) {
            CHECK_VALUE(value->int32, *fields->u.TestMess.test_sint32++); return;
        } else if (field_desc == foo_TestMess_test_sfixed32) {
            CHECK_VALUE(value->int32, *fields->u.TestMess.test_sfixed32++); return;
        } else if (field_desc == foo_TestMess_test_uint32) {
            CHECK_VALUE(value->uint32, *fields->u.TestMess.test_uint32++); return;
        } else if (field_desc == foo_TestMess_test_fixed32) {
            CHECK_VALUE(value->uint32, *fields->u.TestMess.test_fixed32++); return;
        } else if (field_desc == foo_TestMess_test_int64) {
            CHECK_VALUE(value->int64, *fields->u.TestMess.test_int64++); return;
        } else if (field_desc == foo_TestMess_test_sint64) {
            CHECK_VALUE(value->int64, *fields->u.TestMess.test_sint64++); return;
        } else if (field_desc == foo_TestMess_test_sfixed64) {
            CHECK_VALUE(value->int64, *fields->u.TestMess.test_sfixed64++); return;
        } else if (field_desc == foo_TestMess_test_uint64) {
            CHECK_VALUE(value->uint64, *fields->u.TestMess.test_uint64++); return;
        } else if (field_desc == foo_TestMess_test_fixed64) {
            CHECK_VALUE(value->uint64, *fields->u.TestMess.test_fixed64++); return;
        } else if (field_desc == foo_TestMess_test_float) {
            CHECK_FVALUE(value->float_, *fields->u.TestMess.test_float++); return;
        } else if (field_desc == foo_TestMess_test_double) {
            CHECK_FVALUE(value->double_, *fields->u.TestMess.test_double++); return;
        } else if (field_desc == foo_TestMess_test_boolean) {
            CHECK_VALUE(value->bool, *fields->u.TestMess.test_boolean++); return;
        } else if (field_desc == foo_TestMess_test_enum_small) {
            CHECK_VALUE(value->enum_, *fields->u.TestMess.test_enum_small++); return;
        } else if (field_desc == foo_TestMess_test_enum) {
            CHECK_VALUE(value->enum_, *fields->u.TestMess.test_enum++); return;
        } else if (field_desc == foo_TestMess_test_string) {
            CHECK_STRING(value->string.str, value->string.len, *fields->u.TestMess.test_string++); return;
        }
    }
    
    printf("Decoded unhandled field\n");
    abort();
}

static void test_enum_small(void)
{
#define DO_TEST(value)                                                      \
    do {                                                                    \
        lwpb_err_t ret;                                                     \
        struct lwpb_encoder encoder;                                        \
        struct lwpb_decoder decoder;                                        \
        struct testing_fields fields;                                       \
        uint8_t buf[128];                                                   \
        size_t len;                                                         \
        lwpb_encoder_init(&encoder);                                        \
        lwpb_encoder_start(&encoder, foo_TestMessRequiredEnumSmall, buf, sizeof(buf)); \
        ret = lwpb_encoder_add_enum(&encoder, foo_TestMessRequiredEnumSmall_test, FOO_##value); \
        CHECK_LWPB(ret);                                                    \
        len = lwpb_encoder_finish(&encoder);                                \
        CHECK_BUF(buf, len, test_enum_small_##value);                       \
        fields.u.TestMessRequiredEnumSmall.test = FOO_##value;              \
        lwpb_decoder_init(&decoder);                                        \
        lwpb_decoder_arg(&decoder, &fields);                                \
        lwpb_decoder_field_handler(&decoder, generic_field_handler);        \
        ret = lwpb_decoder_decode(&decoder, foo_TestMessRequiredEnumSmall, buf, len); \
        CHECK_LWPB(ret);                                                    \
    } while (0);
    
    DO_TEST(VALUE);
    DO_TEST(OTHER_VALUE);
    
#undef DO_TEST
}

static void test_enum_big(void)
{
#define DO_TEST(value)                                                      \
    do {                                                                    \
        lwpb_err_t ret;                                                     \
        struct lwpb_encoder encoder;                                        \
        struct lwpb_decoder decoder;                                        \
        struct testing_fields fields;                                       \
        uint8_t buf[128];                                                   \
        size_t len;                                                         \
        lwpb_encoder_init(&encoder);                                        \
        lwpb_encoder_start(&encoder, foo_TestMessRequiredEnum, buf, sizeof(buf)); \
        ret = lwpb_encoder_add_enum(&encoder, foo_TestMessRequiredEnum_test, FOO_##value); \
        CHECK_LWPB(ret);                                                    \
        len = lwpb_encoder_finish(&encoder);                                \
        CHECK_BUF(buf, len, test_enum_big_##value);                         \
        fields.u.TestMessRequiredEnum.test = FOO_##value;                   \
        lwpb_decoder_init(&decoder);                                        \
        lwpb_decoder_arg(&decoder, &fields);                                \
        lwpb_decoder_field_handler(&decoder, generic_field_handler);        \
        ret = lwpb_decoder_decode(&decoder, foo_TestMessRequiredEnumSmall, buf, len); \
        CHECK_LWPB(ret);                                                    \
    } while (0);
    
    DO_TEST(VALUE0);
    DO_TEST(VALUE127);
    DO_TEST(VALUE128);
    DO_TEST(VALUE16383);
    DO_TEST(VALUE16384);
    DO_TEST(VALUE2097151);
    DO_TEST(VALUE2097152);
    DO_TEST(VALUE268435455);
    DO_TEST(VALUE268435456);
    
#undef DO_TEST
}

static void test_field_numbers(void)
{
#define DO_TEST(field)                                                      \
    do {                                                                    \
        lwpb_err_t ret;                                                     \
        struct lwpb_encoder encoder;                                        \
        struct lwpb_decoder decoder;                                        \
        struct testing_fields fields;                                       \
        uint8_t buf[128];                                                   \
        size_t len;                                                         \
        lwpb_encoder_init(&encoder);                                        \
        lwpb_encoder_start(&encoder, foo_TestFieldNo##field, buf, sizeof(buf)); \
        ret = lwpb_encoder_add_string(&encoder, foo_TestFieldNo##field##_test, "tst"); \
        CHECK_LWPB(ret);                                                    \
        len = lwpb_encoder_finish(&encoder);                                \
        CHECK_BUF(buf, len, test_field_number_##field);                     \
        fields.u.TestFieldNo.test = "tst";                                  \
        lwpb_decoder_init(&decoder);                                        \
        lwpb_decoder_arg(&decoder, &fields);                                \
        lwpb_decoder_field_handler(&decoder, generic_field_handler);        \
        ret = lwpb_decoder_decode(&decoder, foo_TestFieldNo##field, buf, len); \
        CHECK_LWPB(ret);                                                    \
    } while (0);
    
    DO_TEST(15);
    DO_TEST(16);
    DO_TEST(2047);
    DO_TEST(2048);
    DO_TEST(262143);
    DO_TEST(262144);
    DO_TEST(33554431);
    DO_TEST(33554432);
    
#undef DO_TEST
}

#define DO_TEST_REQUIRED(msg_type, lwpb_type, value, vector)                \
    do {                                                                    \
        lwpb_err_t ret;                                                     \
        struct lwpb_encoder encoder;                                        \
        struct lwpb_decoder decoder;                                        \
        struct testing_fields fields;                                       \
        uint8_t buf[256];                                                   \
        size_t len;                                                         \
        lwpb_encoder_init(&encoder);                                        \
        lwpb_encoder_start(&encoder, foo_TestMessRequired##msg_type, buf, sizeof(buf)); \
        ret = lwpb_encoder_add_##lwpb_type(&encoder, foo_TestMessRequired##msg_type##_test, value); \
        CHECK_LWPB(ret);                                                    \
        len = lwpb_encoder_finish(&encoder);                                \
        CHECK_BUF(buf, len, vector);                                        \
        fields.u.TestMessRequired##msg_type.test = value;                   \
        lwpb_decoder_init(&decoder);                                        \
        lwpb_decoder_arg(&decoder, &fields);                                \
        lwpb_decoder_field_handler(&decoder, generic_field_handler);        \
        ret = lwpb_decoder_decode(&decoder, foo_TestMessRequired##msg_type, buf, len); \
        CHECK_LWPB(ret);                                                    \
    } while (0);

static void test_required_int32(void)
{
    DO_TEST_REQUIRED(Int32, int32, INT32_MIN, test_required_int32_min);
    DO_TEST_REQUIRED(Int32, int32, -1000, test_required_int32_m1000);
    DO_TEST_REQUIRED(Int32, int32, 0, test_required_int32_0);
    DO_TEST_REQUIRED(Int32, int32, INT32_MAX, test_required_int32_max);
}

static void test_required_sint32(void)
{
    DO_TEST_REQUIRED(SInt32, int32, INT32_MIN, test_required_sint32_min);
    DO_TEST_REQUIRED(SInt32, int32, -1000, test_required_sint32_m1000);
    DO_TEST_REQUIRED(SInt32, int32, 0, test_required_sint32_0);
    DO_TEST_REQUIRED(SInt32, int32, INT32_MAX, test_required_sint32_max);
}

static void test_required_sfixed32(void)
{
    DO_TEST_REQUIRED(SFixed32, int32, INT32_MIN, test_required_sfixed32_min);
    DO_TEST_REQUIRED(SFixed32, int32, -1000, test_required_sfixed32_m1000);
    DO_TEST_REQUIRED(SFixed32, int32, 0, test_required_sfixed32_0);
    DO_TEST_REQUIRED(SFixed32, int32, INT32_MAX, test_required_sfixed32_max);
}

static void test_required_uint32(void)
{
    DO_TEST_REQUIRED(UInt32, uint32, 0, test_required_uint32_0);
    DO_TEST_REQUIRED(UInt32, uint32, MILLION, test_required_uint32_million);
    DO_TEST_REQUIRED(UInt32, uint32, UINT32_MAX, test_required_uint32_max);
}

static void test_required_fixed32(void)
{
    DO_TEST_REQUIRED(Fixed32, uint32, 0, test_required_fixed32_0);
    DO_TEST_REQUIRED(Fixed32, uint32, MILLION, test_required_fixed32_million);
    DO_TEST_REQUIRED(Fixed32, uint32, UINT32_MAX, test_required_fixed32_max);
}

static void test_required_int64(void)
{
    DO_TEST_REQUIRED(Int64, int64, INT64_MIN, test_required_int64_min);
    DO_TEST_REQUIRED(Int64, int64, -TRILLION, test_required_int64_mtril);
    DO_TEST_REQUIRED(Int64, int64, 0, test_required_int64_0);
    DO_TEST_REQUIRED(Int64, int64, QUADRILLION, test_required_int64_quad);
    DO_TEST_REQUIRED(Int64, int64, INT64_MAX, test_required_int64_max);
}

static void test_required_sint64(void)
{
    DO_TEST_REQUIRED(SInt64, int64, INT64_MIN, test_required_sint64_min);
    DO_TEST_REQUIRED(SInt64, int64, -TRILLION, test_required_sint64_mtril);
    DO_TEST_REQUIRED(SInt64, int64, 0, test_required_sint64_0);
    DO_TEST_REQUIRED(SInt64, int64, QUADRILLION, test_required_sint64_quad);
    DO_TEST_REQUIRED(SInt64, int64, INT64_MAX, test_required_sint64_max);
}

static void test_required_sfixed64(void)
{
    DO_TEST_REQUIRED(SFixed64, int64, INT64_MIN, test_required_sfixed64_min);
    DO_TEST_REQUIRED(SFixed64, int64, -TRILLION, test_required_sfixed64_mtril);
    DO_TEST_REQUIRED(SFixed64, int64, 0, test_required_sfixed64_0);
    DO_TEST_REQUIRED(SFixed64, int64, QUADRILLION, test_required_sfixed64_quad);
    DO_TEST_REQUIRED(SFixed64, int64, INT64_MAX, test_required_sfixed64_max);
}

static void test_required_uint64(void)
{
    DO_TEST_REQUIRED(UInt64, uint64, 0, test_required_uint64_0);
    DO_TEST_REQUIRED(UInt64, uint64, THOUSAND, test_required_uint64_thou);
    DO_TEST_REQUIRED(UInt64, uint64, MILLION, test_required_uint64_mill);
    DO_TEST_REQUIRED(UInt64, uint64, BILLION, test_required_uint64_bill);
    DO_TEST_REQUIRED(UInt64, uint64, TRILLION, test_required_uint64_tril);
    DO_TEST_REQUIRED(UInt64, uint64, QUADRILLION, test_required_uint64_quad);
    DO_TEST_REQUIRED(UInt64, uint64, QUINTILLION, test_required_uint64_quint);
    DO_TEST_REQUIRED(UInt64, uint64, UINT64_MAX, test_required_uint64_max);
}

static void test_required_fixed64(void)
{
    DO_TEST_REQUIRED(Fixed64, uint64, 0, test_required_fixed64_0);
    DO_TEST_REQUIRED(Fixed64, uint64, THOUSAND, test_required_fixed64_thou);
    DO_TEST_REQUIRED(Fixed64, uint64, MILLION, test_required_fixed64_mill);
    DO_TEST_REQUIRED(Fixed64, uint64, BILLION, test_required_fixed64_bill);
    DO_TEST_REQUIRED(Fixed64, uint64, TRILLION, test_required_fixed64_tril);
    DO_TEST_REQUIRED(Fixed64, uint64, QUADRILLION, test_required_fixed64_quad);
    DO_TEST_REQUIRED(Fixed64, uint64, QUINTILLION, test_required_fixed64_quint);
    DO_TEST_REQUIRED(Fixed64, uint64, UINT64_MAX, test_required_fixed64_max);
}

static void test_required_float(void)
{
    DO_TEST_REQUIRED(Float, float, -THOUSAND, test_required_float_mthou);
    DO_TEST_REQUIRED(Float, float, 0, test_required_float_0);
    DO_TEST_REQUIRED(Float, float, 420, test_required_float_420);
}

static void test_required_double(void)
{
    DO_TEST_REQUIRED(Double, double, -THOUSAND, test_required_double_mthou);
    DO_TEST_REQUIRED(Double, double, 0, test_required_double_0);
    DO_TEST_REQUIRED(Double, double, 420, test_required_double_420);
}

static void test_required_bool(void)
{
    DO_TEST_REQUIRED(Bool, bool, 0, test_required_bool_0);
    DO_TEST_REQUIRED(Bool, bool, 1, test_required_bool_1);
}

static void test_required_enum_small(void)
{
    DO_TEST_REQUIRED(EnumSmall, enum, FOO_VALUE, test_required_enum_small_VALUE);
    DO_TEST_REQUIRED(EnumSmall, enum, FOO_OTHER_VALUE, test_required_enum_small_OTHER_VALUE);
}

static void test_required_enum_big(void)
{
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE0, test_required_enum_0);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE1, test_required_enum_1);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE127, test_required_enum_127);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE128, test_required_enum_128);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE16383, test_required_enum_16383);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE16384, test_required_enum_16384);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE2097151, test_required_enum_2097151);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE2097152, test_required_enum_2097152);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE268435455, test_required_enum_268435455);
    DO_TEST_REQUIRED(Enum, enum, FOO_VALUE268435456, test_required_enum_268435456);
}

static void test_required_string(void)
{
    DO_TEST_REQUIRED(String, string, "", test_required_string_empty);
    DO_TEST_REQUIRED(String, string, "hello", test_required_string_hello);
    DO_TEST_REQUIRED(String, string, "two hundred xs follow: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", test_required_string_long);
}

static void test_required_bytes(void)
{
    /* TODO: implement
    DO_TEST_REQUIRED(Bytes, bytes, "", test_required_bytes_empty);
    DO_TEST_REQUIRED(Bytes, bytes, "hello", test_required_bytes_hello);
    DO_TEST_REQUIRED(Bytes, bytes, "\1\0\375\2\4", test_required_bytes_random);
    */
}

static void test_required_submess(void)
{
    // TODO: implement
}
    
#if 0

static void test_required_SubMess (void)
{
  Foo__SubMess submess = FOO__SUB_MESS__INIT;
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED (Message, MESSAGE, message, value, example_packed_data, submesses_equals)
  submess.test = 0;
  DO_TEST (&submess, test_required_submess_0);
  submess.test = 42;
  DO_TEST (&submess, test_required_submess_42);
#undef DO_TEST
}

#endif

static void test_empty_optional(void)
{
    lwpb_err_t ret;
    struct lwpb_encoder encoder;
    struct lwpb_decoder decoder;
    struct testing_fields fields;
    uint8_t buf[256];
    size_t len;
    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, foo_TestMessOptional, buf, sizeof(buf));
    len = lwpb_encoder_finish(&encoder);
    CHECK_ASSERT(len == 0, "length must be null");
    lwpb_decoder_init(&decoder);
    ret = lwpb_decoder_decode(&decoder, foo_TestMessOptional, buf, len);
    CHECK_LWPB(ret);
}

#define DO_TEST_OPTIONAL(lwpb_type, field, value, vector)                   \
    do {                                                                    \
        lwpb_err_t ret;                                                     \
        struct lwpb_encoder encoder;                                        \
        struct lwpb_decoder decoder;                                        \
        struct testing_fields fields;                                       \
        uint8_t buf[256];                                                   \
        size_t len;                                                         \
        lwpb_encoder_init(&encoder);                                        \
        lwpb_encoder_start(&encoder, foo_TestMessOptional, buf, sizeof(buf)); \
        ret = lwpb_encoder_add_##lwpb_type(&encoder, foo_TestMessOptional_test_##field, value); \
        CHECK_LWPB(ret);                                                    \
        len = lwpb_encoder_finish(&encoder);                                \
        CHECK_BUF(buf, len, vector);                                        \
        fields.u.TestMessOptional.test_##field = value;                     \
        lwpb_decoder_init(&decoder);                                        \
        lwpb_decoder_arg(&decoder, &fields);                                \
        lwpb_decoder_field_handler(&decoder, generic_field_handler);        \
        ret = lwpb_decoder_decode(&decoder, foo_TestMessOptional, buf, len); \
        CHECK_LWPB(ret);                                                    \
    } while (0);


static void test_optional_int32(void)
{
    DO_TEST_OPTIONAL(int32, int32, INT32_MIN, test_optional_int32_min);
    DO_TEST_OPTIONAL(int32, int32, -1, test_optional_int32_m1);
    DO_TEST_OPTIONAL(int32, int32, 0, test_optional_int32_0);
    DO_TEST_OPTIONAL(int32, int32, 666, test_optional_int32_666);
    DO_TEST_OPTIONAL(int32, int32, INT32_MAX, test_optional_int32_max);
}

static void test_optional_sint32(void)
{
    DO_TEST_OPTIONAL(int32, sint32, INT32_MIN, test_optional_sint32_min);
    DO_TEST_OPTIONAL(int32, sint32, -1, test_optional_sint32_m1);
    DO_TEST_OPTIONAL(int32, sint32, 0, test_optional_sint32_0);
    DO_TEST_OPTIONAL(int32, sint32, 666, test_optional_sint32_666);
    DO_TEST_OPTIONAL(int32, sint32, INT32_MAX, test_optional_sint32_max);
}

static void test_optional_sfixed32(void)
{
    DO_TEST_OPTIONAL(int32, sfixed32, INT32_MIN, test_optional_sfixed32_min);
    DO_TEST_OPTIONAL(int32, sfixed32, -1, test_optional_sfixed32_m1);
    DO_TEST_OPTIONAL(int32, sfixed32, 0, test_optional_sfixed32_0);
    DO_TEST_OPTIONAL(int32, sfixed32, 666, test_optional_sfixed32_666);
    DO_TEST_OPTIONAL(int32, sfixed32, INT32_MAX, test_optional_sfixed32_max);
}

static void test_optional_uint32(void)
{
    DO_TEST_OPTIONAL(uint32, uint32, 0, test_optional_uint32_0);
    DO_TEST_OPTIONAL(uint32, uint32, 669, test_optional_uint32_669);
    DO_TEST_OPTIONAL(uint32, uint32, UINT32_MAX, test_optional_uint32_max);
}

static void test_optional_fixed32(void)
{
    DO_TEST_OPTIONAL(uint32, fixed32, 0, test_optional_fixed32_0);
    DO_TEST_OPTIONAL(uint32, fixed32, 669, test_optional_fixed32_669);
    DO_TEST_OPTIONAL(uint32, fixed32, UINT32_MAX, test_optional_fixed32_max);
}

static void test_optional_int64(void)
{
    DO_TEST_OPTIONAL(int64, int64, INT64_MIN, test_optional_int64_min);
    DO_TEST_OPTIONAL(int64, int64, -1111111111LL, test_optional_int64_m1111111111LL);
    DO_TEST_OPTIONAL(int64, int64, 0, test_optional_int64_0);
    DO_TEST_OPTIONAL(int64, int64, QUINTILLION, test_optional_int64_quintillion);
    DO_TEST_OPTIONAL(int64, int64, INT64_MAX, test_optional_int64_max);
}

static void test_optional_sint64(void)
{
    DO_TEST_OPTIONAL(int64, sint64, INT64_MIN, test_optional_sint64_min);
    DO_TEST_OPTIONAL(int64, sint64, -1111111111LL, test_optional_sint64_m1111111111LL);
    DO_TEST_OPTIONAL(int64, sint64, 0, test_optional_sint64_0);
    DO_TEST_OPTIONAL(int64, sint64, QUINTILLION, test_optional_sint64_quintillion);
    DO_TEST_OPTIONAL(int64, sint64, INT64_MAX, test_optional_sint64_max);
}

static void test_optional_sfixed64(void)
{
    DO_TEST_OPTIONAL(int64, sfixed64, INT64_MIN, test_optional_sfixed64_min);
    DO_TEST_OPTIONAL(int64, sfixed64, -1111111111LL, test_optional_sfixed64_m1111111111LL);
    DO_TEST_OPTIONAL(int64, sfixed64, 0, test_optional_sfixed64_0);
    DO_TEST_OPTIONAL(int64, sfixed64, QUINTILLION, test_optional_sfixed64_quintillion);
    DO_TEST_OPTIONAL(int64, sfixed64, INT64_MAX, test_optional_sfixed64_max);
}

static void test_optional_uint64(void)
{
    DO_TEST_OPTIONAL(uint64, uint64, 0, test_optional_uint64_0);
    DO_TEST_OPTIONAL(uint64, uint64, 669669669669669ULL, test_optional_uint64_669669669669669);
    DO_TEST_OPTIONAL(uint64, uint64, UINT64_MAX, test_optional_uint64_max);
}

static void test_optional_fixed64(void)
{
    DO_TEST_OPTIONAL(uint64, fixed64, 0, test_optional_fixed64_0);
    DO_TEST_OPTIONAL(uint64, fixed64, 669669669669669ULL, test_optional_fixed64_669669669669669);
    DO_TEST_OPTIONAL(uint64, fixed64, UINT64_MAX, test_optional_fixed64_max);
}

static void test_optional_float(void)
{
    DO_TEST_OPTIONAL(float, float, -100, test_optional_float_m100);
    DO_TEST_OPTIONAL(float, float, 0, test_optional_float_0);
    DO_TEST_OPTIONAL(float, float, 141243, test_optional_float_141243);
}

static void test_optional_double(void)
{
    DO_TEST_OPTIONAL(double, double, -100, test_optional_double_m100);
    DO_TEST_OPTIONAL(double, double, 0, test_optional_double_0);
    DO_TEST_OPTIONAL(double, double, 141243, test_optional_double_141243);
}

static void test_optional_bool(void)
{
    DO_TEST_OPTIONAL(bool, boolean, 0, test_optional_bool_0);
    DO_TEST_OPTIONAL(bool, boolean, 1, test_optional_bool_1);
}

static void test_optional_enum_small(void)
{
    DO_TEST_OPTIONAL(enum, enum_small, FOO_VALUE, test_optional_enum_small_0);
    DO_TEST_OPTIONAL(enum, enum_small, FOO_OTHER_VALUE, test_optional_enum_small_1);
}

static void test_optional_enum_big(void)
{
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE0, test_optional_enum_0);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE1, test_optional_enum_1);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE127, test_optional_enum_127);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE128, test_optional_enum_128);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE16383, test_optional_enum_16383);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE16384, test_optional_enum_16384);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE2097151, test_optional_enum_2097151);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE2097152, test_optional_enum_2097152);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE268435455, test_optional_enum_268435455);
    DO_TEST_OPTIONAL(enum, enum, FOO_VALUE268435456, test_optional_enum_268435456);
}

static void test_optional_string(void)
{
    DO_TEST_OPTIONAL(string, string, "", test_optional_string_empty);
    DO_TEST_OPTIONAL(string, string, "hello", test_optional_string_hello);
}

static void test_optional_bytes(void)
{
    /* TODO: implement
  static ProtobufCBinaryData bd_empty = { 0, (uint8_t*)"" };
  static ProtobufCBinaryData bd_hello = { 5, (uint8_t*)"hello" };
  static ProtobufCBinaryData bd_random = { 5, (uint8_t*)"\1\0\375\2\4" };
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL (test_bytes, value, example_packed_data, binary_data_equals)
  DO_TEST (bd_empty, test_optional_bytes_empty);
  DO_TEST (bd_hello, test_optional_bytes_hello);
  DO_TEST (bd_random, test_optional_bytes_random);
  */
}

static void test_optional_submess(void)
{
    // TODO: implement
}

static void test_empty_repeated(void)
{
    lwpb_err_t ret;
    struct lwpb_encoder encoder;
    struct lwpb_decoder decoder;
    struct testing_fields fields;
    uint8_t buf[256];
    size_t len;
    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, foo_TestMess, buf, sizeof(buf));
    len = lwpb_encoder_finish(&encoder);
    CHECK_ASSERT(len == 0, "length must be null");
    lwpb_decoder_init(&decoder);
    ret = lwpb_decoder_decode(&decoder, foo_TestMess, buf, len);
    CHECK_LWPB(ret);
}

#define DO_TEST_REPEATED(lwpb_type, field, array, vector)                   \
    do {                                                                    \
        lwpb_err_t ret;                                                     \
        struct lwpb_encoder encoder;                                        \
        struct lwpb_decoder decoder;                                        \
        struct testing_fields fields;                                       \
        uint8_t buf[512];                                                   \
        size_t len;                                                         \
        int i;                                                              \
        lwpb_encoder_init(&encoder);                                        \
        lwpb_encoder_start(&encoder, foo_TestMess, buf, sizeof(buf)); \
        for (i = 0; i < ARRAY_SIZE(array); i++)                             \
            ret = lwpb_encoder_add_##lwpb_type(&encoder, foo_TestMess_test_##field, array[i]); \
        CHECK_LWPB(ret);                                                    \
        len = lwpb_encoder_finish(&encoder);                                \
        CHECK_BUF(buf, len, vector);                                        \
        fields.u.TestMess.test_##field = array;                             \
        lwpb_decoder_init(&decoder);                                        \
        lwpb_decoder_arg(&decoder, &fields);                                \
        lwpb_decoder_field_handler(&decoder, generic_field_handler);        \
        ret = lwpb_decoder_decode(&decoder, foo_TestMess, buf, len);        \
        CHECK_LWPB(ret);                                                    \
    } while (0);


static void test_repeated_int32(void)
{
    DO_TEST_REPEATED(int32, int32, int32_arr0, test_repeated_int32_arr0);
    DO_TEST_REPEATED(int32, int32, int32_arr1, test_repeated_int32_arr1);
    DO_TEST_REPEATED(int32, int32, int32_arr_min_max, test_repeated_int32_arr_min_max);
}

static void test_repeated_sint32(void)
{
    DO_TEST_REPEATED(int32, sint32, int32_arr0, test_repeated_sint32_arr0);
    DO_TEST_REPEATED(int32, sint32, int32_arr1, test_repeated_sint32_arr1);
    DO_TEST_REPEATED(int32, sint32, int32_arr_min_max, test_repeated_sint32_arr_min_max);
}

static void test_repeated_sfixed32(void)
{
    DO_TEST_REPEATED(int32, sfixed32, int32_arr0, test_repeated_sfixed32_arr0);
    DO_TEST_REPEATED(int32, sfixed32, int32_arr1, test_repeated_sfixed32_arr1);
    DO_TEST_REPEATED(int32, sfixed32, int32_arr_min_max, test_repeated_sfixed32_arr_min_max);
}

static void test_repeated_uint32(void)
{
    DO_TEST_REPEATED(uint32, uint32, uint32_roundnumbers, test_repeated_uint32_roundnumbers);
    DO_TEST_REPEATED(uint32, uint32, uint32_0_max, test_repeated_uint32_0_max);
}

static void test_repeated_fixed32(void)
{
    DO_TEST_REPEATED(uint32, fixed32, uint32_roundnumbers, test_repeated_fixed32_roundnumbers);
    DO_TEST_REPEATED(uint32, fixed32, uint32_0_max, test_repeated_fixed32_0_max);
}

static void test_repeated_int64(void)
{
    DO_TEST_REPEATED(int64, int64, int64_roundnumbers, test_repeated_int64_roundnumbers);
    DO_TEST_REPEATED(int64, int64, int64_min_max, test_repeated_int64_min_max);
}

static void test_repeated_sint64(void)
{
    DO_TEST_REPEATED(int64, sint64, int64_roundnumbers, test_repeated_sint64_roundnumbers);
    DO_TEST_REPEATED(int64, sint64, int64_min_max, test_repeated_sint64_min_max);
}

static void test_repeated_sfixed64(void)
{
    DO_TEST_REPEATED(int64, sfixed64, int64_roundnumbers, test_repeated_sfixed64_roundnumbers);
    DO_TEST_REPEATED(int64, sfixed64, int64_min_max, test_repeated_sfixed64_min_max);
}

static void test_repeated_uint64(void)
{
    DO_TEST_REPEATED(uint64, uint64, uint64_roundnumbers, test_repeated_uint64_roundnumbers);
    DO_TEST_REPEATED(uint64, uint64, uint64_0_1_max, test_repeated_uint64_0_1_max);
    DO_TEST_REPEATED(uint64, uint64, uint64_random, test_repeated_uint64_random);
}

static void test_repeated_fixed64(void)
{
    DO_TEST_REPEATED(uint64, fixed64, uint64_roundnumbers, test_repeated_fixed64_roundnumbers);
    DO_TEST_REPEATED(uint64, fixed64, uint64_0_1_max, test_repeated_fixed64_0_1_max);
    DO_TEST_REPEATED(uint64, fixed64, uint64_random, test_repeated_fixed64_random);
}

static void test_repeated_float(void)
{
    DO_TEST_REPEATED(float, float, float_random, test_repeated_float_random);
}

static void test_repeated_double(void)
{
    DO_TEST_REPEATED(double, double, double_random, test_repeated_double_random);
}

static void test_repeated_bool(void)
{
    DO_TEST_REPEATED(bool, boolean, boolean_0, test_repeated_boolean_0);
    DO_TEST_REPEATED(bool, boolean, boolean_1, test_repeated_boolean_1);
    DO_TEST_REPEATED(bool, boolean, boolean_random, test_repeated_boolean_random);
}

static void test_repeated_enum_small(void)
{
    DO_TEST_REPEATED(enum, enum_small, enum_small_0, test_repeated_enum_small_0);
    DO_TEST_REPEATED(enum, enum_small, enum_small_1, test_repeated_enum_small_1);
    DO_TEST_REPEATED(enum, enum_small, enum_small_random, test_repeated_enum_small_random);
}

static void test_repeated_enum_big(void)
{
    DO_TEST_REPEATED(enum, enum, enum_0, test_repeated_enum_0);
    DO_TEST_REPEATED(enum, enum, enum_1, test_repeated_enum_1);
    DO_TEST_REPEATED(enum, enum, enum_random, test_repeated_enum_random);
}

static void test_repeated_string(void)
{
    DO_TEST_REPEATED(string, string, repeated_strings_0, test_repeated_strings_0);
    DO_TEST_REPEATED(string, string, repeated_strings_1, test_repeated_strings_1);
    DO_TEST_REPEATED(string, string, repeated_strings_2, test_repeated_strings_2);
    DO_TEST_REPEATED(string, string, repeated_strings_3, test_repeated_strings_3);
}

static void test_repeated_bytes(void)
{
    // TODO: implement
}

static void test_repeated_submess(void)
{
    // TODO: implement
}

static void test_required_default_values(void)
{
    // TODO: implement
}

static void test_optional_default_values(void)
{
    // TODO: implement
}

#if 0

static void test_repeated_bytes (void)
{
  static ProtobufCBinaryData test_binary_data_0[] = {
    { 4, (uint8_t *) "text" },
    { 9, (uint8_t *) "str\1\2\3\4\5\0" },
    { 10, (uint8_t *) "gobble\0foo" }
  };
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_bytes, , \
                   static_array, example_packed_data, \
                   binary_data_equals)

  DO_TEST (test_binary_data_0, test_repeated_bytes_0);

#undef DO_TEST
}

static void test_repeated_SubMess (void)
{
  static Foo__SubMess submess0 = FOO__SUB_MESS__INIT;
  static Foo__SubMess submess1 = FOO__SUB_MESS__INIT;
  static Foo__SubMess submess2 = FOO__SUB_MESS__INIT;
  static Foo__SubMess *submesses[3] = { &submess0, &submess1, &submess2 };

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_message, , \
                   static_array, example_packed_data, \
                   submesses_equals)

  DO_TEST (submesses, test_repeated_submess_0);
  submess0.test = 42;
  submess1.test = -10000;
  submess2.test = 667;
  DO_TEST (submesses, test_repeated_submess_1);

#undef DO_TEST
}



static void
test_required_default_values (void)
{
  Foo__DefaultRequiredValues mess = FOO__DEFAULT_REQUIRED_VALUES__INIT;
  Foo__DefaultRequiredValues *mess2;
  size_t len; uint8_t *data;
  assert_required_default_values_are_default (&mess);
  mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  free (data);
  assert_required_default_values_are_default (mess2);
  foo__default_required_values__free_unpacked (mess2, NULL);
}

static void
assert_optional_default_values_are_default (Foo__DefaultOptionalValues *mess)
{
  assert (!mess->has_v_int32);
  assert (mess->v_int32 == -42);
  assert (!mess->has_v_uint32);
  assert (mess->v_uint32 == 666);
  assert (!mess->has_v_int64);
  assert (mess->v_int64 == 100000);
  assert (!mess->has_v_uint64);
  assert (mess->v_uint64 == 100001);
  assert (!mess->has_v_float);
  assert (mess->v_float == 2.5);
  assert (!mess->has_v_double);
  assert (mess->v_double == 4.5);
  assert (strcmp (mess->v_string, "hi mom\n") == 0);
  assert (!mess->has_v_bytes);
  assert (mess->v_bytes.len = /* a */ 1
                               + /* space */ 1
                               + /* NUL */ 1
                               + /* space */ 1
                               + /* "character" */ 9);
  assert (memcmp (mess->v_bytes.data, "a \0 character", 13) == 0);
}

static void
test_optional_default_values (void)
{
  Foo__DefaultOptionalValues mess = FOO__DEFAULT_OPTIONAL_VALUES__INIT;
  Foo__DefaultOptionalValues *mess2;
  size_t len; uint8_t *data;
  assert_optional_default_values_are_default (&mess);
  mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  assert (len == 0);            /* no non-default values */
  free (data);
  assert_optional_default_values_are_default (mess2);
  foo__default_optional_values__free_unpacked (mess2, NULL);
}


#endif


// Simple testing framework

typedef void (*test_func_t)(void);

struct test_entry {
    const char *name;
    test_func_t func;
};

static struct test_entry tests[] = {
    { "small enums", test_enum_small },
    { "big enums", test_enum_big },
    { "field numbers", test_field_numbers },
    { "required int32", test_required_int32 },
    { "required sint32", test_required_sint32 },
    { "required sfixed32", test_required_sfixed32 },
    { "required uint32", test_required_uint32 },
    { "required fixed32", test_required_fixed32 },
    { "required int64", test_required_int64 },
    { "required sint64", test_required_sint64 },
    { "required sfixed64", test_required_sfixed64 },
    { "required uint64", test_required_uint64 },
    { "required fixed64", test_required_fixed64 },
    { "required float", test_required_float },
    { "required double", test_required_double },
    { "required bool", test_required_bool },
    { "required small enum", test_required_enum_small },
    { "required big enum", test_required_enum_big },
    { "required string", test_required_string },
    { "required bytes", test_required_bytes },
    { "required sub message", test_required_submess },
    
    { "empty optional", test_empty_optional },
    { "optional int32", test_optional_int32 },
    { "optional sint32", test_optional_sint32 },
    { "optional sfixed32", test_optional_sfixed32 },
    { "optional uint32", test_optional_uint32 },
    { "optional fixed32", test_optional_fixed32 },
    { "optional int64", test_optional_int64 },
    { "optional sint64", test_optional_sint64 },
    { "optional sfixed64", test_optional_sfixed64 },
    { "optional uint64", test_optional_uint64 },
    { "optional fixed64", test_optional_fixed64 },
    { "optional float", test_optional_float },
    { "optional double", test_optional_double },
    { "optional bool", test_optional_bool },
    { "optional small enum", test_optional_enum_small },
    { "optional big enum", test_optional_enum_big },
    { "optional string", test_optional_string },
    { "optional bytes", test_optional_bytes },
    { "optional sub message", test_optional_submess },
    
    { "empty repeated", test_empty_repeated},
    { "repeated int32", test_repeated_int32 },
    { "repeated sint32", test_repeated_sint32 },
    { "repeated sfixed32", test_repeated_sfixed32 },
    { "repeated uint32", test_repeated_uint32 },
    { "repeated fixed32", test_repeated_fixed32 },
    { "repeated int64", test_repeated_int64 },
    { "repeated sint64", test_repeated_sint64 },
    { "repeated sfixed64", test_repeated_sfixed64 },
    { "repeated uint64", test_repeated_uint64 },
    { "repeated fixed64", test_repeated_fixed64 },
    { "repeated float", test_repeated_float },
    { "repeated double", test_repeated_double },
    { "repeated bool", test_repeated_bool },
    { "repeated small enum", test_repeated_enum_small },
    { "repeated big enum", test_repeated_enum_big },
    { "repeated string", test_repeated_string },
    { "repeated bytes", test_repeated_bytes },
    { "repeated sub message", test_repeated_submess },

    { "required default values", test_required_default_values },
    { "optional default values", test_optional_default_values },
    
};

int main()
{
    int i;
    
    for (i = 0; i < ARRAY_SIZE(tests); i++) {
        printf("Test: %s\n", tests[i].name);
        tests[i].func();
    }
    
    printf("All tests successful\n");

    return 0;
}
