
#include <lwpb/lwpb.h>

#include "generated/test_struct_map_pb2.h"

struct test_struct_nested1 {
    s32_t field_int32;
    s64_t field_int64;
};

struct test_struct_nested2 {
    char field_string[32];
};

struct test_struct {
    s32_t field_int32;
    s64_t field_int64;
    lwpb_bool_t field_bool;
    lwpb_enum_t field_enum;
    char field_string[32];
    u8_t field_bytes[8];
    struct test_struct_nested1 nested1;
    struct test_struct_nested2 nested2[8];
};

LWPB_STRUCT_MAP_BEGIN(test_struct_nested1_map, test_StructTest_Nested1, struct test_struct_nested1)
LWPB_STRUCT_MAP_INT32(test_StructTest_Nested1_field_int32, struct test_struct_nested1, field_int32, 1)
LWPB_STRUCT_MAP_INT64(test_StructTest_Nested1_field_int64, struct test_struct_nested1, field_int64, 1)
LWPB_STRUCT_MAP_END

LWPB_STRUCT_MAP_BEGIN(test_struct_nested2_map, test_StructTest_Nested2, struct test_struct_nested2)
LWPB_STRUCT_MAP_STRING(test_StructTest_Nested2_field_string, struct test_struct_nested2, field_string, 32, 1)
LWPB_STRUCT_MAP_END

LWPB_STRUCT_MAP_BEGIN(test_struct_map, test_StructTest, struct test_struct)
LWPB_STRUCT_MAP_INT32(test_StructTest_field_int32, struct test_struct, field_int32, 1)
LWPB_STRUCT_MAP_INT64(test_StructTest_field_int64, struct test_struct, field_int64, 1)
LWPB_STRUCT_MAP_BOOL(test_StructTest_field_bool, struct test_struct, field_bool, 1)
LWPB_STRUCT_MAP_ENUM(test_StructTest_field_enum, struct test_struct, field_enum, 1)
LWPB_STRUCT_MAP_STRING(test_StructTest_field_string, struct test_struct, field_string, 32, 1)
LWPB_STRUCT_MAP_BYTES(test_StructTest_field_bytes, struct test_struct, field_bytes, 8, 1)
LWPB_STRUCT_MAP_MESSAGE(test_StructTest_nested1, struct test_struct, nested1, &test_struct_nested1_map, 1)
LWPB_STRUCT_MAP_MESSAGE(test_StructTest_nested2, struct test_struct, nested2, &test_struct_nested2_map, 8)
LWPB_STRUCT_MAP_END



void print_buf(u8_t *buf, size_t len)
{
    int i = 0;
    
    LWPB_DIAG_PRINTF("{ ");
    while (i < len) {
        LWPB_DIAG_PRINTF("0x%02x", buf[i]);
        if (i < len - 1)
            LWPB_DIAG_PRINTF(", ");
        i++;
    }
    
    LWPB_DIAG_PRINTF(" }");
}

int main()
{
    char buf[4096];
    size_t len;
    lwpb_err_t ret;
    u8_t bytes[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    struct test_struct test_struct_instance;
    int i;
    
    struct lwpb_encoder encoder;
    struct lwpb_struct_decoder sdecoder;

    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, test_StructTest, buf, sizeof(buf));
    lwpb_encoder_add_int32(&encoder, test_StructTest_field_int32, 12345);
    lwpb_encoder_add_int64(&encoder, test_StructTest_field_int64, 1234567890);
    lwpb_encoder_add_bool(&encoder, test_StructTest_field_bool, 1);
    lwpb_encoder_add_enum(&encoder, test_StructTest_field_enum, TEST_STRUCTTEST_VALUE1);
    lwpb_encoder_add_string(&encoder, test_StructTest_field_string, "this is a test");
    lwpb_encoder_add_bytes(&encoder, test_StructTest_field_bytes, bytes, sizeof(bytes));
    
    lwpb_encoder_nested_start(&encoder, test_StructTest_nested1);
    lwpb_encoder_add_int32(&encoder, test_StructTest_Nested1_field_int32, 123456);
    lwpb_encoder_add_int64(&encoder, test_StructTest_Nested1_field_int64, 987654321);
    lwpb_encoder_nested_end(&encoder);
    
    for (i = 0; i < 8; i++) {
        char tmp[] = "test string x";
        lwpb_encoder_nested_start(&encoder, test_StructTest_nested2);
        tmp[12] = '0' + i;
        lwpb_encoder_add_string(&encoder, test_StructTest_Nested2_field_string, tmp);
        lwpb_encoder_nested_end(&encoder);
    }
    
    len = lwpb_encoder_finish(&encoder);

    LWPB_DIAG_PRINTF("encoded message length = %d\n", len);
    
    lwpb_struct_decoder_init(&sdecoder);
    ret = lwpb_struct_decoder_decode(&sdecoder, &test_struct_map, &test_struct_instance, buf, len, NULL);
    
    LWPB_DIAG_PRINTF("ret = %d\n", ret);
    
    LWPB_DIAG_PRINTF("test_struct.field_int32 = %d\n", test_struct_instance.field_int32);
    LWPB_DIAG_PRINTF("test_struct.field_int64 = %lld\n", test_struct_instance.field_int64);
    LWPB_DIAG_PRINTF("test_struct.field_bool = %d\n", test_struct_instance.field_bool);
    LWPB_DIAG_PRINTF("test_struct.field_enum = %d\n", test_struct_instance.field_enum);
    LWPB_DIAG_PRINTF("test_struct.field_string = '%s'\n", test_struct_instance.field_string);
    LWPB_DIAG_PRINTF("test_struct.field_bytes = ");
    print_buf(test_struct_instance.field_bytes, sizeof(test_struct_instance.field_bytes));
    LWPB_DIAG_PRINTF("\n");
    
    LWPB_DIAG_PRINTF("test_struct.nested1.field_int32 = %d\n", test_struct_instance.nested1.field_int32);
    LWPB_DIAG_PRINTF("test_struct.nested1.field_int64 = %lld\n", test_struct_instance.nested1.field_int64);
    
    for (i = 0; i < 8; i++)
        LWPB_DIAG_PRINTF("test_struct.nested2[%d].field_string = '%s'\n", i, test_struct_instance.nested2[i].field_string);
    
    return 0;
}
