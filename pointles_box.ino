/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

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

const long intervalLed = 1000;
const long intervalPrintStatus= 1000;
unsigned long previousMillis = 0; 
unsigned long currentMillis;
unsigned long dellayDoorMills;
unsigned long doorDellayInterval = 400;
unsigned long dellayWaitPosMills;
unsigned long waitPosDellayInterval = 500;


int ledState = LOW;  
bool btn_state[5];
int nextBtnToPress=0;
int prevBtnToPress=0;
bool waitPointReached;

int servo1_pos[5] = {17,41,71,99,130};
int servo2_pos[5] = {61,61,61,61,61};
int servo2_wait_pos = 75;
int servo2_down_pos = 136;

int s1Pos;
int s2Pos;
 
enum working_mode{
MANU,
AUTO  
};
working_mode mode = AUTO;

//------------     FUNCTIONS      ---------------------
void printStatus(void);
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




  if (nextBtnToPress != prevBtnToPress and nextBtnToPress > 0)
  { 
    dellayWaitPosMills  = currentMillis + waitPosDellayInterval;
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
  
  prevBtnToPress = nextBtnToPress;
  previousMillis = currentMillis;
}







void doorLogic()
{ 
  if (prevBtnToPress >0 and nextBtnToPress == 0)
  {
    dellayDoorMills = currentMillis + doorDellayInterval;
  }
  if (currentMillis > dellayDoorMills and nextBtnToPress ==0)
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
    printStatus();
      if (ledState  == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(LED_BUILTIN, ledState);   // turn the LED on (HIGH is the voltage level)  
   }
}

void printStatus()
{
      Serial.print("pot1 =");
  Serial.println(pot1);
  Serial.print("pot2 =");
  Serial.println(pot2);
  Serial.print("pos1 =");
  Serial.println(pos1);
  Serial.print("pos2 =");
  Serial.println(pos2);
  Serial.print(btn_state[0]);
  Serial.print(btn_state[1]);
  Serial.print(btn_state[2]);
  Serial.print(btn_state[3]);
  Serial.print(btn_state[4]);      

  Serial.print("\nnext btn to press");
  Serial.println(nextBtnToPress);
  Serial.println();
  Serial.println();
  
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
    if(btn_state[i]==1 and nextBtnToPress == i+1)
    {
      nextBtnToPress = 0;
    }
  }
  
  if (nextBtnToPress == 0)
  {
    for(int i = 0;i<5;i++)
    {
     if(btn_state[i] == 0)
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
    analogWrite(LED_PIN, 0);
    servo1.write(80);
    servo2.write(138);
    updateBtn();
    printStatus();
    closeDoor();
    delay(1000);
  }
}



