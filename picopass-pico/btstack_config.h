#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

#define HCI_OUTGOING_PRE_BUFFER_SIZE 4
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4

#define ENABLE_BLE                1
#define ENABLE_LOG_INFO           1
#define ENABLE_LOG_ERROR          1
#define ENABLE_PRINTF_HEXDUMP     1

#define ENABLE_LE_ADVERTISING     1
#define ENABLE_LE_SCANNING        1
#define ENABLE_LE_PERIPHERAL      1
#define ENABLE_LE_CENTRAL         1

#define HCI_INCOMING_PRE_BUFFER_SIZE   6
#define HCI_ACL_PAYLOAD_SIZE           255

#define MAX_NR_HCI_CONNECTIONS         2
#define MAX_NR_L2CAP_CHANNELS          2
#define MAX_NR_L2CAP_SERVICES          1
#define MAX_NR_GATT_CLIENTS            1
#define MAX_NR_GATT_SUBSCRIPTIONS      1

#define MAX_ATT_DB_SIZE                512
#define HAVE_EMBEDDED_TIME_MS          1

#define MAX_NR_LE_DEVICE_DB_ENTRIES    4
#define NVM_NUM_DEVICE_DB_ENTRIES      4

#endif