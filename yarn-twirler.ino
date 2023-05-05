// DEBUGGER: {{{
#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else 
#define debug(x)
#define debugln(x)
#endif
// -=-=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGER: }}}

// MISC: {{{
// int rotations[] = {1, 3, 218, 110, 143, 60, 73, 325, 245, 148, 51, 42};
int rotations[] = {218, 110, 143, 60, 73, 30, 37, 130, 120, 95 };
int deBounce=20;
// -=-=-=-=-=-=-=-=-=-=-=-=-=- MISC: }}}

// COUNTER: {{{
int countDown = 0;
int countPrev = 0;
const int cntPin = 3;
int cntState = HIGH;
int cntStatePrev = HIGH;
int selectOption=0;
// -=-=-=-=-=-=-=-=-=-=-=-=-=- COUNTER: }}}


// LIGHTS: {{{
const int whiteLed=5;
// const int BlueLed=5;
const int greenLed=7;
const int redLed=13;
// -=-=-=-=-=-=-=-=-=-=-=-=-=- LIGHTS: }}}


// BUTTONS: {{{
// stop
const int BpRed = 2;
// select
const int BpWhite = 4;
// manual
const int BpBlue = 6;
// go
const int BpGreen = 8;
int selectState = HIGH;
int selectStatePrev = HIGH;
int goState = HIGH;
int goStatePrev = HIGH;
// -=-=-=-=-=-=-=-=-=-=-=-=-=- BUTTONS: }}}


// MOTOR: {{{
#include <Servo.h>
Servo myservo;
int motorPin = 9;
volatile int motorSet = 0;
volatile int motorPaused = 0;

const int rampStop = 10;
const int rampUp = 280;
const int rampDown = 420;
// ^^ smaller means faster
const int halfAtCnt = 7;
const int slowAtCnt = 2;
const int motorFull = 55; 
const int motorHalf = 53;
const int motorSlow = 49;
const int motorStop = 0;
const int motorTouch = 30;
int motorSpeed = 10;
// -=-=-=-=-=-=-=-=-=-=-=-=-=- Motor: }}}

// LCD: {{{
#include <LiquidCrystal_74HC595.h>
#define DS 10
#define SHCP 11
#define STCP 12
#define RS 1
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6
LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7);
// -=-=-=-=-=-=-=-=-=-=-=-=-=- LCD: }}}

void setup () {
  Serial.begin(115200);

  pinMode(redLed, OUTPUT);
  pinMode(whiteLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  pinMode(cntPin, INPUT_PULLUP);

  pinMode(BpRed, INPUT_PULLUP);
  pinMode(BpWhite, INPUT_PULLUP);
  pinMode(BpGreen, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BpRed), panicStop, FALLING);

  myservo.attach(motorPin);
  myservo.write(motorSpeed);

  // LCD setCursor: column,row
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Dragon Strings");
  lcd.setCursor(0,1);
  lcd.print("Twirler Bot");

  debugln("Dragon Strings");
  debugln("Twirler Bot");
} // void setup

