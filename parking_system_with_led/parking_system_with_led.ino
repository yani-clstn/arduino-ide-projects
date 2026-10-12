#include <Servo.h>
Servo myServo;

int buttonPin = 8;
int greenLed = 11;
int redLed = 12;
int buzzerPin = 13;
int servoPin = 7;

bool isActive = false; // mode toggle: false = green mode, true = red mode
bool lastButtonState = HIGH; // because of INPUT_PULLUP
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long previousMillis = 0;
const long interval = 500; // blink interval
bool ledState = LOW;

void setup() {
  myServo.attach(servoPin);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // start in green mode
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
  myServo.write(0);
  noTone(buzzerPin);
}

void loop() {
  int reading = digitalRead(buttonPin);

  // debounce logic
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      // toggle mode
      isActive = !isActive;

      // immediately apply mode state
      if (!isActive) {
        // return to green mode
        digitalWrite(greenLed, HIGH);
        digitalWrite(redLed, LOW);
        myServo.write(0);
        noTone(buzzerPin);
      }
    }
  }

  lastButtonState = reading;

  // if active, blink red LED + buzzer + servo
  if (isActive) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(redLed, ledState);

      if (ledState) {
        tone(buzzerPin, 440);
      } else {
        noTone(buzzerPin);
      }
    }

    digitalWrite(greenLed, LOW);
    myServo.write(90);
  }
}

