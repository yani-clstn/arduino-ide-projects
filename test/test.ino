#define IR_PIN 34
#define LED_PIN 23
#define BUZZER_PIN 14

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(IR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // LED self-test: should blink 3 times
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH); delay(300);
        digitalWrite(LED_PIN, LOW);  delay(300);
    }
    Serial.println("LED self-test done");
}

void loop() {
    int raw = digitalRead(IR_PIN);
    int analog = analogRead(IR_PIN);
    Serial.print("IR digital: ");
    Serial.print(raw);
    Serial.print("   analog: ");
    Serial.println(analog);

    digitalWrite(LED_PIN, raw == LOW ? HIGH : LOW);
    delay(300);
}