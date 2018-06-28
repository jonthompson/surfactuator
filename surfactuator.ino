
#include <NMEAGPS.h>
#include <GPSport.h>
#include <EEPROM.h>



//Surf Settings - deploy speeds
float minSpeed=8.0;
float maxSpeed=14.0;
  
// All entries for timing/tabs/buttons are arrays where   array[0] = surf left controls, array[1] = surf right controls.
// When wiring, "surf left" means deploy right tab, so make sure you follow wiring diagram.  This is to avoid confusion in code.

// Actuator settings
// Default -- is overridden by the rotary encoder -- to reset to this default hold down rot button for 15s
int tabTime[ ] = {3500,3500}; // The amount of time it eachs each tab to deploy -- left=0, right=1

// Static values
int retractTime[ ] = {8000,8000}; // full time in ms to retract tabs fully from fully extended.  Assuming 8s to be safe
int maxTime[ ] = {7000,7000}; // the max amount of time an actuator should run, this should be smaller than above.
int coolDown = 500; // Wait 500ms before taking a second action on same tab to prevent any issues

// Pin Settings
// Switch pins -- wire with a ground and pin 10/11 as your surf left/right buttons on your switch.
const int buttonPins[ ] = {10,11}; // Set to your button pins, array[0]=surf left, array[1]= surf right button.  Using 3 way switch.

// Rotary pins
// Should be labeled - connect power and gnd as well
int CLK = 4;  // Pin 9 to clk on encoder
int DT = 5;  // Pin 8 to DT on encode
int ROTBUTTON = 2;
int rotInterval=100; // ms each "click" on rotary encoder adds/subtracts -- your rot may double count with the below code - so you may end up with 2x this value.  Tweak as you like, the moves are logged.


// Relay control pins
const int relayGate[ ] = {A2,A4}; // A2= left gate pin, A4=right gate pin
const int relayGateRetract[ ] = {A3,A5}; // A3= left retract gate pin, A5 = right retract gate pin

const int LEDPIN = 3;

// LED Intervals
int gpsFlashInterval=100;
int movingFlashInterval=500;

// No need to edit below here

//GPS data
long lastUpdate = 0; //when the last update occurred
float lastSpeed;
int satCount;
bool gpsOut;


// Timers
long deployTimers[ ] = {-1,-1}; // deploy timer -  0= left, 1= right for all arrays
long retractTimers[ ] ={-1,-1}; // retracting of actuators;
long coolDownTimers[ ] = {-1,-1};
int buttonStart[ ]={-1,-1}; // start deploy time for buttons
long LEDTimer=0;
long loopTimer; // a reusable loop timer.
int loopTime; // how long the full last loop took

// Misc
int i; // counter
bool speedlimit;
bool forceDeploy = false;
int start = 0;
int surf=-1; //off=-1, surf left=0, surf right=1
String tabMap[ ] = {"LEFT","RIGHT"}; // just for printing out in serial
int deployed[ ]={0,0}; // how far tab is deployed
int batchMoveTime = 1000; // How many ms to wait after a rotary move to take actoin -- this prevents repeated short bursts of time, with more accurate longer ones.

// Rotary variables
int RotPosition = 0; 
int rotation; 
int curval;
boolean LeftRight;
long lastRotUpdate;
long rotButtonTimer=-1;
boolean pendingMove = false;


NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values





void setup()
{

  DEBUG_PORT.begin(9600);
  while (!Serial)
    ;
  DEBUG_PORT.print( F("SurfGate Starting Up...\n") );

  // Set the button pins to INPUT_PULLUP
  for(i=0; i<2; i=i+1) {
      pinMode(buttonPins[i], INPUT_PULLUP);
  }
  
  // ENcoder pins
   pinMode (CLK,INPUT);
   pinMode (DT,INPUT);
   pinMode (2,INPUT);

   // LED pin
   pinMode(LEDPIN, OUTPUT);
   
  // Set the gates control pins to OUTPUT
  for(i=0; i<2;i=i+1) {
    pinMode(relayGate[i], OUTPUT);
    pinMode(relayGateRetract[i], OUTPUT);
  }
  
  // If we have any saved settings, restore them from EEPROM
  restoreSettings();
  
  // Enable GPS
  gpsPort.begin(9600);

}

