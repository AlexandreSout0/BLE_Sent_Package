/*
 __    __   __   _______ .______      ______   ___   ___  __   _______    ______   
|  |  |  | |  | |       \ |   _  \   /  __  \  \  \ /  / |  | |       \  /  __  \  
|  |__|  | |  | |  .--.  ||  |_)  | |  |  |  |  \  V  /  |  | |  .--.  ||  |  |  | 
|   __   | |  | |  |  |  ||      /  |  |  |  |   >   <   |  | |  |  |  ||  |  |  | 
|  |  |  | |  | |  '--'  ||  |\  \ .|  `--'  |  /  .  \  |  | |  '--'  ||  `--'  | 
|__|  |__| |__| |_______/ | _| `._|  \______/  /__/ \__\ |__| |_______/  \______/  
                                                                    Alexandre Souto 
   Bluetooth Low Energy
 
   Compilador: Visual Studio 2022

   Autor: Alexandre Souto
   Data:  Setember 2022 
*/

// TCP server at port 80 will respond to HTTP requests

#include <Arduino.h>
<<<<<<< HEAD
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include "AsyncTCP.h"


#ifndef CONFIG_H
#define CONFIG_H

#define SSID "SN-VIRTUAL-OBC"
#define PASSWORD "152152153"

#define SERVER_HOST_NAME "OBC_SERVER"

#define TCP_SERVER_PORT 9002
#define DNS_PORT 53

#define READ_INTERVAL     2000
#define LEDPIN               2
#define TIMER_INTERVAL 1000000 // 1 SEGUNDO


#endif // CONFIG_H

static void sentDataClient_task(void *args);

AsyncClient *conectado;

unsigned int blinkMillis = 0;
unsigned int readMillis = 0;


String lastPackage = "";
String chave = "$POK";

String Gerador_de_Checksum(String package);
void Sent_Package();

bool freedom = false;
int32_t clientes = 0;

static DNSServer DNS;

static void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
	Serial.printf("\n data received from client %s \n", client->remoteIP().toString().c_str()); // Print IP
	//Serial.write((uint8_t *)data, len); //
  //Serial.printf(&data);

  //Serial.println( " [" + String((char*)data) + "]" );


//strstr
//strcmp
//har recebido [] = String((char*)data);

char chave []  = "$POK!";
char *ponteiro;

// procura pela string str dentro da string palavra
ponteiro = strstr((char *)data,chave);

// se ponteiro for diferente de null, imprime 3 caracteres (dia)
if(ponteiro != nullptr)
{  
  freedom = true;
  Serial.printf("\n%c%c%c\n", *ponteiro, *(ponteiro+1), *(ponteiro+2));
}
else
{
  return;
}


	//our big json string test
	//String jsonString = "{\"testeteste\":5}";
	// reply to client

}

