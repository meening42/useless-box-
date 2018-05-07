/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>  // used if 
//#include <VarSpeedServo.h>

// --------CONSTANTS (won't change)---------------
// PINS
int POT1PIN = A11;
int POT2PIN = A12;
int BTN_ON_OFF = 47;
int BTN0 = 41;
int BTN1 = 37;
int BTN2 = 33;
int BTN3 = 28;
int BTN4 = 24;
int LED_PIN=3; 

int SERVO1_PIN = 9;
int SERVO2_PIN = 8;
int SERVODOOR_PIN = 2;

//------------ VARIABLES (will change)---------------------
Servo servo1;  // create servo object to control a servo
Servo servo2;
Servo servoDoor;

int pot1, pot2; 
int pos1,pos2;    // variable to store the servo position

int speedServo1, speedServo2;


const long intervalLed = 1000;
const long intervalPrintStatus= 1000;

unsigned long previousMillis = 0; 
unsigned long currentMillis;
unsigned long dellayDoorMills;
unsigned long doorDellayInterval = 300;
unsigned long dellayWaitPosMills;
unsigned long waitPosDellayInterval = 250;
unsigned long lastRobotPressedBtnTime = 0;
unsigned long dellayBackDriving = 100;


enum eBtnState{
  BTN_ON =0,
  BTN_OFF =1
};

int ledState = LOW;  
bool btn_state[5];
int nextBtnToPress=0;
int prevBtnToPress=0;
bool waitPointReached;

int servo1_pos[5] = {17,41,71,99,135};
int servo2_pos[5] = {80,80,80,85,80};
int servo2_wait_pos = 95;
int servo2_down_pos = 155;

int s1Pos;
int s2Pos;
 
enum working_mode{
MANU,
AUTO  
};
working_mode mode = AUTO;

//------------     FUNCTIONS      ---------------------
void myPinMode(void);
void blinkInternalLed(void);
void updateBtn(void);
void update_fifo(void);
void closeDoor(void);
void openDoor(void);
void ifTurnedOff(void);
void doorLogic(void);


void setup() {
  Serial.begin(9600);
  servo1.attach(SERVO1_PIN);  // attaches the servo on pin 9 to the servo object
  servo2.attach(SERVO2_PIN);
  servoDoor.attach(SERVODOOR_PIN);
  //door.attach();
  myPinMode();
  if (mode == AUTO)
  {
   servo1.write(70);
   servo2.write(130);
   delay(1000);
  }
}


void loop() 
{
  // OFF STATE
  ifTurnedOff();

  // ON STATE
  currentMillis = millis();

  // LEDs
  analogWrite(LED_PIN, 140);
  blinkInternalLed();

  // MANUAL mode - drive servos with potentiometers 
  if (mode == MANU){manual_mode();}
  
  updateBtn();
  //printStatus();
  doorLogic();




  if (nextBtnToPress != prevBtnToPress and nextBtnToPress > 0 and prevBtnToPress >0)
  { 
    dellayWaitPosMills  = currentMillis +55*abs(prevBtnToPress-nextBtnToPress)+75;  //+waitPosDellayInterval
  }
  if (nextBtnToPress >0 and dellayWaitPosMills > currentMillis)
  {
    //goToWaitPos();
    s1Pos = servo1_pos[nextBtnToPress-1];
    s2Pos = servo2_wait_pos; 
  }
  else if (currentMillis > dellayWaitPosMills and nextBtnToPress >0)
  {
    //pressButton();
    s1Pos = servo1_pos[nextBtnToPress-1];
    s2Pos = servo2_pos[nextBtnToPress-1];
  }
  else
  {
    s1Pos = servo1_pos[2];
    s2Pos = servo2_down_pos; 
  }


  servo1.write(s1Pos);
  servo2.write(s2Pos);

  if (nextBtnToPress >0)
  {
    prevBtnToPress = nextBtnToPress;
  }
  previousMillis = currentMillis;
}







void doorLogic()
{ 

  if (currentMillis > lastRobotPressedBtnTime+doorDellayInterval and nextBtnToPress ==0)
  {
    closeDoor();
  }
  else if (nextBtnToPress > 0)
  {
    openDoor();
  }
} 

void myPinMode(){
  pinMode(BTN_ON_OFF, INPUT_PULLUP);
  pinMode(BTN0, INPUT_PULLUP);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}


void manual_mode(){
   pot1 = analogRead(POT1PIN);
   pot2 = analogRead(POT2PIN);
   pos1 = map(pot1, 0, 1023, 10, 122); // limit servos 
   pos2 = map(pot2, 0, 1023, 30, 127);
   servo1.write(pos1);
   servo2.write(pos2);
}
  


void blinkInternalLed(){
   if (currentMillis - previousMillis >= intervalLed) 
   {
    // save the last time 
      if (ledState  == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(LED_BUILTIN, ledState);   // turn the LED on (HIGH is the voltage level)  
   }
}

void updateBtn()
{
  btn_state[0] = digitalRead(BTN0);
  btn_state[1] = digitalRead(BTN1);
  btn_state[2] = digitalRead(BTN2);
  btn_state[3] = digitalRead(BTN3);
  btn_state[4] = digitalRead(BTN4);
  
  for(int i = 0;i<5; i++)
  {
    if(btn_state[i]== BTN_OFF and nextBtnToPress == i+1)
    {
      nextBtnToPress = 0;
      lastRobotPressedBtnTime = millis();
    }
  }
  
  if (nextBtnToPress == 0)
  {
    for(int i = 0;i<5;i++)
    {
     if(btn_state[i] == BTN_ON and lastRobotPressedBtnTime + dellayBackDriving < millis())
     {
        nextBtnToPress = i+1;
        break;
     }
    }
  }
}

void openDoor()
{
  servoDoor.write(44);
}

void closeDoor()
{
  servoDoor.write(117);
}

void ifTurnedOff()
{
  while (digitalRead(BTN_ON_OFF)==HIGH)
  {
    Serial.print("Turned OFF");
    analogWrite(LED_PIN, 0);
    //servo1.write(17);
    servo2.write(160);
    updateBtn();
    closeDoor();
    //delay(1000);
  }
}






