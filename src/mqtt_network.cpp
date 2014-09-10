#include "mqtt_network.h"

MQTTNetwork::MQTTNetwork(char* brokerHost,
                         uint16_t brokerPort) : host(brokerHost), port(brokerPort)
{
    client = new TCPClient();
}

MQTTNetwork::~MQTTNetwork()
{
    client->stop();
    delete client;
}

int MQTTNetwork::connect(void)
{
    return client->connect(this->host, this->port);
}

void MQTTNetwork::disconnect(void)
{
    client->stop();
}

bool MQTTNetwork::connected(void)
{
    return client->status() == 1;
}

bool MQTTNetwork::available(void)
{
    return client->available() > 0;
}

// Note this implementation does not respect the timeout
int MQTTNetwork::write(char *buf, size_t length, system_tick_t timeout_ms)
{
    DEBUG("Attempting to write %d chars to MQTT broker", length);
    return client->write((const uint8_t*)buf, length);
}

int MQTTNetwork::read(char *buf, size_t length, system_tick_t timeout_ms)
{
    Timer timer = Timer(timeout_ms);
    int rc;
    size_t recvd = 0;

    while(!timer.expired() && recvd < length){
        rc = client->read((uint8_t*)buf, length - recvd);
        if (rc > 0){
            recvd += rc;
        }
    }

    DEBUG("Read %d bytes from MQTT broker (of %d requested)", recvd, length);

    return recvd;
}
