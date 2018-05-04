#include <NMEAGPS.h>
#include <GPSport.h>





//Surf Settings - deploy speeds
float minSpeed=7.0;
float maxSpeed=14.0;
  
// All entries for timing/tabs/buttons are arrays where array[0] = off  array[1] = surf left controls, array[2] = surf right controls.
// So below example, off doesnt mean anything for actuators, left surfing tab takes 3 seconds to deploy, right surfing tab takes 3 seconds
// When wiring, "surf left" means deploy right tab, so make sure you follow wiring diagram.  This is to avoid confusion in code.

// Actuator settings
int tabTime[ ] = {0,3000,3000}; // The amount of time it eachs each tab to deploy/retract.  Longer than it takes is fine - but dont go over by a ton.  Time when underway, not when no load is on it!
int coolDown = 500; // Wait 500ms before taking a second action on same tab

// Pin settings for your arduino
const int buttonPins[ ] = {6,10,11}; // Set to your button pins, array[0]= off, array[1]=surf left, array[2]= surf right button.  Using momentary push buttons with a long/short press in code below.
const int ledPins[ ] = {0,45,43}; // Set to your button pins, array[0]= off, array[1]=surf left, array[2]= surf right button.  Using momentary push buttons with a long/short press in code below.
const int relayGate[ ] = {0,2,4}; //0 off, 31= left gate pin, 35=right gate pin
const int relayGateRetract[ ] = {0,3,5}; // 0= off, 33= left retract gate pin, 36 = right retract gate pin


// No need to edit below here

//GPS data
long lastUpdate = 0;
float lastSpeed;
int satCount;
bool gpsOut;
bool speedlimit;
bool forceDeploy = false;
int interval;

int surf=0; //off=0, surf left=1, surfright=2
int deployed[ ]={0,0,0}; // tab deployed

// Tab counters
long deployTimers[ ] = {0,-1,-1}; // deploy timer - 0=off, 1= left, 2= right for all arrays
long retractTimers[ ] ={0,-1,-1}; // retracting of actuators;
long ledTimers[ ] ={0,-1,-1}; // retracting of actuators;
long ledState[ ] ={0,-1,-1}; // retracting of actuators;

long coolDownTimers[ ] = {0,-1,-1};
String tabMap[ ] = {"OFF","LEFT","RIGHT"};

long buttons[ ] = {-1,-1,-1}; //off, left, right timer for long press/short press.
long loopTimer; // a reusable loop timer.
int loopTime; // how long the last loop took
int i; // counter


NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values



void setup()
{
  DEBUG_PORT.begin(9600);
  while (!Serial)
    ;
  DEBUG_PORT.print( F("SurfGate Starting Up...\n") );
  for(i=0; i<3; i=i+1) {
      pinMode(buttonPins[i], INPUT_PULLUP);

  }
  
  for(i=1; i<3;i=i+1) {
    pinMode(relayGate[i], OUTPUT);
    pinMode(relayGateRetract[i], OUTPUT);
    pinMode(ledPins[i], OUTPUT);


  }

    gpsPort.begin(9600);

  delay(1500);                                 

  
}

//--------------------------


void GPSloop()
{
  while (gps.available( gpsPort )) {
    fix = gps.read();
    lastUpdate=millis();
    lastSpeed=fix.speed_mph();
    satCount = fix.satellites;
  }

} 

void gpsLagCheck() {
  if((millis() - lastUpdate) > 5000) {
    gpsOut=true;
  } else {
    gpsOut = false;
  }
}

void checkSpeed() {
  if(lastSpeed > minSpeed and lastSpeed < maxSpeed|| forceDeploy) {
    speedlimit=true;
  } else {
    speedlimit=false;
  }
}

void reportSerial() {
  
  if(millis() % 1000 == 0) {
    Serial.print(millis());
    Serial.print(" surf side: ");
    Serial.print(tabMap[surf]);
    Serial.print(" Speed: ");
    Serial.print(lastSpeed);
    Serial.print(" Sats: ");  
    Serial.print(satCount);
    Serial.print(" CycleTime: ");
    Serial.print(loopTime);
    Serial.print(" Speedlimit status:");
    Serial.print(speedlimit);
    Serial.print(" Deploy status:");
    if(deployed[1] ==1) {
      Serial.print(tabMap[1]);
    } else if (deployed[2] == 1) {
      Serial.print(tabMap[2]);
    } else {
      Serial.print("Off");
    }
    Serial.println();
  }
}

void deployTab(int tab) {
    Serial.print("Deploying: ");
    Serial.println(tabMap[tab]);
    for (i = 1; i < 3; i = i + 1) { // loop thru to see if we need to retract anything
      if(i != tab and (deployed[i] == 1 or deployTimers[i] > 0)) {
        retractTab(i);
      }
    }
    digitalWrite(relayGateRetract[tab], LOW);
    digitalWrite(relayGate[tab], HIGH);

    //Do deploy logic here
    deployTimers[tab] = millis();
}