//--------------------------
void(* resetFunc) (void) = 0; //declare reset function @ address 0


void GPSloop()
{
  while (gps.available( gpsPort )) {
    fix = gps.read();
    lastUpdate=millis();
    lastSpeed=fix.speed_mph();
    satCount = fix.satellites;
  }

} 

void saveSettings() {
    for (i = 0; i < 2; i = i + 1) { // loop thru to save
    Serial.print("Saving Settings for ");
    Serial.println(tabMap[i]);
    EEPROM.write(i, tabTime[i]/rotInterval);
    Serial.print("EE: ");
    Serial.println(EEPROM.read(i));
    
  }
}

void restoreSettings() {
  Serial.print("Restoring Settings for tabs:");
  for (i = 0; i < 2; i = i + 1) { // loop thru to save
    int ee = EEPROM.read(i);
    if(ee >1 and ee*rotInterval < maxTime[i]) {
      tabTime[0] = ee*rotInterval;
      } else {
        Serial.println("Default");
      }
  }
  
  Serial.print(tabMap[0]);
  Serial.println(tabTime[0]);
  Serial.print(tabMap[1]);
  Serial.println(tabTime[1]);
 
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
    if( surf > -1) {
      Serial.print(tabMap[surf]);
    } else {
      Serial.print("OFF");
    }
    Serial.print(" Speed: ");
    Serial.print(lastSpeed);
    Serial.print(" Sats: ");  
    Serial.print(satCount);
    Serial.print(" CycleTime: ");
    Serial.print(loopTime);
    Serial.print(" Speedlimit:");
    Serial.print(speedlimit);
    Serial.print(" Deploy:");
    if(deployed[0] >1) {
      Serial.print(tabMap[0]);
    } else if (deployed[1]  > 1) {
      Serial.print(tabMap[1]);
    } else {
      Serial.print("OFF");
    }

    Serial.print(" Deploy Settings: C");
    Serial.print(deployed[0]);
    Serial.print("/T");
    Serial.print(tabTime[0]);
    Serial.print(" - C");
    Serial.print(deployed[1]);
    Serial.print("/T");
    Serial.print(tabTime[1]);  
      
    Serial.print(" Timers: D");
    Serial.print(deployTimers[0]);
    Serial.print("/R");
    Serial.print(retractTimers[0]);
    Serial.print(" - D");
    Serial.print(deployTimers[1]);
    Serial.print("/R");
    Serial.print(retractTimers[1]);  
    Serial.print("EE: ");
    Serial.print(EEPROM.read(0));
    Serial.print(" ");
    Serial.print(EEPROM.read(1));
    Serial.println();
    
  }
}

void deployTab(int tab, int start) {
    Serial.print("Deploying: ");
    Serial.println(tabMap[tab]);

    for (i = 0; i < 2; i = i + 1) { // loop thru to see if we need to retract anything
      if(i != tab and (deployed[i] > 0 or deployTimers[i] > 0)) {
        retractTab(i,0);
      }
    }
    retractTimers[tab] = -1;
    digitalWrite(relayGateRetract[tab], LOW);
    digitalWrite(relayGate[tab], HIGH);
    deployTimers[tab] = millis()-start;
}

void retractTab(int tab, int start) {
    Serial.print("Retracting: ");
    Serial.println(tabMap[tab]);
    deployTimers[tab] = -1;
    digitalWrite(relayGate[tab], LOW);
    digitalWrite(relayGateRetract[tab], HIGH);
    if(start <0) {
      retractTimers[tab] = millis()-(tabTime[surf] + start);
    } else {
      retractTimers[tab] = millis();
    }
}