void loop () {
  // COUNT REVOLUTIONS: {{{
  cntState = digitalRead(cntPin);

  // Stop counting down 1200 times a second...
  if (cntState != cntStatePrev)
  {
    if (cntState == LOW)
    {
      countDown--;
      debug("R: ");
      debugln(countDown);
      debug("S: ");
      debugln(motorSpeed);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rotations: ");
      lcd.print(rotations[selectOption]);
      lcd.setCursor(0, 1);
      lcd.print("Remaining: "); // but not really stopping or anything
      lcd.print(countDown);
      delay(deBounce);
    }
    cntStatePrev = cntState;
  }
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-COUNT REVOLUTIONS: }}}

  
  // MOTOR CONTROL: {{{
  if (motorPaused==0) {
    if (countDown <= 0)
    {
      motorSet=motorStop;
      if (countDown != countPrev) {
        countPrev=countDown;
        debugln("Motor Stop!");
        debug("Speed: ");
        debugln(motorSpeed);
      }
    }
    else if (countDown <= slowAtCnt)
    {
      motorSet=motorSlow;
      if (countDown != countPrev) {
        countPrev=countDown;
        debugln("Motor Slow...");
        debug("Speed: ");
        debugln(motorSpeed);
      }
    }
    else if (countDown <= halfAtCnt)
    {
      motorSet=motorHalf;
      if (countDown != countPrev) {
        countPrev=countDown;
        debugln("Motor Half");
        debug("Speed: ");
        debugln(motorSpeed);
      }
    }
  }
  
  // controlled speedup & slowdown: {{{
  static unsigned long rampUpCheck;
  if (millis() - rampUpCheck >= rampUp)
  {
    rampUpCheck = millis();
    if (motorSpeed < motorSet && countDown > 0)
    { /* speed up */
      motorSpeed++;
      debug("MotorSet: ");
      debugln(motorSet);
      debug("Speed up: ");
      debugln(motorSpeed);
      myservo.write(motorSpeed);
    }
  } // END if (millis() - rampUpCheck >= rampUp)

  static unsigned long rampDownCheck;
  if (millis() - rampDownCheck >= rampDown)
  {
    rampDownCheck = millis();
    if (motorSpeed>motorSet)
    { /* slow down */
      motorSpeed--;
      if (motorSpeed>motorStop) {
        debug("MotorSet: ");
        debugln(motorSet);
        debug("Slow Down: ");
        debugln(motorSpeed);
      }
      myservo.write(motorSpeed);
      if (motorSpeed<motorSlow) {
        motorSpeed=motorStop + 1;
      }
    }
  } // END if (millis() - rampDownCheck >= rampDown)
  // end controlled speedup & slowdown: }}}

  // -=-=-=-=-=-=-=-=-=-=-=-=-=- MOTOR CONTROL: }}}

  // SELECTING ROTATIONS {{{
  selectState = digitalRead(BpWhite);
  if (selectState != selectStatePrev)
  {
    if (selectState == LOW)
    {
      if (motorPaused == 1)
      {
        selectOption = -1;
        countDown = 0;
        motorPaused = 0;
        motorSpeed = 0;
        myservo.write(motorSpeed);
      }
      
      selectOption++;
      if (selectOption >= sizeof(rotations) / sizeof(rotations[0]))
      {
        selectOption = 0;
      }

      debug("Option: ");
      debugln(selectOption);
      debug("countDown: ");
      debugln(rotations[selectOption]);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rotations: ");
      lcd.print(rotations[selectOption]);
      lcd.setCursor(0, 1);
      lcd.print("Press Go"); // but not really stopping or anything
      delay(deBounce);
    }

    selectStatePrev = selectState;
  }

  // Accept & Start Rotations
  goState = digitalRead(BpGreen);
  if (goState != goStatePrev)
  {
    if (goState == LOW)
    {
      debugln("BpGreen");
      debug("Rotations: ");
      debugln(rotations[selectOption]);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Rotations: ");
      lcd.print(rotations[selectOption]);

      if (countDown <= 0)
      {
        countDown = rotations[selectOption];
        motorPaused = 1;
      }

      if (countDown > 0 && motorPaused == 0)
      {
        motorSet = motorStop;
        motorPaused = 1;
        lcd.setCursor(0, 1);
        lcd.print("Paused...");
        debugln("Paused");
      }
      else
      {
        motorSet = motorFull;
        motorPaused = 0;
        motorSpeed = motorTouch;
        myservo.write(motorSpeed);
        lcd.setCursor(0, 1);
        lcd.print("Starting Up...");
        debugln("Starting Up...");
      }

      delay(deBounce);
    }
    goStatePrev = goState;
  }
  // -=-=-=-=-=-=-=-=-=-=-=-=-=- SELECTING & RESETTING ROTATIONS }}}


  // LIGHTS LOGICS: {{{
  digitalWrite(whiteLed, LOW);
  if (motorPaused == 1) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  } else {
    digitalWrite(redLed, LOW);
    if (motorSpeed>motorStop && countDown>0)
    {
      digitalWrite(greenLed, HIGH);
    }
    else {
      digitalWrite(greenLed, LOW);
      digitalWrite(whiteLed, HIGH);
    }
  }
  // -=-=-=-=-=-=-=-=-=-=-=-=-=- LIGHTS LOGICS: }}}

} // void loop

void panicStop()
{
  motorSet = motorStop;
  motorPaused = 1;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rotations: ");
  lcd.print(rotations[selectOption]);
  lcd.setCursor(0, 1);
  lcd.print("Panic!");

  while (motorSpeed > motorStop)
  {
    debugln("Stopping Motor");
    motorSpeed--;
    if (motorSpeed < motorSlow + 4)
    {
      motorSpeed = motorStop;
    }
    myservo.write(motorSpeed);

    debug("motorSpeed: ");
    debugln(motorSpeed);
    delay(rampStop);
  } // while (motorSpeed > motorStop)

} // void panicStop