#ifndef OPTISMS_PB_H_
#define OPTISMS_PB_H_

#include "lib/lwpb.h"

#define MESSAGE_NETWORKCONFIGURATION                    0
#define MESSAGE_SETCONFIGREQUEST                        1

#define FIELD_NETWORKCONFIGURATION_NUMBER               1
#define FIELD_NETWORKCONFIGURATION_APN                  2
#define FIELD_NETWORKCONFIGURATION_USERNAME             3
#define FIELD_NETWORKCONFIGURATION_PASSWORD             4
#define FIELD_NETWORKCONFIGURATION_USE_PEER_DNS         5
#define FIELD_NETWORKCONFIGURATION_DNS1                 6
#define FIELD_NETWORKCONFIGURATION_DNS2                 7

#define FIELD_SETCONFIGREQUEST_NETWORK                  1

const struct lwpb_field_desc networkconfiguration_fields[] = {
    {
        .name = "number",
        .id = FIELD_NETWORKCONFIGURATION_NUMBER,
        .typ = LWPB_STRING,
    },
    {
        .name = "apn",
        .id = FIELD_NETWORKCONFIGURATION_APN,
        .typ = LWPB_STRING,
    },
    {
        .name = "username",
        .id = FIELD_NETWORKCONFIGURATION_USERNAME,
        .typ = LWPB_STRING,
    },
    {
        .name = "password",
        .id = FIELD_NETWORKCONFIGURATION_PASSWORD,
        .typ = LWPB_STRING,
    },
    {
        .name = "use_peer_dns",
        .id = FIELD_NETWORKCONFIGURATION_USE_PEER_DNS,
        .typ = LWPB_BOOL,
    },
    {
        .name = "dns1",
        .id = FIELD_NETWORKCONFIGURATION_DNS1,
        .typ = LWPB_FIXED32,
    },
    {
        .name = "dns2",
        .id = FIELD_NETWORKCONFIGURATION_DNS2,
        .typ = LWPB_FIXED32,
    },
};

const struct lwpb_field_desc setconfigrequest_fields[] = {
    {
        .name = "network",
        .id = FIELD_SETCONFIGREQUEST_NETWORK,
        .typ = LWPB_MESSAGE + 0,
    },
};

const struct lwpb_msg_desc optisms_messages[] = {
    {
        .name = "NetworkConfiguration",
        .num_fields = 7,
        .fields = networkconfiguration_fields,
    },
    {
        .name = "SetConfigRequest",
        .num_fields = 1,
        .fields = setconfigrequest_fields,
    }
};

const lwpb_dict_t optisms_dict = optisms_messages;

#endif /* OPTISMS_PB_H_ */
