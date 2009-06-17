
#include <stdio.h>

#include <lwpb/lwpb.h>
#include "optisms.pb.h"


int main()
{
    FILE *f;
    char buf[4096];
    size_t len;
    
    struct lwpb_decoder decoder;
    struct lwpb_encoder encoder;
    
    f = fopen("test.dat", "r");
    if (!f) {
        printf("Cannot read 'test.dat'\n");
        return 0;
    }
    len = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    lwpb_decoder_decode(&decoder, &buf, len, MESSAGE_SETCONFIGREQUEST);
    
    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, buf, sizeof(buf));
    lwpb_encoder_msg_start(&encoder, MESSAGE_NETWORKCONFIGURATION);
    lwpb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_NUMBER, "just a number");
    lwpb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_APN, "just a test");
    lwpb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_USERNAME, "gprs");
    lwpb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_PASSWORD, "gprs");
    lwpb_encoder_add_bool(&encoder, FIELD_NETWORKCONFIGURATION_USE_PEER_DNS, 1);
    lwpb_encoder_msg_end(&encoder);
    len = lwpb_encoder_finish(&encoder);
    
    printf("encoded message length = %d\n", len);
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    lwpb_decoder_decode(&decoder, &buf, len, MESSAGE_NETWORKCONFIGURATION);
    
    return 0;
}
