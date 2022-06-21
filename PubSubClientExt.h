#include <PubSubClient.h>
class PubSubClientExt : public PubSubClient {

  private:
    char mesg[MQTT_MAX_PACKET_SIZE];
    char topic[MQTT_MAX_PACKET_SIZE / 2];
    int len;
  public:
    PubSubClientExt(Client&client)
      : PubSubClient{client}
    {}

    void init_send(char *t) {
      mesg[0] = '{';
      len = 1;
      sprintf(topic, "/v1.6/devices/%s", t);
    }
    int add_point(char *variavel, int valor) {
      if (len + strlen(variavel) + 15 > MQTT_MAX_PACKET_SIZE)
        return (0);
      sprintf(mesg + len, "\"%s\":%d,", variavel, valor);
      len = strlen(mesg);
      return(1);
    }
    int add_point(char *variavel, float valor) {
      if (len + strlen(variavel) + 15 > MQTT_MAX_PACKET_SIZE)
        return (0);
      sprintf(mesg + len, "\"%s\":%.3f,", variavel, valor);
      len = strlen(mesg);
      return(1);
    }
    int add_point(char *variavel, double valor) {
      if (len + strlen(variavel) + 15 > MQTT_MAX_PACKET_SIZE)
        return (0);
      sprintf(mesg + len, "\"%s\":%.3f,", variavel, valor);
      len = strlen(mesg);
      return(1);
    }

    int send_all(bool debug) {
      int ret;
      if (!len)
        return (0);
      mesg[len - 1] = '}';
      mesg[len] = 0;
      ret = publish(topic, mesg);
      if (debug) {
        Serial.print(topic);
        Serial.print(" ");
        Serial.println(mesg);
      }
      return (ret);
    }
    int subscribeExt(char *device, char * variavel) {
      sprintf(mesg, "/v1.6/devices/%s/%s/lv", device, variavel);
      this->subscribe(mesg, 1);
      return(1);
    }
    int subscribeExt(char *device, char * variavel, bool mostra) {
      subscribeExt(device, variavel);
      if (mostra)
        Serial.println(mesg);
        return(1);
    }
    bool verify(char *mesg, char *device, char *variavel) {
      int len = strlen(device);
      if (strncmp(mesg + 14, device, len))
        return (false);
      if (strncmp(mesg + 15 + len, variavel, strlen(variavel)))
        return (false);
      return (true);

    }


};

int get_int(byte *payload, int len)
{
  char str[len + 1];
  memcpy(str, payload, len);
  str[len] = 0;
  return (atoi(str));

}
