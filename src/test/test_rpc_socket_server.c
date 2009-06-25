
#include <stdio.h>

#include <lwpb/lwpb.h>
#include <lwpb/rpc/socket_server.h>

#include "generated/test_rpc_pb2.h"

// Server handlers

static lwpb_err_t server_request_handler(
    struct lwpb_server *server, const struct lwpb_method_desc *method_desc,
    const struct lwpb_msg_desc *req_desc, void *req_buf, size_t req_len,
    const struct lwpb_msg_desc *res_desc, void *res_buf, size_t *res_len,
    void *arg)
{
    struct lwpb_decoder decoder;
    struct lwpb_encoder encoder;
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    
    lwpb_encoder_init(&encoder);
    
    if (method_desc == test_Search_search_by_name) {
        printf("Server: Received request\n");
        lwpb_decoder_decode(&decoder, req_desc, req_buf, req_len, NULL);
        
        printf("Server: Preparing response\n");
        lwpb_encoder_start(&encoder, test_LookupResult, res_buf, *res_len);
        lwpb_encoder_nested_start(&encoder, test_LookupResult_person);
        lwpb_encoder_add_string(&encoder, test_Person_name, "Simon Kallweit");
        lwpb_encoder_add_int32(&encoder, test_Person_id, 123);
        lwpb_encoder_add_string(&encoder, test_Person_email, "simon.kallweit@intefo.ch");
        lwpb_encoder_nested_start(&encoder, test_Person_phone);
        lwpb_encoder_add_string(&encoder, test_PhoneNumber_number, "123456789");
        lwpb_encoder_add_enum(&encoder, test_PhoneNumber_type, TEST_PHONENUMBER_MOBILE);
        lwpb_encoder_nested_end(&encoder);
        lwpb_encoder_nested_end(&encoder);
        *res_len = lwpb_encoder_finish(&encoder);
        
        return LWPB_ERR_OK;
    }

    *res_len = 0;
    return LWPB_ERR_OK;
}

static const struct lwpb_service_desc *service_list[] = {
    test_Search, NULL,
};

int main()
{
    lwpb_err_t ret;
    struct lwpb_transport_socket_server transport_socket_server;
    lwpb_transport_t transport;
    struct lwpb_server server;
    
    transport = lwpb_transport_socket_server_init(&transport_socket_server);
    
    lwpb_server_init(&server, service_list, transport);
    lwpb_server_handler(&server, server_request_handler);
    
    ret = lwpb_transport_socket_server_open(transport, "localhost", 12345);
    if (ret != LWPB_ERR_OK) {
        printf("Cannot open socket server\n");
        return 1;
    }
    
    while (1) {
        ret = lwpb_transport_socket_server_update(transport);
        if (ret != LWPB_ERR_OK) {
            printf("Socket server failed\n");
            return 1;
        }
    }
    
    lwpb_transport_socket_server_close(transport);
    
    return 0;
}
