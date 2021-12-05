#ifndef BUILDCONFIG_H
#define BUILDCONFIG_H

extern "C"
{
    #include <esp_log.h>
}

#pragma GCC diagnostic ignored "-Wunused-variable"

#ifndef DEVICE_FIRMWARE_VERSION
    #define DEVICE_FIRMWARE_VERSION     "1.0"
#endif

#ifndef DEVICE_FIRMWARE_BUILD
    #define DEVICE_FIRMWARE_BUILD       "-"
#endif

#define DISABLE_ENERGY_METER            0
#define DISABLE_TLS_CA_VALIDATION       1
#define ALLOW_3RD_PARTY_FIRMWARE        0
#define FIRMWARE_MAGIC_BYTES            "2LOGSW"

#define DEVICE_LOG_LEVEL                ESP_LOG_VERBOSE
#define SYSTEM_MONITORING               0

#define	DEVICE_DEBUGGING                0
#define DEBUGGING_DEVICE_ID             "DUDE"
#define DUMP_TASK_STATS                 0
#define OVERRIDE_CONFIG                 0
#define OVERRIDE_WIFI                   0

#if OVERRIDE_CONFIG == 1
    #define SERVER_URL                  "wss://fritzkatz.2log.io"
#endif

#define MEMORY_DEBUGGING                0
#define MAX_DEBUGGING_TASKS_NUMBER      15

#define SWITCH_ACTOR_GPIO               15
#define SWITCH_BUTTON_GPIO              13
#define LED_GPIO                        2
#define WIFI_SSID                       ""
#define WIFI_PASSWORD                   ""

namespace
{
    const int   PING_TIMEOUT_TIMER                              = 15000; // ms
    const int   PING_TIMEOUT                                    = 60000; // ms

    const int   CONNECTION_RETRY_LIMIT_UNTIL_DELAY              = 6;
    const int   CONNECTION_RETRY_LIMIT_UNTIL_WIFI_RECONNECT     = 24;
    const int   CONNECTION_RETRY_DELAY_TIME                     = 60000; // ms

    const int   CONFIGURATION_MAX_RETRY                         = 4;
    const int   CONFIGURATION_RETRY_DELAY                       = 5000;	// ms
    const int   SEND_DATA_INTERVAL                              = 2000; //ms

    const char* CONFIGURATION_WIFI_SSID                         = "I'm a Switch";
    const char* CONFIGURATION_WIFI_PWD                          = "";

    const char*	DEVICE_TYPE                                     = "2log-Switch";
}

// Fallback values. Do not edit and use configuration above.

#if defined __has_include
    #if __has_include("confidential-password.h")
        #include "confidential-password.h"
    #endif
#endif

#if OVERRIDE_WIFI == 1

    #ifndef WIFI_SSID
        #error "Define WIFI_SSID in confidential-password.h"
    #endif

    #ifndef WIFI_PASSWORD
        #error "Define WIFI_PASSWORD in confidential-password.h"
    #endif

#endif

#ifndef DEVICE_LOG_LEVEL
    #define DEVICE_LOG_LEVEL    ESP_LOG_NONE
#endif

#ifndef SYSTEM_MONITORING
    #define SYSTEM_MONITORING   0
#endif

#endif
