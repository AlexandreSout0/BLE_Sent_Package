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

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEAdvertisedDevice.h>
#include "esp_gap_ble_api.h"
#include "analogRead.hpp"
#include <BLEAddress.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdio.h>

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
      std::string rxValue = pacote_rx -> getValue(); 
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

      if(buffer != "$POK!")
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
  void onConnect(BLEServer *s, esp_ble_gatts_cb_param_t *param)
  {
    devicesConnected ++; // quando um usuario se conecta soma mais na variavel
    BLEDevice::startAdvertising(); // Mesmo que esteja alguém conectado o Advertinsing é chamado novamente e permite conecções com outros dispositivos simultaneos
    Serial.println("Device Connected");
   // String address = cliente.getAddress();
		char remoteAddress[18];

		sprintf(
			remoteAddress,
			"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			param->connect.remote_bda[0],
			param->connect.remote_bda[1],
			param->connect.remote_bda[2],
			param->connect.remote_bda[3],
			param->connect.remote_bda[4],
			param->connect.remote_bda[5]
		);

		//ESP_LOGI(LOG_TAG, "myServerCallback onConnect, MAC: %s", remoteAddress);
    Serial.printf(remoteAddress);
  }


  void onDisconnect(BLEServer *s, esp_ble_gatts_cb_param_t *param)
  {
    devicesConnected--; // quando um usuario se desconecta subtrai um na variavel
    Serial.println("Device Disconnected");
    flag_retorno = 0;
    testeReset();
    char remoteAddress[20];

		sprintf(
			remoteAddress,
			"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			param->disconnect.remote_bda[0],
			param->disconnect.remote_bda[1],
			param->disconnect.remote_bda[2],
			param->disconnect.remote_bda[3],
			param->disconnect.remote_bda[4],
			param->disconnect.remote_bda[5]
		);

    Serial.printf(remoteAddress);
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
  pacote = service -> createCharacteristic( PACOTE_UUID, BLECharacteristic::PROPERTY_READ |BLECharacteristic::PROPERTY_NOTIFY); // Habilita a assinatura do serviço para receber alteraçoes de pacote //Configurar Características
  pacote_rx = service -> createCharacteristic( RX_UUID, BLECharacteristic::PROPERTY_WRITE ); // Create a BLE Characteristic para recebimento de dados
  pacote->addDescriptor(new BLE2902());



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

  //  frame.digital1 =  pins.analog_digital(PIN_DIGITAL_1);
  //  frame.digital2 =  pins.analog_digital(PIN_DIGITAL_2);
  //  frame.digital3 =  pins.analog_digital(PIN_DIGITAL_3);
  //  frame.digital4 =  pins.analog_digital(PIN_DIGITAL_4);
  //  frame.rpm      = random(1000);// pins.analog_rpm(PIN_RPM);
  //  frame.pulse1   =  pins.analog_pulse(PIN_PULSE_1);

    
  frame.digital1 =  random(2);
  frame.digital2 =  random(2);
  frame.digital3 =  random(2);
  frame.digital4 =  random(2);
  frame.rpm      =  random(1000);
  frame.pulse1   =  pins.analog_pulse(PIN_PULSE_1);
  
 
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
      pacote -> notify(true); //notifica que houve alterações no pacote
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