void resetTabs() {
  Serial.println("RESETTING TABS TO DEFAULT");
  for (i = 0; i < 2; i = i + 1) { // loop thru both 
    deployTimers[i] = -1;
    deployed[i] = 0;
    coolDownTimers[i]=-1;
    retractTab(i,0);
  }

}
void goSurf() {
  
  if(surf > -1) { // Is surf enabled?  surf 1=left surf2=right
    
    if(speedlimit ==1) { // Are we in a deploy speed range
      if(deployTimers[surf] == -1 and retractTimers[surf] ==-1 and deployed[surf] < 1 and (millis()- coolDownTimers[surf]) > coolDown) { // are we not deploying or retracting and not deployed and not in a cooldown for this actuator?
        deployTab(surf,0);
      }
    }
    
    // Check if we have anything to retract
    if(speedlimit ==0) { // Are we outside of a deploy speed range
        for (i = 0; i < 2; i = i + 1) { // loop thru both to see if we need to retract
          if(retractTimers[i] == -1 and deployTimers[i] ==-1 and deployed[i] > 1 and (millis()- coolDownTimers[surf]) > coolDown) { // are we not deploying or retracting and deployed and not in a cooldown for this actuator?
            retractTab(i,0);
          }
        }
      }
      
  } else { //  auto-retract when surf is off
   for (i = 0; i < 2; i = i + 1) { // loop thru both to see if we need to retract
    if(retractTimers[i] == -1 and deployTimers[i] ==-1 and deployed[i] > 1 and (millis()- coolDownTimers[surf]) > coolDown) { // are we not deploying or retracting and deployed and not in a cooldown for this actuator?
        retractTab(i,0);
      }
    }
  }
}

void updateLEDs() {
 // LED logic here if you want status lights.  DIY.

    // If GPS out flash quickly
   if(gpsOut) {
    if(LEDTimer ==0) {
          LEDTimer=millis();
    }
    if(millis()-LEDTimer > gpsFlashInterval) {
      if(digitalRead(LEDPIN) == HIGH) {
        digitalWrite(LEDPIN, LOW);
      } else {
        digitalWrite(LEDPIN, HIGH);
    }
    LEDTimer=millis();

    }
  
    return;
  }

  // If moving flash
  if(deployTimers[0] > 0 or deployTimers[1] > 0 or retractTimers[0] > 0 or retractTimers[1] > 0) {
    if(LEDTimer ==0) {
          LEDTimer=millis();
    }
    if(millis()-LEDTimer > movingFlashInterval) {
      if(digitalRead(LEDPIN) == HIGH) {
        digitalWrite(LEDPIN, LOW);
      } else {
        digitalWrite(LEDPIN, HIGH);
    }
    LEDTimer=millis();

    }
    return;
  }
  
 // If surfing solid
 if(deployed[0] >0 || deployed[1] > 0) {
  digitalWrite(LEDPIN,HIGH);
 } else {
  digitalWrite(LEDPIN, LOW);
  return;
 }
}

