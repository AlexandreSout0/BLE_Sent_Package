#include <Arduino.h>
#include "analogRead.hpp"


analog_read::analog_read(const int pin1,const int pin2,const int pin3,const int pin4,const int rpm,const int pulse)
{
    pinMode(pin1,   INPUT_PULLDOWN);
    pinMode(pin2,   INPUT_PULLDOWN);
    pinMode(pin3,   INPUT_PULLDOWN);
    pinMode(pin4,   INPUT_PULLDOWN);
    pinMode(rpm,    INPUT);
    pinMode(pulse,  INPUT);

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
      /*     
                          high
              |--|  |--|  |--|  |--|  |--|
            __|  |__|  |__|  |__|  |__|  |____  signal PWM of integrad alternador
                  low
      -----------------------------------------------> time
      */
      float totalTime = 0;
      int highPulseTime = 0;
      int lowPulseTime = 0;
      int frequency = 0;

      highPulseTime = pulseIn(rpm, HIGH); // Time the signal is high
      lowPulseTime = pulseIn(rpm, LOW); // Time the signal is low
    
      totalTime = highPulseTime + lowPulseTime;

      frequency = 1000000/totalTime;    //obter frequência com o tempo está em microssegundos

      return frequency;
}

unsigned analog_read::analog_pulse(int pulse)
{
     /*     
                          high
              |--|  |--|  |--|  |--|  |--|
            __|  |__|  |__|  |__|  |__|  |____  signal PWM pulse
                  low
      -----------------------------------------------> time
   */
      float totalTime = 0;
      int highPulseTime = 0;
      int numPulse = 0;

      highPulseTime = pulseIn(pulse, LOW); // Time the signal is high
    
      totalTime = highPulseTime;

      numPulse = totalTime/1000000;    //obter frequência com o tempo está em microssegundos

      return numPulse;
         
}