void retractTab(int tab) {
    Serial.print("Retracting: ");
    Serial.println(tabMap[tab]);
    digitalWrite(relayGate[tab], LOW);
    digitalWrite(relayGateRetract[tab], HIGH);

    retractTimers[tab] = millis();
}

void goSurf() {
  
  if(surf > 0) { // Is surf enabled?  surf 1=left surf2=right
    
    if(speedlimit ==1) { // Are we in a deploy speed range
      if(deployTimers[surf] == -1 and retractTimers[surf] ==-1 and deployed[surf] != 1 and (millis()- coolDownTimers[surf]) > coolDown) { // are we not deploying or retracting and not deployed and not in a cooldown for this actuator?
        deployTab(surf);
      }
    }
    
    // Check if we have anything to retract
    if(speedlimit ==0) { // Are we outside of a deploy speed range
        for (i = 1; i < 3; i = i + 1) { // loop thru both to see if we need to retract
          if(retractTimers[i] == -1 and deployTimers[i] ==-1 and deployed[i] == 1 and (millis()- coolDownTimers[surf]) > coolDown) { // are we not deploying or retracting and deployed and not in a cooldown for this actuator?
            retractTab(i);
          }
        }
      }
      
  } else { //  auto-retract when surf is off
   for (i = 1; i < 3; i = i + 1) { // loop thru both to see if we need to retract
    if(retractTimers[i] == -1 and deployTimers[i] ==-1 and deployed[i] == 1 and (millis()- coolDownTimers[surf]) > coolDown) { // are we not deploying or retracting and deployed and not in a cooldown for this actuator?
        retractTab(i);
      }
    }
  }
}

void updateLEDs() {
  for (i = 1; i < 3; i = i + 1) { 
    if(deployTimers[i] > 0 or retractTimers[i] > 0) {
    if(ledTimers[i]== -1) {
      ledTimers[i] = millis();
    }
    if(deployTimers[i] > 0) {
      interval=250;
    } else {
      interval=100;
    }
    if(millis() - ledTimers[i] > interval) {
      if(ledState[i] == LOW) {
        ledState[i] = HIGH;
      } else {
        ledState[i] = LOW;
      }
      ledTimers[i] = millis();
      digitalWrite(ledPins[i], ledState[i]);
    }



    } else {
    if(deployed[i] == 1) {
      ledTimers[i] =-1;
      digitalWrite(ledPins[i], HIGH);
    } else {
      ledTimers[i] =-1;
      digitalWrite(ledPins[i], LOW);
    }
    }
  }
}

void tabTimers() {
  // Check timers for status
  for (i = 1; i < 3; i = i + 1) {
    if(deployTimers[i] > 0 and (millis()- deployTimers[i]) > tabTime[i] and retractTimers[i] == -1) {
      deployed[i] = 1;
      //Do your deploy complete logic here, IE: set pins
      Serial.print("Deploy complete: ");
      Serial.println(tabMap[i]);

      digitalWrite(relayGate[i], LOW); // Stop deploying gate
            digitalWrite(relayGateRetract[i], LOW); // Stop retracting gate

      deployTimers[i]=-1;
      coolDownTimers[i] = millis();
    }
  }
  for (i = 1; i < 3; i = i + 1) {
    if(retractTimers[i] > 0 and (millis()- retractTimers[i]) > tabTime[i] and deployTimers[i] ==-1) {
      deployed[i]=0;
      //Do your retract complete logic here, IE: set pins
      digitalWrite(relayGate[i], LOW); // Stop retracting gate
      digitalWrite(relayGateRetract[i], LOW); // Stop retracting gate

      Serial.print("Retract complete: ");
      Serial.println(tabMap[i]);
      retractTimers[i]=-1;
      coolDownTimers[i] = millis();

     }
  }
}

void checkButtons() {
  for (i = 0; i < 3; i = i + 1) {
   int buttonValue = digitalRead(buttonPins[i]);
   // Check for short press
   if(buttonValue == HIGH and surf == i and surf != 0) {
    Serial.print("No longer surfing on side");
    Serial.println(tabMap[i]);
    buttons[i] =-1;
    surf = 0;
   }
   if(buttonValue == LOW and surf != i) {
    Serial.print("Got trigger on switch");
    Serial.println(tabMap[i]);
    buttons[i] =-1;
    surf = i;
   }
  }
}
void loop()
{
  
  if(millis() - loopTimer> 100) {
    Serial.print("LONG LOOP - GPS ISSUE POSSIBLE");
    Serial.println(millis()-loopTimer);
    loopTime = millis()-loopTimer;
  } 
  loopTimer=millis();
  
  tabTimers();
  checkButtons();
  GPSloop(); // Get GPS data

  gpsLagCheck(); // Is the GPS data old?  If so, report it.
  checkSpeed();  // Check  if my speed is in the deploy range
  speedlimit=1;  // override cause i'm sick of driving around
  updateLEDs();

  goSurf();
  reportSerial(); // Report status on serial
  
}
