SYSTEM_THREAD(ENABLED); //enables system functions to happen in a separate thread from the application setup and loop
//this includes connecting to the network and the cloud

char *sourceCode = "https://github.com/Toby-Mills/GarageController";

//Pin mappings
int intPIR = A1;
int intLIGHT = A0;
int intRelay = D7;
int intLedRelayIndicator = D4; //GREEN
int intLedLightIndicator = D2; //YELLOW
int intLedMotionIndicator = D0; //RED

//Constants
int intLightTimeout = 60000; //60000ms = 1min
int intPirThreshold = 1000;
int intLightThreshold = 1500;
int mode_off = 0;
int mode_on =1;
int mode_motion = 2;
int debugFrequency = 500;

//Variables
int intPIRReading;
int intLightReading;
int intTimerStarted = millis();
int intMode = mode_motion;//current mode, defaulted to "motion"
bool connectedOnce = false; //connected to cloud
int intDebugCount = 0;
int intLastDebug = 0;

void setup() {
        pinMode(intPIR, INPUT);
        pinMode(intLIGHT, INPUT);
        pinMode(intRelay, OUTPUT);
        pinMode(intLedRelayIndicator, OUTPUT);
        pinMode(intLedLightIndicator, OUTPUT);
        pinMode(intLedMotionIndicator, OUTPUT);

        digitalWrite(intLedRelayIndicator,HIGH);
        digitalWrite(intRelay, HIGH);
}

void debug(String eventname, String message, int value) {
  if (intDebugCount > 0){
    char msg [50];
    sprintf(msg, message.c_str(), value);
    Particle.publish(eventname, msg, 600, PRIVATE);
  }
}

void loop() {

   //code to register cloud functions once the particle is connected
   if (connectedOnce == false) {
     if (Particle.connected()) {
       //Register variables and methods to allow control via Particle Cloud
       Particle.variable("sourceCode", sourceCode, STRING);
       Particle.function("setMode", setMode_cloud);
       Particle.function("setDebugSeconds", setDebug_cloud);
       connectedOnce = true;
     }
   }

   intPIRReading = analogRead(intPIR);

   if (intPIRReading > intPirThreshold){
       digitalWrite(intLedMotionIndicator,HIGH);
   }else{
       digitalWrite(intLedMotionIndicator,LOW);
   }

   intLightReading = analogRead(intLIGHT);
    if (intLightReading > intLightThreshold){
       digitalWrite(intLedLightIndicator,HIGH);
   }else{
       digitalWrite(intLedLightIndicator,LOW);
   }

   if (intTimerStarted > 0){
       if (intPIRReading > intPirThreshold){
           intTimerStarted = millis();
       }
   } else if (intLightReading < intLightThreshold && intPIRReading > intPirThreshold) {
     intTimerStarted = millis();
   }

   if (millis()-intTimerStarted > intLightTimeout){
     intTimerStarted = 0;
   }

   if ((intMode == mode_motion && intTimerStarted > 0) || intMode == mode_on) {
       digitalWrite(intLedRelayIndicator,HIGH);
       digitalWrite(intRelay, HIGH);
   } else {
         digitalWrite(intLedRelayIndicator,LOW);
         digitalWrite(intRelay, LOW);
   }

    if(intDebugCount > 0){
      if(millis() - intLastDebug > debugFrequency){
        intDebugCount -=1;
        intLastDebug = millis();
        debug("PIRReading","%d",intPIRReading);
        debug("LightReading","%d",intLightReading);
      }
    }
}


//---------------------------------------------------------------
// Functions
//---------------------------------------------------------------

//Set the Lights Override
void setMode(int mode){
  intMode = mode;
}

//Set the Lights Override
int setMode_cloud(String mode){
  if (mode == "off"){
    setMode(mode_off);
    return mode_off;
  }else if(mode == "on"){
    setMode(mode_on);
    return mode_on;
  }else if(mode == "motion"){
    setMode(mode_motion);
    return mode_motion;
  }
}

void setDebug(int seconds){
  intDebugCount = seconds * (1000 / debugFrequency);
}

int setDebug_cloud(String seconds){
  setDebug(seconds.toFloat());
}
