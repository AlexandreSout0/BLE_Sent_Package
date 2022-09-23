#include <Arduino.h>

#ifndef ANALOG_READ
#define ANALOG_READ

class analog_read
{
    public:
        analog_read(const int pin1,const int pin2,const int pin3,const int pin4,const int pulse1,const int rpm);
        unsigned analog_digital(const int digital1);
        unsigned analog_pulse(const int pulse1);
        unsigned analog_rpm(const int rpm);


    private:
        int pins[6];
};


#endif