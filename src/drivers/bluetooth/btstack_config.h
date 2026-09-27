/*
 * BTstack Configuration for PureOS
 * Lightweight, modular Bluetooth Stack Integration
 */

#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

// Core BTstack Features
#define HAVE_EMBEDDED_TICK
#define HAVE_MALLOC
#define HAVE_BZERO
#define HAVE_MEMCPY
#define HAVE_MEMSET

// Bluetooth Stack Profiles & Layers
#define ENABLE_CLASSIC
#define ENABLE_BLE
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR

// HCI Controller and Transport Settings
#define HCI_ACL_PAYLOAD_SIZE 1021
#define HCI_INCOMING_PRE_BUFFER_SIZE 14
#define MAX_NR_BTM_SCAN_RESULTS 16
#define MAX_NR_HCI_CONNECTIONS 4
#define MAX_NR_L2CAP_CHANNELS 8
#define MAX_NR_L2CAP_SERVICES 4
#define MAX_NR_SM_LOOKUP_ENTRIES 8

// GAP / SDP Profiles
#define ENABLE_SDP
#define ENABLE_RFCOMM
#define ENABLE_HFP_HF
#define ENABLE_A2DP_SINK
#define ENABLE_AVRCP

#endif // BTSTACK_CONFIG_H
