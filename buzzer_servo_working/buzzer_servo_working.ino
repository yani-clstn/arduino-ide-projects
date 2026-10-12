#include <Servo.h>   // Include the Servo library

Servo myServo;

// Pin assignments
int buttonPin = 8;
int greenLed = 11;
int redLed = 12;
int buzzerPin = 13;
int servoPin = 7;

// Variables for button logic
int buttonState;
int lastButtonState = HIGH;
int mode = 0;  // 0 = default (green), 1 = alert (red)
unsigned long previousMillis = 0;
int blinkInterval = 300; // speed of blinking

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Button uses pull-up resistor
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  myServo.attach(servoPin);
  myServo.write(0);

  // Start in default mode
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
  noTone(buzzerPin);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  // Detect button press (toggle when pressed once)
  if (buttonState == LOW && lastButtonState == HIGH) {
    mode = 1 - mode; // Switch between 0 and 1
    delay(200);      // Debounce delay
  }
  lastButtonState = buttonState;

  switch (mode) {
    case 0: // Default mode
      digitalWrite(greenLed, HIGH);
      digitalWrite(redLed, LOW);
      noTone(buzzerPin);
      myServo.write(0);
      break;

    case 1: // Alert mode (blink red + buzzer)
      digitalWrite(greenLed, LOW);
      myServo.write(90);

      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;

        // Toggle red LED and buzzer on/off
        int redState = digitalRead(redLed);
        if (redState == LOW) {
          digitalWrite(redLed, HIGH);
          tone(buzzerPin, 440);
        } else {
          digitalWrite(redLed, LOW);
          noTone(buzzerPin);
        }
      }
      break;
  }
}
