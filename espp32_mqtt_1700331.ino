#include <UbidotsEsp32Mqtt.h>
#include <UbiConstants.h>
#include <UbiTypes.h>
#include "DHTesp.h"


#include <WiFi.h>             //biblioteca com as funções WiFi
#include "PubSubClientExt.h"  //ficheiro produzido por Luis Figueiredo para facilitar a utilização da "internet das coisas"
//deve ser colocado na mesma pasta do ficheiro .ino que se está a usar
//este ficheiro funciona unicamente para o broker da UBIDOTS. para outros brokers terá que ser alterado
//deve ser previamente instalada a biblioteca PubSubClient.h sem a qual a biblioteca PubSubClientExt.h não funciona


#define WIFISSID "HUAWEI-B311-BA97"                         //nome da rede WiFi a que se vai ligar
#define PASSWORD "HDTdqYtj3MMmbQF3"                         //Password da rede WiFi a que se vai ligar
#define TOKEN "BBFF-da6QEAl7rrlQAk1Oq22YrK5V23aIfE" //TOKEN da UBIDOTS. Deve-se obter do site da UBIDOTS depois de se fazer o registo
#define MQTT_CLIENT_NAME "FGFLQCWY1FMJ"             //12 carateres com letras ou números gerados aleatoriamente;
//devem ser diferentes para cada dispositivo. Não usar esta sequência
//sugestão: usar o site https://www.random.org/strings/

#define DEVICE_LABEL "esp32"                        // Nome do dispositivo que se pretende usar
char mqttBroker[] = "industrial.api.ubidots.com";   //endereço do broker da UBIDOTS
#define PORTO 1883                                  //porto do broker da UBIDOTS
#define INTERVALO 5000                             //Intervalo em milissegundos para o envio de dados para a UBIDOTS 
#define TIME_TO_RECONECT  500                       //tempo entre duas tentativas seguidas para restabelecer a ligação ao broker

DHTesp dht;                         //variavel (objeto) para contolo da ligacao ao DHT11
WiFiClient ubidots;                 //variável (objeto) para controlo da ligação WiFi
PubSubClientExt client(ubidots);    //variável (objeto) para controlo da ligação MQTT

#define pino 25        //pino onde se liga um led para testar o ligar e desligar através do site da UBIDOTS
#define pino_brilho 32  //pino onde se liga um led para testar o controlo do brilho através do site da UBIDOTS
#define canal_pwm 0     //canal PWM que se vai usar para controlar o brilho do led
#define pino_led 12     //led que vai desligar ou ligar consoante o limite da temperatura implementada no UBIDOTS
#define dhtpino 14      //pino em que se encontra ligado o sensor DHT11 

unsigned long ti = 0;                   //variável para controlar o tempo de envio de dados para o broker da UBIDOTS
unsigned long tempo = 0; 
unsigned int limite = 0;                 //variavel global para deliniar o limite da temperatura em que o led liga ou desliga
float temperatura, humidade;             //variavel temperatura e humidade usada para registar as mesmas 
unsigned int histerysis = 2; 


void setup()
{
  int contador = 0;
  WiFi.disconnect();

  Serial.begin(115200);                 //inicia a porta série à velocidade de 115200bps
  WiFi.begin(WIFISSID, PASSWORD);       //inicia a ligação à rede WIFI com o nome WIFISSID e a password  PASSWORD

  Serial.print("\nEfetuando a ligação à rede  WiFi...");

  while (WiFi.status() != WL_CONNECTED) //ciclo para aguardar que seja estabelecia a ligação à rede WIFI
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nLigação WiFi estabelecida com sucesso");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());      //imprime o endereço IP privado atribuído ao ESP32


  client.setServer(mqttBroker, PORTO); //Configura o endereço do broker da UBIDOTS
  client.setCallback(callback);        //estabelece a função callback para ser chamada sempre que o broker enviar informações para este dispositivo
  //será dentro desta função que se analisará a informação recebida e em função dessa informação se decidirá o que fazer


  pinMode(pino, OUTPUT);                //define-se o pino como OUTPUT
  pinMode(pino_brilho, OUTPUT); //define-se o pino_brilho como OUTPUT
  pinMode(pino_led, OUTPUT);
  ledcSetup(canal_pwm, 15000, 8);       //define-se o canal_pwm com a frequência de 15000Hz e 8 bits de resolução (0 a 255)
  ledcAttachPin(pino_brilho, canal_pwm); //associa o canal_pwm ao pino_brilho. a partir deste momento o pino_brilho é controlado pelo canal_pwm

  ti = millis();                        //inicia-se o contador de tempo inicial para o envio de dados para o broker.

  dht.setup(dhtpino, DHTesp::DHT11);   //associa o sensor ao pino 
  Serial.println("DHT11 inicializado");
}

