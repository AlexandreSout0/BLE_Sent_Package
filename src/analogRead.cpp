#include <Arduino.h>
#include "analogRead.hpp"


analog_read::analog_read(const int pin1,const int pin2,const int pin3,const int pin4,const int rpm,const int pulse)
{
    pinMode(pin1,   INPUT_PULLDOWN);
    pinMode(pin2,   INPUT_PULLDOWN);
    pinMode(pin3,   INPUT_PULLDOWN);
    pinMode(pin4,   INPUT_PULLDOWN);
    pinMode(rpm,    INPUT_PULLDOWN);
    pinMode(pulse,  INPUT_PULLDOWN);

}
 
unsigned analog_read::analog_digital(int pinoDigital)
{
    int DigitalVal = 0;
    
    if ((DigitalVal =! digitalRead(pinoDigital)))
    {
        DigitalVal = 1;
    }
    

    return DigitalVal;
}

unsigned analog_read::analog_rpm(int rpm)
{ 
    float totalTime      =  0;
    int   highPulseTime  =  0;
    int   lowPulseTime   =  0;
    int   frequency      =  0;

    /*     
                          high
              |--|  |--|  |--|  |--|  |--|
            __|  |__|  |__|  |__|  |__|  |____  signal PWM pulse
                  low
      -----------------------------------------------> time
    */
   
    highPulseTime = pulseIn(rpm, HIGH); // Time the signal is high
    lowPulseTime = pulseIn(rpm, LOW); // Time the signal is low
    totalTime = highPulseTime + lowPulseTime; // Sum time high and low

    if (totalTime <= 0)
    {
      totalTime = 1000000;
    }

    frequency = 1000000 / totalTime;

    if (frequency == 1)
    {
      frequency = 0;
    }
    return frequency;
}

unsigned analog_read::analog_pulse(int pulse)
{
     /*     
                          high = pulse
              |--|  |--|  |--|  |--|  |--|
            __|  |__|  |__|  |__|  |__|  |____  signal pulses
                  low
      -----------------------------------------------> time
   */
      float totalTime     = 0;
      int   highPulseTime = 0;
      int   lowPulseTime  = 0;
      int   numPulse      = 0;

      highPulseTime = pulseIn(pulse, HIGH); // Time the signal is high
      lowPulseTime =  pulseIn(pulse, HIGH); // Time the signal is low

      totalTime = highPulseTime + lowPulseTime;

      if (totalTime <= 0)
      {
        totalTime = 1000000;
      }

      numPulse = 1000000/totalTime;    //obter frequência com o tempo está em microssegundos
      if (numPulse == 1)
      {
        numPulse = 0;
      }
      
      return numPulse;
         
}