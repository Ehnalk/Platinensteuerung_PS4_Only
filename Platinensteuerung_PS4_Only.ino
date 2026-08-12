#include <BuggyControl.h>
#include <LEDManager.h>
#include <Motor.h>
#include <SteeringServo.h>

#include <PS4Controller.h>
#include <ps4.h>
#include <ps4_int.h>

#include <vector>

#include <esp_task_wdt.h>

// *********************************************************
//        Enter the PS4 controller's MAC address here
// *********************************************************
const char* CONTROLLER_MAC  = "60:5b:b4:b2:90:b6";
// *********************************************************
//
// *********************************************************

bool isLightAnimationRunning = false;

//Initialize classes for motor control globally
Motor motor(13, //PWM Front
            12, //PWM Back
            14, //Always High Front
            27, //Always High Back
            100,//Max Duty
            30, //Min Duty
            100, //Direction Change Delay
            30000//Freq
            );
SteeringServo steering(26,  // Servo pin
                       -3,  // Power pin, not on the ESP
                       90,  // Start angle
                       90,  // Steering angle (i.e. start angle +- steering angle is maximum steering)
                       6);  // Servo deadzone -> below this value there is no steering
LEDManager leftIndicator({16},    // LED pin
                          0,      // Default value 0 = off by default
                          100,    // Brightness (aka PWM duty)
                          1000);  // Freq
LEDManager rightIndicator({5}, 0, 100, 1000);
LEDManager frontLights({19}, 1, 100, 1000);
LEDManager rearLights({18}, 1, 100, 1000);
LEDManager brakeLights({4}, 1, 100, 1000);


std::vector<LEDManager*> allLeds;

// ------------------------------------------------------------------------
//        Setup Functions
// ------------------------------------------------------------------------


void setup() {
  setZero();
  Serial.begin(115200);
  Serial.println("Start");

  setZero();
  pinMode(26, OUTPUT);  // Servo
  setZero();

  // Initialize the PS4 controller's callback loop
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisconnect);
  PS4.attach(onIncommingPS4Data);
  setZero();
  // Start the PS4 controller's callback loop
  beginPS4Connection(NULL);
  setZero();
  // CRITICAL: steering.begin() MUST be called in setup()!
  // Not in a subfunction!
  steering.begin();


  // setZero() is already called in config()
  esp_task_wdt_deinit();
  setZero();
  delay(100);
}


// ------------------------------------------------------------------------
//        Bluetooth PS4 Controller Functions
// ------------------------------------------------------------------------


// Starts the Bluetooth connection with the PS4 controller, i.e. the Bluetooth search.
// The parameter 'pvParameters' is for later use with multithreading.
void beginPS4Connection(void *pvParameters) {

  PS4.begin(CONTROLLER_MAC);

  Serial.printf("Waiting for Controller. PS4 Controller searching in Thread: %d\n", xPortGetCoreID());
  motor.changeSpeedAbsolute(0);
}

// Logic for the PS4 buttons like right, left, and triangle.
void parseButtonLogic() {

  // Pressing triangle turns on all lights.
  if(PS4.Triangle())  {
    Serial.print("Triangle, ");

    animateAllLEDs();
  }
  // The right arrow makes the right indicator blink
  if(PS4.Right()) {
    rightIndicator.startIndicating();
  }
  // The left arrow makes the left indicator blink
  if(PS4.Left())  {
    leftIndicator.startIndicating();
  }
  // The down arrow stops all LEDs from blinking
  if(PS4.Down())  {
    Serial.println("Down");
    stopAllLEDs();

    frontLights.turnOn(100);
    brakeLights.turnOn(100);
  }

}

// Variable to only process every third R2/L2 value, an optimization attempt
uint8_t SkipDataCounter = 0;

// Function called when incoming PS4 data arrives; this function is also
// responsible for processing/using the data.
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

  // The left joystick's input ranges from -127 to 127.
  if(LX_percentage != steering.getCurrentSteeringPercent()) {

    if(abs(PS4.LStickX()) > 10 ){
      Serial.printf("LStickX: %d\n", PS4.LStickX());
      // The left joystick outputs a value on the X-axis from 0 to 255
      steering.steerAbsolute(LX_percentage);
    }else {
      steering.steerAbsolute(0);
      Serial.printf("LStickX: 0\n");
    }
  }

  // If the incoming R2/L2 value is redundant, the function returns here.
  if(R2L2_in_percentage == motor.getCurrentDuty()){
    return;
  }
  
  Serial.printf(" R2: %d L2: %d\n", PS4.R2Value(), PS4.L2Value());
  motor.changeSpeedAbsolute(R2L2_in_percentage);

}


void animateAllLEDs() {
  leftIndicator.startIndicating();
  rightIndicator.startIndicating();
  frontLights.startIndicating();
  brakeLights.startIndicating();
  rearLights.startIndicating();
}

void stopAllLEDs() {
  leftIndicator.stopIndicating();
  rightIndicator.stopIndicating();
  frontLights.stopIndicating();
  brakeLights.stopIndicating();
  rearLights.stopIndicating();
}

void blockingLEDAnimation() {
  animateAllLEDs();
  delay(1000);
  stopAllLEDs();
}

void onConnect()  {
  Serial.println("Controller Connected!");
  blockingLEDAnimation();
}


void onDisconnect() {
  Serial.println("Controller Disconnected!");
  blockingLEDAnimation();
}



void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}

void setZero()
{

  Serial.println("Null");
  ledcWrite(12, 0);
  ledcWrite(13, 0);

  digitalWrite(14, LOW);
  digitalWrite(27, LOW);
  
}