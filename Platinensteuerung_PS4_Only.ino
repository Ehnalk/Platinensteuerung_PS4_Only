#include <BuggyControl.h>
#include <LEDManager.h>
#include <Motor.h>
#include <SteeringServo.h>

#include <PS4Controller.h>
#include <ps4.h>
#include <ps4_int.h>

#include <dummy.h>

#include <ESP32Servo.h>
#include <Arduino.h>
#include <BLESecurity.h>
#include <Ticker.h>
#include <vector>
#include <algorithm>
#include <string>
#include <math.h>

#include <esp_task_wdt.h>

// *********************************************************
//        PS4 Controller mac Adresse hier einfungen
// *********************************************************
const char* CONTROLLER_MAC  = "60:5b:b4:b2:90:b6";
// *********************************************************
// 
// *********************************************************

Ticker LED_indicator_manager;

bool isLightAnimationRunning = false;
bool isBlinking = false;
bool indicatorTiming = false; // anstatt int da nur 2 werte benötigt werden

//Klassen für die Motorsteuerung global initialisieren
Motor motor(13, //PWM Front
            12, //PWM Back
            14, //Dauer_High Front
            27, //Dauer-High Back
            100,//Max Duty
            30, //Min Duty
            100, //Direction Change Delay
            30000//Freq
            );  // direction_change_delay = 1 (NICHT 500!)
SteeringServo steering(26,  // Pin des Servos
                       -3,  // Power pin nicht auf dem ESP
                       90,  // Start Winkel
                       90,  // Lenkwinkel ( also Start Winkel +- Lenkwinkel ist Maximale Lenkung)
                       6);  // Deadzone des Servos -> unterhalb diesem Wertes wird nicht gelenkt
LEDManager leftIndicator({16},    // Pin LED 
                          0,      // Standart Wert 0 = standartmäßig aus
                          100,    // Helligkeit (aka Duty des PWM)
                          1000);  // Freq
LEDManager rightIndicator({5}, 0, 100, 1000);
LEDManager frontLights({19}, 1, 100, 1000);
LEDManager rearLights({18}, 1, 100, 1000);
LEDManager brakeLights({4}, 1, 100, 1000);


std::vector<LEDManager*> allLeds;

// ------------------------------------------------------------------------
//        Setup Funktionen
// ------------------------------------------------------------------------


void config()   //Config-Klasse, hier können alle Werte angepasst werden.
{
  Serial.begin(115200);
  Serial.println("Start");

  setZero();
  pinMode(26, OUTPUT);  // Servo
  setZero();

  // Callback-Loop des PS4 Controllers Initialisieren
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisconnect);
  PS4.attach(onIncommingPS4Data);
  setZero();
  // Callback-Loop des PS4 Controllers starten
  beginPS4Connection(NULL);
  setZero();
}

// ------------------------------------------------------------------------
//        LED Animations Funktionen
// ------------------------------------------------------------------------

//  Licht Animationsfunktion, aka eine funktion um die LED an und aus zu machen für 'blink_amount' mal.
//  verwendung von Delays kann evtl blocking sein.
void lightAnimation()  
{ 
  
  if(isLightAnimationRunning){
    Serial.println("stop LED Animation");
     for(LEDManager* i : allLeds)
    {
      i->stopIndicating();
    }
  }
  Serial.println("starting LED Animation");
  for(LEDManager* i : allLeds)
  {
    i->stopIndicating();
  }
}


// ------------------------------------------------------------------------
//        BLE PS4 Controller Funktionen
// ------------------------------------------------------------------------


// Beginnt die Bluetooth verbindung mit dem PS4 Controller bzw die Bluetooth suche.
// Die Paramater '*pvParamaeters* ist für eine Spätere verwendung von Multithreading.
void beginPS4Connection(void *pvParameters) {

  PS4.begin(CONTROLLER_MAC);

  Serial.println("Waiting for Controller");
  Serial.println("PS4 Controller searching in Thread: ");
  Serial.println(xPortGetCoreID());
  motor.changeSpeedAbsolute(0);
}

