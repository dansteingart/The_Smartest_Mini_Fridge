// This #include statement was automatically added by the Particle IDE.
#include <RelayShield.h>

RelayShield myRelays;
char publishString[200]; //a place holer for the publish string
int state = 0;

void setup() {
    //.begin() sets up a couple of things and is necessary to use the rest of the functions
    myRelays.begin();
    Particle.function("set_state", set_state);
    
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



    sprintf(publishString,"{\"a0\": %d, \"a1\": %d, \"a2\": %d,\"a3\": %d, \"a4\": %d,\"state\":%d}",a0,a1,a2,a3,a4,state);
    Particle.publish("coke_fridge_test",publishString);
    delay(5000);

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


int set_state(String potter)
{

//break the input string down into two parts.
int port = potter.charAt(0)-48; //1 or 2
state = port;
if (port == 0) off();
else if (port == 1) hot();
else if (port == 2) cold();
else off();
return potter.toInt();
}