void tabTimers() {
  // Check timers for status
  for (i = 0; i < 2; i = i + 1) {
    if(deployTimers[i] > 0 and (millis()- deployTimers[i]) > tabTime[i] and retractTimers[i] == -1) {
      deployed[i] = tabTime[i];
      //Do your deploy complete logic here, IE: set pins
      Serial.print("Deploy complete: ");
      Serial.println(tabMap[i]);

      digitalWrite(relayGate[i], LOW); // Stop deploying gate
      digitalWrite(relayGateRetract[i], LOW); // Stop retracting gate

      deployTimers[i]=-1;
      coolDownTimers[i] = millis();
    }
  }
  for (i = 0; i < 2; i = i + 1) {
    if(i != surf) {
    if(retractTimers[i] > 0 and (millis()- retractTimers[i]) > retractTime[i] and deployTimers[i] ==-1) {

      deployed[i]=0;
      
      //Do your retract complete logic here, IE: set pins
      digitalWrite(relayGate[i], LOW); // Stop retracting gate
      digitalWrite(relayGateRetract[i], LOW); // Stop retracting gate

      Serial.print("Retract complete: ");
      Serial.println(tabMap[i]);
      retractTimers[i]=-1;
      coolDownTimers[i] = millis();
     }
    } else {
      // Do adjust here instead of full retract
    
    if(retractTimers[i] > 0 and millis() > retractTimers[i] and (millis()-retractTimers[i]) > tabTime[i] and deployTimers[i] ==-1) {

      deployed[i]=tabTime[i];
      
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
}

void checkButtons() {
     curval = digitalRead(CLK);
     if (curval != rotation){ // we use the DT pin to find out which way we turning.
     if (digitalRead(DT) != curval) {  // Clockwise
       RotPosition ++;
       LeftRight = true;
     } else { //Counterclockwise
       LeftRight = false;
       RotPosition--;
     }
      
    if(surf> -1) {
     if (LeftRight){ 
       if(tabTime[surf] + RotPosition*rotInterval < maxTime[surf]  ) {
        tabTime[surf] = tabTime[surf] + RotPosition*rotInterval;
       } else {
          Serial.println("MAX");
       }
     }else{        // decrease deploy
     if(tabTime[surf] + RotPosition*rotInterval >0 ) {
        tabTime[surf] = tabTime[surf] + RotPosition*rotInterval;
        } else {
           Serial.println("MIN");
        }
     }
     lastRotUpdate=millis();
     pendingMove=true;
    }
      rotation = curval;
      RotPosition=0;
    } 
    
   // Moving after detecting a rot change - wait 1 second to batch turns into 1 action instead of pulsing actuator 200ms.
  if(millis() >lastRotUpdate+batchMoveTime and tabTime[surf] != deployed[surf] and deployTimers[surf] == -1 and retractTimers[surf] == -1 and pendingMove) {
    Serial.print("Batched move occuring:");
    Serial.println(tabTime[surf] - deployed[surf]);
    if(tabTime[surf] > deployed[surf]) {
      deployTab(surf, deployed[surf]);
    } else {
      retractTab(surf, tabTime[surf] - deployed[surf]);
    }
    pendingMove=false;
  }

  int rotVal = digitalRead(ROTBUTTON);
  if(rotVal==LOW) {
    if(rotButtonTimer == -1) {
      rotButtonTimer = millis();
    }

   } else {
    if(rotButtonTimer > 0) {
      if(millis() - rotButtonTimer > 15000) {
       Serial.println("Rebooting");
       resetFunc();
       rotButtonTimer=-1;
       return;
    }
      
    if(millis() - rotButtonTimer > 5000) {
      resetTabs();
      rotButtonTimer=-1;
      return;
    }
    if(millis() - rotButtonTimer > 100 ) {
      Serial.println("Saving");
      saveSettings();
      rotButtonTimer=-1;
      return;
   }
   }
   rotButtonTimer=-1;

  }
  for (i = 0; i < 2; i = i + 1) {
   int buttonValue = digitalRead(buttonPins[i]);
   if(buttonValue == HIGH and surf == i and surf != -1) {
    buttonStart[i] = -1;
    surf = -1;
   }
   
   if(buttonValue == LOW and surf != i) {
    if(buttonStart[i] == -1) {
      buttonStart[i] = millis();
    }
    if(millis() - buttonStart[i] > 500) {
      surf = i;
      buttonStart[i] = -1;
    }
   }
  }
}

void loop()
{



  if(millis() - loopTimer> 100) {
    Serial.print("LONG LOOP - GPS ISSUE POSSIBLE");
    Serial.println(millis()-loopTimer);
  } 
  loopTime = millis()-loopTimer;

  loopTimer=millis();

  tabTimers();
  checkButtons();
  GPSloop(); // Get GPS data

  gpsLagCheck(); // Is the GPS data old?  If so, report it.
  checkSpeed();  // Check  if my speed is in the deploy range
  //speedlimit=1;  // override cause i'm sick of driving around
  updateLEDs();  // If you use LEDs for any status updates, set here.

  goSurf();
  reportSerial(); // Report status on serial
  
}
