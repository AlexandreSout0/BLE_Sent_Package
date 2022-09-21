#include <Arduino.h>
#include "analogRead.hpp"


analog_read::analog_read(const int pin1,const int pin2,const int pin3,const int pin4,const int pulse1,const int rpm):
pins{pin1,pin2,pin3,pin4,pulse1,rpm}
{
  for(short i=0;i<6; i+=1)
  {
    pinMode(pins[i], INPUT_PULLUP);
    
  }

}

unsigned analog_read::analog_digital(int digital1,int digital2,int digital3,int digital4)
{
    int  flag1=0;
    int  flag2=0;
    int  flag3=0;
    int  flag4=0;

    flag1 = digitalRead(pins[1]);
    flag2 = digitalRead(pins[2]);
    flag3 = digitalRead(pins[3]);
    flag4 = digitalRead(pins[4]);
    flag1 = 1;
    flag2 = 1;
    flag3 = 1;
    flag4 = 1;

    return flag1;
}

unsigned analog_read::analog_rpm(int rpm)
{ 
    int  flag6=0;
    flag6 = digitalRead(pins[6]);
    flag6 = 1200;

    return flag6;
}

unsigned analog_read::analog_pulse(int pulse)
{
    int  flag5=0;
    flag5 = digitalRead(pins[5]);
    flag5 = 500;
    return flag5;
}