#include <Wire.h>
#include <U8g2lib.h>
#include <Arduino.h>

// Full-buffer I2C SSD1309
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

float speed = 0;
float odometer = 123.4;
bool headlightOn = false;
bool blinkerLeft = false;
bool blinkerRight = false;

unsigned long lastUpdate = 0;
unsigned long lastBlinkToggle = 0;
bool blinkVisible = false;

void setup() {
  u8g2.begin();
}


void loop() {
  unsigned long now = millis();

  // Simulate data
  if (now - lastUpdate > 100) {
    lastUpdate = now;
    speed += 0.5;
    if (speed > 45) speed = 0;
    odometer += speed * 0.001;

    headlightOn = ((now / 5000) % 2) == 1;
    int phase = (now / 3000) % 3;
    blinkerLeft  = (phase == 1);
    blinkerRight = (phase == 2);
  }

  // Blinker flashing
  if (now - lastBlinkToggle > 500) {
    blinkVisible = !blinkVisible;
    lastBlinkToggle = now;
  }

  // --- DRAW ---
  u8g2.clearBuffer();

  // Draw large speed (integer only)
  u8g2.setFont(u8g2_font_fub30_tr);
  int speedInt = (int)speed;
  char speedStr[8];
  itoa(speedInt, speedStr, 10);  // integer to string
  uint8_t speedWidth = u8g2.getStrWidth(speedStr);
  u8g2.drawStr((128 - speedWidth) / 2, 42, speedStr);

  // Label
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr((128 - u8g2.getStrWidth("km/h")) / 2, 56, "km/h");

  // Odometer
  char odoStr[16];
  // Show integer part only for simplicity
  snprintf(odoStr, sizeof(odoStr), "ODO: %lu", (unsigned long)odometer);
  u8g2.drawStr(0, 63, odoStr);

  // Headlight
  if (headlightOn) {
    u8g2.drawStr(100, 10, "HL");
  }

  // Blinkers
  if (blinkVisible) {
    if (blinkerLeft) {
      u8g2.drawTriangle(5, 30, 15, 25, 15, 35);
    }
    if (blinkerRight) {
      u8g2.drawTriangle(123, 30, 113, 25, 113, 35);
    }
  }

  u8g2.sendBuffer();
}