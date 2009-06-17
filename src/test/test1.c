
#include <stdio.h>

#include <lwpb/lwpb.h>

#include "test1_pb2.h"

int main()
{
    FILE *f;
    char buf[4096];
    size_t len;
    lwpb_err_t ret;
    
    struct lwpb_decoder decoder;
    struct lwpb_encoder encoder;
/*    
    f = fopen("test.dat", "r");
    if (!f) {
        printf("Cannot read 'test.dat'\n");
        return 0;
    }
    len = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    lwpb_decoder_decode(&decoder, &buf, len, test_Person);
*/    
    
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

    printf("encoded message length = %d\n", len);
    
    f = fopen("test1.dat", "w");
    if (!f) {
        printf("Cannot write 'test1.dat'\n");
        return 0;
    }
    fwrite(buf, 1, len, f);
    fclose(f);
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    ret = lwpb_decoder_decode(&decoder, test_Person, &buf, len);
    
    printf("ret = %d\n", ret);
    
    return 0;
}
