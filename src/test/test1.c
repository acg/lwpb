
#include <stdio.h>

#include <lwpb/lwpb.h>

#include "test1_pb2.h"

int main()
{
    FILE *f;
    char buf[4096];
    size_t len;
    
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
    lwpb_encoder_start(&encoder, buf, sizeof(buf));
    lwpb_encoder_msg_start(&encoder, test_Person);
    lwpb_encoder_add_string(&encoder, test_Person_name, "Simon Kallweit");
    lwpb_encoder_add_int32(&encoder, test_Person_id, 123);
    lwpb_encoder_add_string(&encoder, test_Person_email, "simon.kallweit@intefo.ch");
    lwpb_encoder_msg_end(&encoder);
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
    lwpb_decoder_decode(&decoder, &buf, len, test_Person);
    
    return 0;
}
