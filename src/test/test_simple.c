
#include <lwpb/lwpb.h>

#include "generated/test_simple_pb2.h"

int main()
{
    char buf[4096];
    size_t len;
    lwpb_err_t ret;
    
    struct lwpb_decoder decoder;
    struct lwpb_encoder encoder;
    
    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, test_Person, buf, sizeof(buf));
    lwpb_encoder_add_string(&encoder, test_Person_name, "Simon Kallweit");
    lwpb_encoder_add_int32(&encoder, test_Person_id, 123);
    lwpb_encoder_add_string(&encoder, test_Person_email, "simon.kallweit@intefo.ch");
    lwpb_encoder_nested_start(&encoder, test_Person_phone);
    lwpb_encoder_add_string(&encoder, test_PhoneNumber_number, "123456789");
    lwpb_encoder_add_enum(&encoder, test_PhoneNumber_type, TEST_PHONENUMBER_MOBILE);
    lwpb_encoder_nested_end(&encoder);
    lwpb_encoder_nested_start(&encoder, test_Person_phone);
    lwpb_encoder_add_string(&encoder, test_PhoneNumber_number, "+123456789");
    lwpb_encoder_add_enum(&encoder, test_PhoneNumber_type, TEST_PHONENUMBER_HOME);
    lwpb_encoder_nested_end(&encoder);
    lwpb_encoder_nested_start(&encoder, test_Person_phone);
    lwpb_encoder_add_string(&encoder, test_PhoneNumber_number, "++123456789");
    lwpb_encoder_add_enum(&encoder, test_PhoneNumber_type, TEST_PHONENUMBER_WORK);
    lwpb_encoder_nested_end(&encoder);
    len = lwpb_encoder_finish(&encoder);

    LWPB_DIAG_PRINTF("encoded message length = %d\n", len);
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    ret = lwpb_decoder_decode(&decoder, test_Person, &buf, len, NULL);
    
    LWPB_DIAG_PRINTF("ret = %d\n", ret);
    
    return 0;
}
