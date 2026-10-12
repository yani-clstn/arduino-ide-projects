#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 LCD

void scrollText(String message, int delayTime) {
  lcd.clear();
  message = " " + message + " "; // spacing for smooth edges

  for (int i = 0; i < message.length() - 15; i++) {
    lcd.setCursor(0, 0);
    lcd.print(message.substring(i, i + 16));
    delay(delayTime);
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  scrollText("You're the coffee that I need in the morning", 80);
  delay(1000);

  scrollText("You're my sunshine in the rain", 100);
  delay(400);

  scrollText("when it's pouring", 100);
  delay(1000);

  scrollText("Won't you give yourself to me, give it all", 100);
  delay(1500);

  scrollText("I just wanna see", 80);
  delay(2000);

  scrollText("I just wanna see how beautiful you are ♡", 100);
  delay(2000);

  scrollText("You know that i see it", 100);
  delay(2000);

  scrollText("I know you're a star", 100);
  delay(2000);

  scrollText("Where you go, i'll follow", 100);
  delay(2000);

  scrollText("No matter how far", 100);
  delay(2000);

  scrollText("If life is a movie, know you're the best part", 200);
  delay(2000);

  

}