// Logik für Die PS4 Knöpfe wie Rechts, Links und Dreieck. 
void parseButtonLogic() {

  //  Auf Dreieck soll man kann man alle Lichter an machen können.
  if(PS4.Triangle())  {
    Serial.print("Triangle, ");
    
    leftIndicator.startIndicating();
    rightIndicator.startIndicating();
    frontLights.startIndicating();
    brakeLights.startIndicating();
    rearLights.startIndicating();
  }
  // Mit dem rechts Pfeil kann man nach rechts Blinken
  if(PS4.Right()) {
    rightIndicator.startIndicating();
  }
  // Mit dem links Pfeil kann man nach links Blinken
  if(PS4.Left())  {
    leftIndicator.startIndicating();
  }
  // Mit dem unten Pfeil kann man alle LEDs aufhören lassen zu blinken
  if(PS4.Down())  {
    Serial.println("Down");
    leftIndicator.stopIndicating();
    rightIndicator.stopIndicating();
    frontLights.stopIndicating();
    brakeLights.stopIndicating();
    rearLights.stopIndicating();

    frontLights.turnOn(100);
    brakeLights.turnOn(100);
  }

}

// Variable um nur jeden dritten R2/L2 Wert zu verwerten, optimierungsversuch
uint8_t SkipDataCounter = 0;

// Funktion die Bei eingehenden PS4 Daten aufgerufen wird, diese funktion ist auch für die verarbeitung 
// /verwertung der Daten zuständig.
void onIncommingPS4Data() { 

  if(motor.getCurrentDuty() < 0)  {
    rearLights.turnOn(100);
    
  }
  if(motor.getCurrentDuty() > 0)  {
    rearLights.turnOff();
  }

  parseButtonLogic();

  if(SkipDataCounter == 3){
    SkipDataCounter = 0;
    return;
  }
  if(SkipDataCounter != 0){
    SkipDataCounter++;
    return;
  }
  SkipDataCounter++;

  int R2L2_in_percentage = (int)round(float((float(PS4.R2Value() - PS4.L2Value()) / 255)) * 100);
  int LX_percentage = (int)round(float((float(PS4.LStickX()) / 127 ) * 100));

  // Der Input des Rechten Joystick ist -127 zu 127.
  if(LX_percentage != steering.getCurrentSteeringPercent()) {

    if(abs(PS4.LStickX()) > 10 ){
      Serial.println(PS4.LStickX());
      // Der Rechte Joystick gibt einen Wert auf der X-Achse von 0 bis 255 aus
      steering.steerAbsolute(LX_percentage);
    }else {
      steering.steerAbsolute(0);
      Serial.println(0);
    }
  }

  // Wenn der eingehende R2/L2 reduntant ist, wird das programm hier beendet.
  if(R2L2_in_percentage == motor.getCurrentDuty()){
    return;
  }
  
  Serial.print(" R2: ");
  Serial.print(PS4.R2Value());
  Serial.print(" L2: ");
  Serial.println(PS4.L2Value());
  motor.changeSpeedAbsolute(R2L2_in_percentage);

}


void onConnect()  {
  Serial.println("Controller Connected!");
  blockingLEDAnimation();
}


void onDisconnect() {
  Serial.println("Controller Disconnected!");
  blockingLEDAnimation();
} 

void blockingLEDAnimation() {
    leftIndicator.startIndicating();
    rightIndicator.startIndicating();
    frontLights.startIndicating();
    brakeLights.startIndicating();
    rearLights.startIndicating();
    delay(1000)
    leftIndicator.stopIndicating();
    rightIndicator.stopIndicating();
    frontLights.stopIndicating();
    brakeLights.stopIndicating();
    rearLights.stopIndicating();
}

void setup() {
  setZero();
  config();
  // KRITISCH: steering.begin() MUSS in setup() aufgerufen werden!
  // Nicht in einer Unterfunktion!
  steering.begin();
  
  
  // setZero() wird bereits in config() aufgerufen
  esp_task_wdt_deinit();
  setZero();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}

void setZero()
{

  Serial.println(("Null"));
  ledcWrite(12, 0);
  ledcWrite(13, 0);

  digitalWrite(14, LOW);
  digitalWrite(27, LOW);
  
}