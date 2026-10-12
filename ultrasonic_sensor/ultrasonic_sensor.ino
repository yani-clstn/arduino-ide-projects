#include <Servo.h>

Servo myServo;

const int trigPin = 9;
const int echoPin = 10;
const int buzzerPin = 3;   // Add your buzzer pin here
const int ledPin = 4;      // Add your LED pin here

long duration;
int distance;

// Define angles
int idleAngle = 0;       // Servo resting position
int pressAngle = 90;     // Servo presses sanitizer

void setup() {
  myServo.attach(6);
  myServo.write(idleAngle);  // Start in idle

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
}

void loop() {

  // Trigger ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // cm

  Serial.print("Distance: ");
  Serial.println(distance);

  // Check if hand is detected
  if (distance > 0 && distance <= 15) {
    
    // Turn LED ON
    digitalWrite(ledPin, HIGH);

    // --- 2 BUZZER BEEPS ---
    for (int i = 0; i < 2; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(150);
      digitalWrite(buzzerPin, LOW);
      delay(150);
    }

    // SERVO PRESS ACTION
    myServo.write(pressAngle);  // Press sanitizer
    delay(1000);                // Hold for 1 second
    
    myServo.write(idleAngle);   // Return to idle

    // Turn LED OFF right after action
    digitalWrite(ledPin, LOW);
  }

  delay(200);  // Short delay for next reading
}