//int x = 0;                             //variável para produzir ondas senosoidais
void loop()
{
  //float seno, cosseno;

  if (!client.connected())              //se deixou de haver uma ligação ao broker volta-se a estabelecê-la
    if (reconnect())
      subscrever();                      //subscreve as variáveis do broker que pretende ser notificado

  if (millis() - ti >= INTERVALO)       //se já passou o tempo definido para enviar dados
  {
    ti = millis();                      //atualiza-se o tempo inicial
    //seno = sin(2 * PI * x / 30);        //calcula-se o valor de seno em função do valor de x
    //cosseno = cos(2 * PI * x / 30);     //calcula-se o valor de cosseno em função do valor de x
    //x++;  //atualiza-se a variável x para da próxima vez se calcular um valor diferente

    temperatura = dht.getTemperature();   // le temperatura 
    humidade = dht.getHumidity();
    Serial.printf("temperatura =%.1f humidade=%.1f\n", temperatura, humidade);


    client.init_send(DEVICE_LABEL); //inicia o processo de envio de dados para o broker do dispositivo DEVICE_LABEL
    //client.add_point("seno", seno);     //adiciona um ponto ao broker da variável com o nome seno cujo valor está na variável seno
    //client.add_point("cosseno", cosseno); //adiciona um ponto ao broker da variável com o nome cosseno cujo valor está na variável cosseno
    //client.add_point("soma", cosseno + seno); //adiciona um ponto ao broker da variável com o nome soma cujo valor é cosseno+seno
    
    client.add_point("temperatura", temperatura);  //adiciona um ponto ao broker da variavel com nome temperatura cujo valor e temperatura
    client.add_point("humidade", humidade);   //adiciona um ponto ao broker da variavel com nome humidade cujo valor e a humidade
    
    client.send_all(true);              //envia todos os pontos para o broker.
    //a mensagem a enviar terá o seguinte formato: /v1.6/devices/esp32 {"seno":0.743,"cosseno":0.669,"soma":1.412}
    //se o parâmetro for true a função envia também para a porta série os dados enviados
  }

  if(temperatura >= (limite + histerysis )){  //histerysis implementada 
    digitalWrite(pino_led, HIGH);
  }else if (temperatura < (limite + histerysis)) {   
    digitalWrite(pino_led, LOW);
  }

  client.loop();                        //função que é fundamental ser chamada o máximo de vezes possivel. Não deve haver no loop qualquer delay
}




void callback(char *topic, byte *payload, unsigned int length) {
  //função que será chamada sempre que o site da UBIDOTS enviar dados para o dispositivo
  //topic: variável que conterá a identificação do dispositivo e da variável do site da UBIDOTS
  //exemplo:  /v1.6/devices/esp32/brilho/lv     neste caso esp32 é o dispositivo, brilho é a variável
  //payload: variável que conterá a informação enviado pelo site da UBIDOTS. A informação vem em carateres ASCII não terminados com \0
  //exemplo:  128
  //length: número de bytes que vêm na variável payload
  //exemplo:  3
  int valor = get_int(payload, length);             //função que converte os carateres ASCII recebidos num número
  if (client.verify(topic, DEVICE_LABEL, "led")) {  //função que verifica se o dispositivo e a variável são as indicadas em DEVICE_LABEL e "led"
    Serial.print("recebi do led do esp32 o valor ");
    Serial.println(valor);                          //imprime o valor do led recebido do site da UBIDOTS
    digitalWrite(pino, valor);                      //escreve o valor recebido no pino do led
  } else if (client.verify(topic, DEVICE_LABEL, "brilho")) { //função que verifica se o dispositivo e a variável são as indicadas em DEVICE_LABEL e "brilho"
    Serial.print("recebi do brilho do esp32 o valor ");
    Serial.println(valor);                          //imprime o valor do brilho recebido do site da UBIDOTS
    ledcWrite(canal_pwm, valor); //escreve o valor recebido no canal_pwm que controla o brilho do led
  }else if (client.verify(topic, DEVICE_LABEL, "controlatemp")){
    Serial.print("recebi do controlatemp do esp32 o valor");
    Serial.println(valor);
    limite = valor;
  }
}

void subscrever() {
  client.subscribeExt(DEVICE_LABEL, "led", true);     //subscreve a variável led do DEVICE_LABEL. Se o último parâmetro for true envia para a porta série a mensagem da subscrição
  //o broker passará a enviar a este dispositivo qualquer alteração da variável led do dispositivo DEVICE_LABEL
  client.subscribeExt(DEVICE_LABEL, "brilho", true);  //subscreve a variável brilho do DEVICE_LABEL. Se o último parâmetro for true envia para a porta série a mensagem da subscrição
  //o broker passará a enviar a este dispositivo qualquer alteração da variável brilho do dispositivo DEVICE_LABEL
  client.subscribeExt(DEVICE_LABEL, "controlatemp", true);  //subscreve a variável brilho do DEVICE_LABEL. Se o último parâmetro for true envia para a porta série a mensagem da subscrição
  //o broker passará a enviar a este dispositivo qualquer alteração da variável brilho do dispositivo DEVICE_LABEL
}


//função para restabelecer uma ligação ao broker da UBIDOTS
bool reconnect()
{
  static unsigned long time_reconect = 0;         //variável para controlar o tempo para voltar a tentar estabelecer a ligação
  static bool first_time = true;                  //variável para controlar as mensagens a apresentar

  if (millis() - time_reconect >= TIME_TO_RECONECT) { //se já passou o tempo TIME_TO_RECONECT
    time_reconect = millis();                     //atualiza a variável time_reconect com o tempo atual
    if (first_time) {                             //se é a primeira vez escreve uma mensagem
      Serial.println("Estabelecendo ligação MQTT");
      first_time = false;                         //altera a variável first_time para não voltar a escrever a mesma mensage
    }
    else
      Serial.print(".");                         //se não é a primeira vez escreve apenas .

    if (client.connect(MQTT_CLIENT_NAME, TOKEN, ""))  //tenta estabelecer a ligação ao broker
    {
      Serial.println("Connected");       //informa do estabelecimento da ligação ao broker
      first_time = true;                 //coloca a variável first_time a true para ficar pronta para a próxima ligação
      return (true) ;                    //indica que a ligação foi estabelecida
    }
    else
    {
      Serial.print("Erro na ligação MQTT, rc=");  //Indica a ocorrência de um erro na ligação
      Serial.print(client.state());

    }
  }
  return (false);                       //indica que a ligação ainda não foi estabelecida
}
