#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

byte XNOR[] = {
  B00000,
  B01110,
  B10101,
  B11111,
  B10101,
  B01110,
  B00000,
  B00000
};
byte XOR[] = {
  B00000,
  B01110,
  B10001,
  B10101,
  B10001,
  B01110,
  B00000,
  B00000
};

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, XNOR);
  lcd.createChar(1, XOR);
  lcd.clear();
}

void loop() { 
  lcd.setCursor(0, 0);
  lcd.print("(A B) C");
  lcd.setCursor(2, 0);
  lcd.write((byte)0);
  lcd.setCursor(5, 0);
  lcd.write((byte)1);
  delay(500);
}