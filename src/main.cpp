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


#define READ_INTERVAL 2000
#define LEDPIN 2

#define SERVICE_UUID "0716bf69-27fa-44bd-b636-4ab49725c6b0"
#define PACOTE_UUID "0716bf69-27fa-44bd-b636-4ab49725c6b1"

int lastRPM = -999;
int devicesConnected = 0; //Contador de usuários conectados

unsigned int blinkMillis = 0;
unsigned int readMillis = 0;

BLEServer *server = nullptr; //Ponteiro para uma variável tipo BLEserver
BLECharacteristic *pacote = nullptr; //Ponteiro para caracteristicas do serviço do periferico


class ServerCallbacks: public BLEServerCallbacks // Classe para herdar os serviços de callback BLE
{
  void onConnect(BLEServer *s)
  {
    devicesConnected ++; // quando um usuario se conecta soma mais na variavel
    BLEDevice::startAdvertising(); // Mesmo que esteja alguém conectado o Advertinsing é chamado novamente e permite conecções com outros dispositivos simultaneos
  }

  void onDisconnect(BLEServer *s)
  {
    devicesConnected--; // quando um usuario se desconecta subtrai um na variavel
  }
};




void setup() {

  Serial.begin(9600);
  Serial.println("Starting ...");
  pinMode(LEDPIN,OUTPUT); //Define pino como saida (led azul da devkit)

  BLEDevice::init("OBC"); //inicio o dispositivo/Periferico
  server = BLEDevice::createServer(); //crio um servidor e coloco seu endereço no ponteiro server

  //======= Callback BLE ======= //
  server -> setCallbacks(new ServerCallbacks()); // cria uma instancia do serviço de callback

  //======= Serviços do Periferico BLE ======= //
  BLEService *service = nullptr; //Ponteiro para variável tipo BLEService
  service = server -> createService(SERVICE_UUID); //crio um serviço com o UUID e guardo seu endereço no ponteiro
  pacote = service -> createCharacteristic(  //Configurar Características
    PACOTE_UUID,
    BLECharacteristic::PROPERTY_READ |    //Habilita a leitura do serviço
    BLECharacteristic::PROPERTY_NOTIFY   // Habilita a assinatura do serviço para receber alteraçoes de pacote
  );

  service -> start(); // inicia o serviço



  // ======= Criação do Advertising para poder ser descoberto ======= //
  BLEAdvertising *advertising = nullptr; // Ponteiro para váriavel tipo BLEAdvertising
  advertising = BLEDevice::getAdvertising(); // Crio um advertising e coloco seu endereço no ponteiro advertising
  advertising -> addServiceUUID(SERVICE_UUID);
  advertising -> setScanResponse(false); //Configurações de Advertising
  advertising -> setMinPreferred(0x06); //Configurações de Advertising
  BLEDevice::startAdvertising(); //inicia o Advertising

}

struct obc_frame{
  unsigned int rpm ;
  unsigned int digital1;
  unsigned int digital2;
  unsigned int digital3;
  unsigned int digital4;
}frame = {1200,1,0,0,0};


void sense()
{

  frame.rpm = frame.rpm + 4;


  if (isnan(frame.rpm))               // verifica se contém um número dentro de RPM 
  {                                  // se não retorna
    Serial.println("RPM reading Failed!");
    return;
  }

  Serial.printf("RPM = %d | Digital 1: %d | Digital 2: %d | Digital 3: %d  | Digital 4: %d", frame.rpm,frame.digital1,frame.digital2,frame.digital3,frame.digital4 );
  
  if (lastRPM != frame.rpm)
  {
      pacote -> setValue(frame.rpm);
      pacote -> notify(); //notifica que houve alterações no pacote
      lastRPM = frame.rpm;
  }



}

void loop() {

  if (readMillis == 0 || (millis() - readMillis) >= READ_INTERVAL)
  {
    sense();
    readMillis = millis();
  }


  if (!devicesConnected)
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