static void handleError(void *arg, AsyncClient *client, int8_t error)
{
	Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleDisconnect(void *arg, AsyncClient *client)
{
	Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
  clientes--;
}

static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
	Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

static void handleNewClient(void *arg, AsyncClient *client)
{
	Serial.printf("\n new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());
  clientes++;
  conectado = client;
	// register events
	client->onData(&handleData, NULL);
	client->onError(&handleError, NULL);
	client->onDisconnect(&handleDisconnect, NULL);
	client->onTimeout(&handleTimeOut, NULL);

}


void setup()
{

  xTaskCreatePinnedToCore(sentDataClient_task,"Rx server data",10000,nullptr,3,nullptr,1);

	Serial.begin(9600);
  pinMode(LEDPIN,OUTPUT); //Define pino como saida (led azul da devkit)

	// create access point
	while (!WiFi.softAP(SSID, PASSWORD, 6, false, 15))
	{
		delay(500);
		Serial.print(".");
	}

    if (!MDNS.begin("OBC_SERVER")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }

    MDNS.addService("OBC", "tcp", 9002);


	// start dns server
	if (!DNS.start(DNS_PORT, SERVER_HOST_NAME, WiFi.softAPIP()))
		Serial.printf("\n failed to start dns service \n");

	AsyncServer *server = new AsyncServer(TCP_SERVER_PORT); // start listening on tcp port 7050
	server->onClient(&handleNewClient, server);
	server->begin();
}




void loop()
{

	  DNS.processNextRequest();
    
    if (readMillis == 0 || (millis() - readMillis) >= READ_INTERVAL)
    {
      readMillis = millis();
    }

    if (clientes == 0)
    {
      if(blinkMillis == 0 || (millis() - blinkMillis) >= 500)
      {
        digitalWrite(LEDPIN, !digitalRead(LEDPIN));
        blinkMillis = millis();
      }
    }
    else
    {
      digitalWrite(LEDPIN,HIGH);
    }
  
}

static void sentDataClient_task(void *args){
    for(;;)
    { // Loop infinito

      if (freedom == true)
      {
        Sent_Package();
      }
    vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    vTaskDelete(nullptr);
}

struct obc_frame 
{
  unsigned int rpm ;
  unsigned int digital1;
  unsigned int digital2;
  unsigned int digital3;
  unsigned int digital4;
  unsigned int pulse1;
}frame = {1,0,0,0,0,0};


void Sent_Package()
{

  String package;

  frame.digital1 =  random(2);
  frame.digital2 =  random(2);
  frame.digital3 =  random(2);
  frame.digital4 =  random(2);
  frame.rpm      =  random(1000);
  frame.pulse1   =  random(100);
  
 
  package = "$ALX,";
  //$ALX,600,0,0,0,0,0,checksum\r\n
  //Serial.printf("Digital 1: %d | Digital 2: %d | Digital 3: %d  | Digital 4: %d | RPM: %d | Pulse: %d \n", frame.digital1,frame.digital2,frame.digital3,frame.digital4,frame.rpm,frame.pulse1);

  package = (package + frame.rpm + "," + frame.digital1 + "," + frame.digital2 + "," + frame.digital3 + "," + frame.digital4 + "," + frame.pulse1 + ",");
  
  String temp_checksum = Gerador_de_Checksum(package);
  
  package = (package + temp_checksum + "\r\n");
  Serial.print(package);

  if (package != lastPackage)
  {
      //pacote -> setValue(package.c_str());
      //pacote -> notify(); //notifica que houve alterações no pacote
      lastPackage = package;
  }

  if (conectado->space() > strlen(package.c_str()) && conectado->canSend())
  {
    conectado->add(package.c_str(), strlen(package.c_str()));
    conectado->send();
  }


}



String Gerador_de_Checksum(String package)
{
  String checksum = "";
  int DV = 0;
  int tamanho = package.length();            
  for (int i = 0; i < tamanho; i++)
  {
    DV ^= package[i];   // bitwise XOR
  }

  String hexValue = String(DV, HEX) + "\0"; // conver DV para hexadecimal
  
  if (hexValue.length() == 1)
    checksum = "0" + hexValue;
  if (hexValue.length() == 2)
    checksum = hexValue;  
  if (hexValue.length() == 3)
    checksum = "0" + hexValue[2];
  if (hexValue.length() == 4)
    checksum = (hexValue[2] + hexValue[4]);
  if (hexValue == "0")
    checksum = "00"; 
                         
  return checksum;  
}







/*
#include <Arduino.h>
#include <WiFi.h>
=======
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
>>>>>>> parent of 2010edd (Update main.cpp)
#include "analogRead.hpp"

#define READ_INTERVAL     2000
#define LEDPIN               2
#define TIMER_INTERVAL 1000000 // 1 SEGUNDO

#define SERVICE_UUID "0716bf69-27fa-44bd-b636-4ab49725c6b0"
#define PACOTE_UUID  "0716bf69-27fa-44bd-b636-4ab49725c6b1"
#define RX_UUID      "4ac8a682-9736-4e5d-932b-e9b31405049c"

#define PIN_DIGITAL_1  5
#define PIN_DIGITAL_2 18
#define PIN_DIGITAL_3 19
#define PIN_DIGITAL_4 21
#define PIN_RPM       34
#define PIN_PULSE_1   35



String lastPackage = "";
String password = "$POK";

int devicesConnected = 0; //Contador de usuários conectados

unsigned int blinkMillis = 0;
unsigned int readMillis = 0;

int flag_retorno = 0;

volatile int interruptCounter;
int totalInterruptCounter;

struct obc_frame 
{
  unsigned int rpm ;
  unsigned int digital1;
  unsigned int digital2;
  unsigned int digital3;
  unsigned int digital4;
  unsigned int pulse1;
}frame = {1,0,0,0,0,0};


analog_read pins(PIN_DIGITAL_1,PIN_DIGITAL_2,PIN_DIGITAL_3,PIN_DIGITAL_4,PIN_RPM,PIN_PULSE_1);


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

BLEServer *server = nullptr; //Ponteiro para uma variável tipo BLEserver
BLECharacteristic *pacote = nullptr; //Ponteiro para caracteristicas do serviço do periferico
BLECharacteristic *pacote_rx = nullptr; //Ponteiro para caracteristicas do serviço do periferico
BLEService *service = nullptr; //Ponteiro para variável tipo BLEService


void printteste(std::string teste)
{
  Serial.println(teste.c_str());

}

void testeReset()
{
  Serial.println("Restarting ...");
  delay(2000);
  ESP.restart();
}

//callback  para envendos das características
class CharacteristicCallbacks: public BLECharacteristicCallbacks 
{
    std::string buffer = "";

    void onWrite(BLECharacteristic *pacote_rx) 
    {
      //retorna ponteiro para o registrador contendo o valor atual da caracteristica
      std::string rxValue = pacote_rx->getValue(); 
      //verifica se existe dados (tamanho maior que zero)
      buffer += rxValue.c_str();
      //Serial.print(rxValue.c_str());
      
      std::string rxValueCheck = "!";
      rxValueCheck.c_str();
      
      if (rxValue == rxValueCheck)
      {
       // Serial.print("passou aqui");
      }

      printteste(buffer.c_str());

      if(buffer == "$POK!")
      {
        Serial.print("passou aqui");
        flag_retorno = 1;
        buffer = "";
      }
      else
      {
        buffer = "";
      }
  }

};


class ServerCallbacks: public BLEServerCallbacks // Classe para herdar os serviços de callback BLE
{
  void onConnect(BLEServer *s)
  {
    devicesConnected ++; // quando um usuario se conecta soma mais na variavel
    BLEDevice::startAdvertising(); // Mesmo que esteja alguém conectado o Advertinsing é chamado novamente e permite conecções com outros dispositivos simultaneos
    Serial.println("Device Connected");
  }

  void onDisconnect(BLEServer *s)
  {
    devicesConnected--; // quando um usuario se desconecta subtrai um na variavel
    Serial.println("Device Disconnected");
    flag_retorno = 0;
    testeReset();
    service -> stop();
  }
};

void IRAM_ATTR onTimer();
void sense();
String Gerador_de_Checksum(String package);

void setup() 
{

  Serial.begin(9600);
  // Rotina de interrupção
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer,TIMER_INTERVAL, true);
  timerAlarmEnable(timer);


  Serial.println("Starting ...");
  pinMode(LEDPIN,OUTPUT); //Define pino como saida (led azul da devkit)

  BLEDevice::init("OBC"); //inicio o dispositivo/Periferico
  server = BLEDevice::createServer(); //crio um servidor e coloco seu endereço no ponteiro server

  //======= Callback BLE ======= //
  server -> setCallbacks(new ServerCallbacks()); // cria uma instancia do serviço de callback

  //======= Serviços do Periferico BLE ======= //
  service = server -> createService(SERVICE_UUID); //crio um serviço com o UUID e guardo seu endereço no ponteiro
  pacote = service -> createCharacteristic(  //Configurar Características
    PACOTE_UUID,
    BLECharacteristic::PROPERTY_READ |    //Habilita a leitura do serviço
    BLECharacteristic::PROPERTY_NOTIFY   // Habilita a assinatura do serviço para receber alteraçoes de pacote
  );

  pacote_rx = service -> createCharacteristic(   // Create a BLE Characteristic para recebimento de dados
                                         RX_UUID,
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_READ
                                       );

  pacote_rx -> setCallbacks(new CharacteristicCallbacks());
  service -> start(); // inicia o serviço

  // ======= Criação do Advertising para poder ser descoberto ======= //
  BLEAdvertising *advertising = nullptr; // Ponteiro para váriavel tipo BLEAdvertising
  advertising = BLEDevice::getAdvertising(); // Crio um advertising e coloco seu endereço no ponteiro advertising
  advertising -> addServiceUUID(SERVICE_UUID);
  advertising -> setScanResponse(false); //Configurações de Advertising
  advertising -> setMinPreferred(0x06); //Configurações de Advertising
  BLEDevice::startAdvertising(); //inicia o Advertising

}

void IRAM_ATTR onTimer() 
{
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);

}

String Gerador_de_Checksum(String package)
{
  String checksum = "";
  int DV = 0;
  int tamanho = package.length();            
  for (int i = 0; i < tamanho; i++)
  {
    DV ^= package[i];   // bitwise XOR
  }

  String hexValue = String(DV, HEX) + "\0"; // conver DV para hexadecimal
  
  if (hexValue.length() == 1)
    checksum = "0" + hexValue;
  if (hexValue.length() == 2)
    checksum = hexValue;  
  if (hexValue.length() == 3)
    checksum = "0" + hexValue[2];
  if (hexValue.length() == 4)
    checksum = (hexValue[2] + hexValue[4]);
  if (hexValue == "0")
    checksum = "00"; 
                         
  return checksum;  
}

void Sent_Package()
{
  String package;

   frame.digital1 =  pins.analog_digital(PIN_DIGITAL_1);
   frame.digital2 =  pins.analog_digital(PIN_DIGITAL_2);
   frame.digital3 =  pins.analog_digital(PIN_DIGITAL_3);
   frame.digital4 =  pins.analog_digital(PIN_DIGITAL_4);
   frame.rpm      = random(1000);// pins.analog_rpm(PIN_RPM);
   frame.pulse1   =  pins.analog_pulse(PIN_PULSE_1);

    
  // frame.digital1 =  random(2);
  // frame.digital2 =  random(2);
  // frame.digital3 =  random(2);
  // frame.digital4 =  random(2);
  // frame.rpm      =  random(1000);
  //  frame.pulse1   =  pins.analog_pulse(PIN_PULSE_1);
  
 
  package = "$ALX,";
   //$ALX,600,0,0,0,0,0,checksum\r\n

  //Serial.printf("Digital 1: %d | Digital 2: %d | Digital 3: %d  | Digital 4: %d | RPM: %d | Pulse: %d \n", frame.digital1,frame.digital2,frame.digital3,frame.digital4,frame.rpm,frame.pulse1);

  package = (package + frame.rpm + "," + frame.digital1 + "," + frame.digital2 + "," + frame.digital3 + "," + frame.digital4 + "," + frame.pulse1 + ",");
  
  String temp_checksum = Gerador_de_Checksum(package);
  
  package = (package + temp_checksum + "\r\n");
  Serial.print(package);

  if (package != lastPackage)
  {
      pacote -> setValue(package.c_str());
      pacote -> notify(); //notifica que houve alterações no pacote
      lastPackage = package;
  }

}

void loop() 
{

  if (interruptCounter > 0) 
  {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    if (flag_retorno == 1) //flag_retorno == 1
    {
      Sent_Package();
    }

   totalInterruptCounter++;

  }

  if (readMillis == 0 || (millis() - readMillis) >= READ_INTERVAL)
  {
    readMillis = millis();
  }

  if (!devicesConnected)
  {
    if(blinkMillis == 0 || (millis() - blinkMillis) >= 1000)
    {
      digitalWrite(LEDPIN, !digitalRead(LEDPIN));
      blinkMillis = millis();
    }
  }
  else
  {
    digitalWrite(LEDPIN,HIGH);
  }


}

*/




