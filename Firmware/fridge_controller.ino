// This #include statement was automatically added by the Particle IDE.
#include <RelayShield.h>
#include <pid.h>

RelayShield myRelays;
char publishString[200]; //a place holer for the publish string
int state = 0;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,1,4,1, PID::DIRECT);
int WindowSize = 1000;
unsigned long windowStartTime;
unsigned long sendTime;
bool pidder = false;

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

void loop()
{

    int a0 = analogRead(0);
    int a1 = analogRead(1);
    int a2 = analogRead(2);
    int a3 = analogRead(3);
    int a4 = analogRead(4);

    for (int i = 1; i < 50; i++)
    {
    a0 += analogRead(0);
    a1 += analogRead(1);
    a2 += analogRead(2);
    a3 += analogRead(3);
    a4 += analogRead(4);
    }

    a0 = a0/51;
    a1 = a1/51;
    a2 = a2/51;
    a3 = a3/51;
    a4 = a4/51;

    Input = a0;
    myPID.Compute();

    if(millis() - windowStartTime>WindowSize)
    { //time to shift the Relay Window
      windowStartTime += WindowSize;
    }

    if (pidder)
    {
      if(Output > millis() - windowStartTime) stater(1); //fixed it?
      else stater(0);
    }

    if (millis() - sendTime > 5000)
    {
      sprintf(publishString,"{\"a0\": %d, \"a1\": %d, \"a2\": %d,\"a3\": %d, \"a4\": %d,\"setpoint\":%f,\"output\":%f,\"state\":%d}",a0,a1,a2,a3,a4,Setpoint,Output,state);
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
}

int set_state(String potter)
{
  pidder = true;
  stater(potter.toInt());
  //break the input string down into two parts.
  return potter.toInt();
}

int set_temp(String potter)
{
  pidder = true;
  Setpoint = potter.toInt();

  return Setpoint;
}
