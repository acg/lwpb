
#include <stdio.h>

#include "lib/lwpb.h"
#include "optisms.pb.h"


int main()
{
    FILE *f;
    char buf[4096];
    size_t len;
    
    struct pb_decoder decoder;
    struct pb_encoder encoder;
    
    f = fopen("test.dat", "r");
    if (!f) {
        printf("Cannot read 'test.dat'\n");
        return 0;
    }
    len = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    
    pb_decoder_init(&decoder, optisms_dict);
    pb_decoder_use_debug_handlers(&decoder);
    pb_decoder_decode(&decoder, &buf, len, MESSAGE_SETCONFIGREQUEST);
    
    pb_encoder_init(&encoder, optisms_dict);
    pb_encoder_start(&encoder, buf, sizeof(buf));
    pb_encoder_msg_start(&encoder, MESSAGE_NETWORKCONFIGURATION);
    pb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_NUMBER, "just a number");
    pb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_APN, "just a test");
    pb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_USERNAME, "gprs");
    pb_encoder_add_string(&encoder, FIELD_NETWORKCONFIGURATION_PASSWORD, "gprs");
    pb_encoder_add_bool(&encoder, FIELD_NETWORKCONFIGURATION_USE_PEER_DNS, 1);
    pb_encoder_msg_end(&encoder);
    len = pb_encoder_finish(&encoder);
    
    printf("encoded message length = %d\n", len);
    
    pb_decoder_init(&decoder, optisms_dict);
    pb_decoder_use_debug_handlers(&decoder);
    pb_decoder_decode(&decoder, &buf, len, MESSAGE_NETWORKCONFIGURATION);
    
    return 0;
}
