
#include <stdio.h>

#include <lwpb/lwpb.h>
#include <lwpb/rpc/socket_client.h>

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
    
    return lwpb_decoder_decode(&decoder, msg_desc, buf, len, NULL);
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

static const struct lwpb_service_desc *service_list[] = {
    test_Search, NULL,
};

int main()
{
    lwpb_err_t ret;
    
    struct lwpb_transport_socket_client service_socket_client;
    lwpb_transport_t service;
    struct lwpb_client client;
    
    service = lwpb_transport_socket_client_init(&service_socket_client);
    
    lwpb_client_init(&client, service);
    lwpb_client_handler(&client,
                        client_request_handler,
                        client_response_handler,
                        client_call_done_handler);
    
    ret = lwpb_transport_socket_client_open(service, "localhost", 12345);
    if (ret != LWPB_ERR_OK) {
        printf("Cannot open socket client\n");
        return 1;
    }
    
    lwpb_client_call(&client, test_Search_search_by_name);
    
    while (1) {
        ret = lwpb_transport_socket_client_update(service);
        if (ret != LWPB_ERR_OK) {
            printf("Socket client failed\n");
            return 1;
        }
    }
    
    lwpb_transport_socket_client_close(service);
 
    return 0;
}
