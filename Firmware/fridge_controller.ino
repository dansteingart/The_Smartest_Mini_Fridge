// This #include statement was automatically added by the Particle IDE.
#include <RelayShield.h>
#include <pid.h>
#include <math.h>

RelayShield myRelays;
char publishString[200]; //a place holer for the publish string
int state = 0;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,4,8,2, PID::DIRECT);
int WindowSize = 1000;
unsigned long windowStartTime;
unsigned long sendTime;
bool pidder = false;

unsigned long lchange = 0;
int diff = 1000; //don't switch faster than


void setup() {
    //.begin() sets up a couple of things and is necessary to use the rest of the functions
    myRelays.begin();
    Particle.function("set_state", set_state);
    Particle.function("set_temp", set_temp);

    //PID Stuff
    windowStartTime = millis();

    //initialize the variables we're linked to
    Setpoint = -42;

    //tell the PID to range between 0 and the full window size
    myPID.SetOutputLimits(0, WindowSize);

    //turn the PID on
    myPID.SetMode(PID::AUTOMATIC);

    //send Stuff
    sendTime = millis();

}

float therm(float an)
{
    float R2 = 10000.0;
    float V1 = 4095.0;

    //R1 = ((V1/a0)-1)*R2
    float R1 = ((V1/an)-1)*R2;

    float B  = 3950.0;
    float T0 = 296.0;
    float R0 = 10000.;

    float oneT = (1/T0)+(1/B)*log(R1/R0);
    float T = 1/oneT;
    T = T-273;
    return T;
}

void loop()
{

    float a0 = 0;
    float a1 = 0;
    float a2 = 0;
    float a3 = 0;
    float a4 = 0;

    int end = 50;
    for (int i = 1; i < end; i++)
    {
      a0 += analogRead(0);
      a1 += analogRead(1);
      a2 += analogRead(2);
      a3 += analogRead(3);
      a4 += analogRead(4);
    }

    a0 = a0/end;
    a1 = a1/end;
    a2 = a2/end;
    a3 = a3/end;
    a4 = a4/end;


    float T0 = therm(a0);
    Input = T0;
    myPID.Compute();

    if(millis() - windowStartTime>WindowSize)
    { //time to shift the Relay Window
      windowStartTime += WindowSize;
    }

    //if we're on autocontrol.....
    if (pidder & (millis() - lchange >diff))
    {
      // pid block
      // if(Output > millis() - windowStartTime) stater(1); //fixed it?
      // else stater(0);

      // hack central. hot or cold. no deadband. threshold on setpoint
      // if too cold.....
      if (T0 < Setpoint)
      {
          if (state == 2)
          {
            stater(0); //if currently cold, make neutral
            delay(2000); //better to just retake heat from sink then to reverse polarity too quickly. arbitrary delay.
          }
          else if (state == 0) stater(1); // if it's still to cold, reverse course
      }
      else
      {
        if (state == 1)    stater(0); // if currently hot, make neutral
        else if (state==0) stater(2); // if neutral, make cold
      }
      lchange = millis(); //waiting period redefined
    }

    if (millis() - sendTime > 5000)
    {
      sprintf(publishString,"{\"a0\": %f, \"a1\": %f, \"a2\": %f,\"a3\": %f, \"a4\": %f,\"setpoint\":%f,\"output\":%f,\"state\":%d,\"T0\":%f}",a0,a1,a2,a3,a4,Setpoint,Output,state,T0);
      Particle.publish("coke_fridge_test",publishString);
      sendTime = millis();
    }


}


void off()
{
    for (int i = 1; i < 5; i++) myRelays.off(i);
}

void hot()
{
    off();
    myRelays.on(1);
    myRelays.on(3);

}

void cold()
{
    off();
    myRelays.on(2);
    myRelays.on(4);

}


void stater(int port)
{
  state = port;
  if (port == 0) off();
  else if (port == 1) hot();
  else if (port == 2) cold();
  else off();
  delay(500);
}

int set_state(String potter)
{
  pidder = false;
  stater(potter.toInt());
  //break the input string down into two parts.
  return potter.toInt();
}

int set_temp(String potter)
{
  pidder = true;
  Setpoint = potter.toFloat();

  return Setpoint;
}
