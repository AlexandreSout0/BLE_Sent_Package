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

#define READ_INTERVAL     2000
#define LEDPIN               2
#define TIMER_INTERVAL 1000000 // 1 SEGUNDO

#define SERVICE_UUID "0716bf69-27fa-44bd-b636-4ab49725c6b0"
#define PACOTE_UUID  "0716bf69-27fa-44bd-b636-4ab49725c6b1"
#define RX_UUID      "4ac8a682-9736-4e5d-932b-e9b31405049c"

#define PIN_DIGITAL_1 2
#define PIN_DIGITAL_2 3
#define PIN_DIGITAL_3 4
#define PIN_DIGITAL_4 5
#define PIN_PULSE_1  18
#define PIN_RPM      19




int lastRPM = -999;
int devicesConnected = 0; //Contador de usuários conectados

unsigned int blinkMillis = 0;
unsigned int readMillis = 0;

int flag_retorno = 0;

volatile int interruptCounter;
int totalInterruptCounter;


#include "analogRead.hpp"
analog_read meuspinos(18,19,15,4,5,3);



hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

BLEServer *server = nullptr; //Ponteiro para uma variável tipo BLEserver
BLECharacteristic *pacote = nullptr; //Ponteiro para caracteristicas do serviço do periferico
BLECharacteristic *pacote_rx = nullptr; //Ponteiro para caracteristicas do serviço do periferico
BLEService *service = nullptr; //Ponteiro para variável tipo BLEService

//callback  para envendos das características
class CharacteristicCallbacks: public BLECharacteristicCallbacks 
{
    String temp;

    void onWrite(BLECharacteristic *pacote_rx) 
    {
      //retorna ponteiro para o registrador contendo o valor atual da caracteristica
      std::string rxValue = pacote_rx->getValue().c_str(); 
      //verifica se existe dados (tamanho maior que zero)
      if (rxValue.length() > 0) 
      {
        //Serial.print(rxValue.c_str());
        temp = temp + rxValue.c_str();
      }
      Serial.println(temp);
      if(temp == "$POK")
      {
        flag_retorno = 1;
      }

  }

};

/*
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) 
        {
          Serial.print(rxValue[i]);
        }
*/


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
    service -> stop();
    delay(1000);
    service -> start();
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

struct obc_frame 
{
  unsigned int rpm ;
  unsigned int digital1;
  unsigned int digital2;
  unsigned int digital3;
  unsigned int digital4;
  unsigned int pulse1;
}frame = {1200,1,0,0,0,15};


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

void sense()
{
  String package;
  package = "$ALX,";
  frame.rpm = frame.rpm + 4;
   
   //$ALX,600,0,0,0,0,0,checksum\r\n

  if (isnan(frame.rpm))               // verifica se contém um número dentro de RPM 
  {                                  // se não retorna
    Serial.println("RPM reading Failed!");
    return;
  }
  //Serial.println("RPM = %d | Digital 1: %d | Digital 2: %d | Digital 3: %d  | Digital 4: %d", frame.rpm,frame.digital1,frame.digital2,frame.digital3,frame.digital4 );
  package = (package + frame.rpm + "," + frame.digital1 + "," + frame.digital2 + "," + frame.digital3 + "," + frame.digital4 + "," + frame.digital4 + "," + frame.pulse1 + ",");
  String temp_checksum = Gerador_de_Checksum(package);
  package = (package + temp_checksum + "\r\n");

  if (lastRPM != frame.rpm && flag_retorno == 1)
  {
      pacote -> setValue(package.c_str());
      pacote -> notify(); //notifica que houve alterações no pacote
      lastRPM = frame.rpm;
  }

}

void loop() 
{

  if (interruptCounter > 0) 
  {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;
 
    //Serial.print("An interrupt as occurred. Total number: ");
    //Serial.println(totalInterruptCounter);

     int teste = meuspinos.analog_rpm(18);
      Serial.println(teste);


  }

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