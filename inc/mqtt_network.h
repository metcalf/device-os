#ifndef __MQTT_NETWORK_H
#define __MQTT_NETWORK_H

#include <stdint.h>

#include "spark_wiring_tcpclient.h"
#include "timer.h"

class MQTTNetwork {
public:
    MQTTNetwork(char* brokerHost,
                    uint16_t brokerPort);
    ~MQTTNetwork(void);

    int connect(void);
    void disconnect(void);
    bool connected(void);
    bool available(void);

    // Interface for MQTTClient
    int write(char* buf, size_t length, system_tick_t timeout_ms);
    int read(char* buf, size_t length, system_tick_t timeout_ms);

private:
    TCPClient* client;
    char* host;
    uint16_t port;
};

#endif  /* __MQTT_NETWORK_H */
