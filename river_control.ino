#include <Ethernet.h>
#include <SPI.h>
#include "RestClient.h"
#include "Wire.h"
#include "Adafruit_BMP085.h"
#include <Ultrasonic.h>
#include <dht.h>

#define dht_dpin A1 //Pino DATA do Sensor ligado na porta Analogica A1

#define TRIGGER_PIN  12//Pinos sensor ultasonico
#define ECHO_PIN     13

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
dht DHT; //Inicializa o sensor

Adafruit_BMP085 bmp;

//Configuracoes Ethernet
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Endereço MAC precisa ser único na rede
  IPAddress ip(192,168,1,2); // Endereço IP, altere para sua rede
  EthernetClient client;
  //Configuracoes do servidor
  const int updateServerInterval = 30 * 1000; // Intervalo para atualização
  char send_data[128]; //variavel para envio dos dados
  //variavéis para controle
  long lastConnectionTime = 0;
  boolean lastConnected = false;
  int failedCounter = 0;

  void setup()
  {
    Serial.begin(9600);
    Ethernet.begin(mac,ip);//inicializa placa Ethernet
    delay(5000);//Aguarda 5 seg antes de acessar as informaÃ§Ãµes do sensor
    bmp.begin();
    Serial.println("Setup!");
  }

  void loop(){
    sensorHigrometro();
    sensorBarometrico();
    sensorUltrasonico();
    //NÃ£o diminuir o valor abaixo. O ideal Ã© a leitura a cada 2 segundos
    delay(2000);  
  }



  void sensorUltrasonico(){
    float cmMsec, inMsec;
    long microsec = ultrasonic.timing();
    cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
    inMsec = ultrasonic.convert(microsec, Ultrasonic::IN);

    Serial.print("MS: ");
    Serial.print(microsec);
    Serial.print(", CM: ");
    Serial.print(cmMsec);
    Serial.print(", IN: ");
    Serial.println(inMsec); 
  }

  void sensorHigrometro(){
    DHT.read11(dht_dpin); //LÃª as informaÃ§Ãµes do sensor
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    char umidade[6];
    char temperatura[6];

    if(isnan(umidade) || isnan(temperatura)){//verifica se ouve leitura do sensor
      Serial.println("Falha de leitura sensor dht");
    }
    else{
      dtostrf(umidade, 1, 2, h);// copia variavel float para variavel char
      dtostrf(temperatura, 1, 2, t);
    }

    sprintf(send_data,"field1=%s&amp;field2=%s", umidade, temperatura);//salva na variavel os dados que serao enviados

    if (!client.connected() &amp;
    &amp; 
    lastConnected)
    {
      Serial.println("...desconectado");
      Serial.println();

      client.stop();
    }

    // Faz Update de dados no servidor
    if(!client.connected() &amp;
    &amp; 
    (millis() - lastConnectionTime &gt; 
    updateServerInterval))
    {
      updateServer(send_data); // envia variavel thingspeak_data para a função updateThingSpeak
      Serial.print("Dados enviados:");
      Serial.println(send_data);// exibe via serial os dados enviados de temperatura e umidade
    }

    // Verifica se o módulo/Shield Ethernet precisa ser reiniciado
    if (failedCounter &gt; 
    3 ) {
      startEthernet();
    }//se o contador de falhas for &gt; 3 , inicia novamente a placa Ethernet

      lastConnected = client.connected();//armazena em lastConnected o horário da última conexão
  }

  /* Serial.print("Umidade = ");
   Serial.print(DHT.humidity);
   Serial.print(" %  ");
   Serial.print("Temperatura = ");
   Serial.print(DHT.temperature); 
   Serial.println(" Celsius  ");
   */
}

void sensorBarometrico(){
  Serial.print("Temperatura = ");
  Serial.print(bmp.readTemperature());
  Serial.print("*C ");

  Serial.print("Pressao = ");
  Serial.print(bmp.readPressure());
  Serial.print(" Pa ");

  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(101500));
  Serial.print(" Metros");

  Serial.println();
}

void updateServer(String dados)//funçao recebe a variavel char contendo os campos com respectivos dados a serem enviados ao servidor thingspeak
{
  if (client.connect(192.168.1.1, 9000))//tenta conexão com o servidor thingSpeakAdrress,porta do servidor
  {
    //cabeçalho de POST via HTTP
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: 192.168.1.1:900\n");//ip do servidor
    client.print("Connection: close\n");   
    client.print("Content-Length: ");
    client.print(tsData.length());//comprimento dos dados a serem enviados
    client.print("\n\n");
    client.print(tsData);//propriamente os dados field1 &amp; field que serão enviados

    lastConnectionTime = millis();

    if (client.connected())
    {
      Serial.println("Conectando ao River Control");
      Serial.println();

      failedCounter = 0;
    }
    else
    {
      failedCounter++;

      Serial.println("Conexão ao Server Control Falhou ("+String(failedCounter, DEC)+")");
      Serial.println();
    }

  }
  else
  {
    failedCounter++;

    Serial.println("Conexão ao Server Control Falhou ("+String(failedCounter, DEC)+")");
    Serial.println();

    lastConnectionTime = millis();
  }
}

void startEthernet()
{
  client.stop();
  Serial.println("Connectando Arduino à rede...");
  Serial.println();
  delay(1000);
  Ethernet.begin(mac,ip);//inicializa placa Ethernet
  delay(1000);
}





