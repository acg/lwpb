
#include <stdio.h>

#include <lwpb/lwpb.h>
#include <lwpb/rpc/direct/direct.h>

#include "generated/test_rpc_pb2.h"

// Client handlers

static lwpb_err_t client_request_handler(
    struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
    const struct lwpb_msg_desc *msg_desc, void *buf, size_t *len, void *arg)
{
    struct lwpb_encoder encoder;
    
    lwpb_encoder_init(&encoder);
    
    if (method_desc == test_Search_search_by_name) {
        printf("Client: Preparing request\n");
        lwpb_encoder_start(&encoder, msg_desc, buf, *len);
        lwpb_encoder_add_string(&encoder, test_Name_name, "some name");
        *len = lwpb_encoder_finish(&encoder);
        return LWPB_ERR_OK;
    }
    
    *len = 0;
    return LWPB_ERR_OK;
}

static lwpb_err_t client_response_handler(
    struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
    const struct lwpb_msg_desc *msg_desc, void *buf, size_t len, void *arg)
{
    struct lwpb_decoder decoder;
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_use_debug_handlers(&decoder);
    
    if (method_desc == test_Search_search_by_name) {
        printf("Client: Received response\n");
    }
    
    return lwpb_decoder_decode(&decoder, msg_desc, buf, len);
}

static void client_call_done_handler(
    struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
    lwpb_rpc_result_t result, void *arg)
{
    switch (result) {
    case LWPB_RPC_OK: printf("Client: Result = OK\n"); break;
    case LWPB_RPC_NOT_CONNECTED: printf("Client: Result = Not connected\n"); break;
    case LWPB_RPC_FAILED: printf("Client: Result = Failed\n"); break;
    }
}

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
        lwpb_decoder_decode(&decoder, req_desc, req_buf, req_len);
        
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

int main()
{
    lwpb_err_t ret;
    
    struct lwpb_service_direct service_direct;
    lwpb_service_t service;
    struct lwpb_client client;
    struct lwpb_server server;
    
    service = lwpb_service_direct_init(&service_direct);
    
    lwpb_client_init(&client, service);
    lwpb_client_handler(&client,
                        client_request_handler,
                        client_response_handler,
                        client_call_done_handler);
    
    lwpb_server_init(&server, service);
    lwpb_server_handler(&server, server_request_handler);
    
    lwpb_client_call(&client, test_Search_search_by_name);
    
    return 0;
}
