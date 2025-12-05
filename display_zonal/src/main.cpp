#include <Wire.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include <Canbus.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <omcar.h>

// Full-buffer I2C SSD1309
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

float speed = 0;
float odometer = 123.4;
bool headlightOn = false;
bool blinkerLeft = false;
bool blinkerRight = false;
bool hazardLights = false;
int pedal = 0;   // 0–100 accelerator pedal position
bool gear = false;  // false = forward, true = reverse

unsigned long lastUpdate = 0;
unsigned long lastBlinkToggle = 0;
bool blinkVisible = false;

// Track previous values to only update when changed
int lastPedal = -1;
int lastSpeedInt = -1;
unsigned long lastOdometerInt = 0;
bool lastHeadlightOn = false;
bool lastBlinkerLeft = false;
bool lastBlinkerRight = false;
bool lastBlinkVisible = false;
bool lastGear = false;
bool lastHazardLights = false;

// Pre-calculated constants
const int barX = 120;
const int barY = 7;
const int barW = 6;
const int barH = 50;
const int kmhLabelWidth = 24;  // Pre-calculated width of "km/h"
const int kmhLabelX = (128 - kmhLabelWidth) / 2;

void setup() {
  Wire.begin();
  Wire.setClock(800000);  // Set I2C to 800kHz (faster than 400kHz, if display supports it)
  u8g2.begin();
  Serial.begin(115200);
  Canbus.init(CANSPEED_500);
}


void loop() {
  unsigned long now = millis();

  // Read CAN messages (non-blocking, check multiple times per loop)
  tCAN message;
  if (mcp2515_check_message())
  {
    if (mcp2515_get_message(&message))
    {
      if (message.id == OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_FRAME_ID && message.header.length == OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_LENGTH)
      {
        omcar_requested_motor_power_message_t motor_power_msg;
        omcar_requested_motor_power_message_unpack(&motor_power_msg, message.data, OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_LENGTH);
        pedal = motor_power_msg.requested_motor_power;
        if (pedal < 0) pedal = 0;
        if (pedal > 100) pedal = 100;
      }
      else if (message.id == OMCAR_GEAR_SELECT_FRAME_ID && message.header.length == OMCAR_GEAR_SELECT_LENGTH)
      {
        omcar_gear_select_t gear_select_msg;
        omcar_gear_select_unpack(&gear_select_msg, message.data, OMCAR_GEAR_SELECT_LENGTH);
        gear = gear_select_msg.gear_control_switch_state;
      }
      else if (message.id == OMCAR_LIGHT_CONTROLS_FRAME_ID && message.header.length == OMCAR_LIGHT_CONTROLS_LENGTH)
      {
        omcar_light_controls_t light_controls_msg;
        omcar_light_controls_unpack(&light_controls_msg, message.data, OMCAR_LIGHT_CONTROLS_LENGTH);
        headlightOn = light_controls_msg.headlight_switch_state;
        blinkerLeft = light_controls_msg.left_blinker_switch_state;
        blinkerRight = light_controls_msg.right_blinker_switch_state;
        hazardLights = light_controls_msg.hazard_lights_switch_state;
      }
    }
  }

  // Simulate data (only for speed and odometer, lights come from CAN)
  if (now - lastUpdate > 100) {
    lastUpdate = now;
    speed += 0.5;
    if (speed > 45) speed = 0;
    odometer += speed * 0.001;
  }

  // Blinker flashing
  if (now - lastBlinkToggle > 500) {
    blinkVisible = !blinkVisible;
    lastBlinkToggle = now;
  }

  // Check what changed - only update display when something actually changed
  int speedInt = (int)speed;
  unsigned long odometerInt = (unsigned long)odometer;
  bool pedalChanged = (pedal != lastPedal);
  bool speedChanged = (speedInt != lastSpeedInt);
  bool odometerChanged = (odometerInt != lastOdometerInt);
  bool headlightChanged = (headlightOn != lastHeadlightOn);
  bool blinkerLeftChanged = (blinkerLeft != lastBlinkerLeft);
  bool blinkerRightChanged = (blinkerRight != lastBlinkerRight);
  bool blinkVisibleChanged = (blinkVisible != lastBlinkVisible);
  bool gearChanged = (gear != lastGear);
  bool hazardLightsChanged = (hazardLights != lastHazardLights);
  
  bool shouldUpdate = pedalChanged || speedChanged || odometerChanged || 
                      headlightChanged || blinkerLeftChanged || blinkerRightChanged || blinkVisibleChanged || gearChanged || hazardLightsChanged;

  if (shouldUpdate) {
    // Update tracking variables
    if (pedalChanged) lastPedal = pedal;
    if (speedChanged) lastSpeedInt = speedInt;
    if (odometerChanged) lastOdometerInt = odometerInt;
    if (headlightChanged) lastHeadlightOn = headlightOn;
    if (blinkerLeftChanged) lastBlinkerLeft = blinkerLeft;
    if (blinkerRightChanged) lastBlinkerRight = blinkerRight;
    if (blinkVisibleChanged) lastBlinkVisible = blinkVisible;
    if (gearChanged) lastGear = gear;
    if (hazardLightsChanged) lastHazardLights = hazardLights;

    // --- DRAW ---
    u8g2.clearBuffer();

    // Draw large speed (integer only) - optimized
    u8g2.setFont(u8g2_font_fub30_tr);
    char speedStr[8];
    itoa(speedInt, speedStr, 10);
    uint8_t speedWidth = u8g2.getStrWidth(speedStr);
    u8g2.drawStr((128 - speedWidth) / 2, 42, speedStr);

    // Label - use pre-calculated position
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(kmhLabelX, 56, "km/h");

    // Odometer - optimized
    char odoStr[16];
    snprintf(odoStr, sizeof(odoStr), "ODO: %lu", odometerInt);
    u8g2.drawStr(0, 63, odoStr);

    // Headlight
    if (headlightOn) {
      u8g2.drawStr(100, 10, "HL");
    }

    // Gear display (Forward/Reverse)
    u8g2.setFont(u8g2_font_6x10_tr);
    if (gear) {
      u8g2.drawStr(0, 10, "R");  // Reverse
    } else {
      u8g2.drawStr(0, 10, "F");  // Forward
    }

    // Hazard lights indicator
    if (hazardLights) {
      u8g2.drawStr(10, 10, "!");  // Hazard indicator
    }

    // Blinkers - if hazards are on, both blinkers flash together
    if (blinkVisible) {
      bool showLeft = hazardLights || blinkerLeft;
      bool showRight = hazardLights || blinkerRight;
      
      if (showLeft) {
        u8g2.drawTriangle(5, 30, 15, 25, 15, 35);
      }
      if (showRight) {
        u8g2.drawTriangle(123, 30, 113, 25, 113, 35);
      }
    }

    // --- Accelerator Pedal Bar ---
    // Outline
    u8g2.drawFrame(barX, barY, barW, barH);

    // Fill based on pedal % - optimized calculation
    int fillH = (pedal * barH) / 100;

    // Draw bottom-up fill (ensure non-negative height)
    if (fillH > 0) {
      int boxHeight = fillH - 2;
      if (boxHeight < 0) boxHeight = 0;
      u8g2.drawBox(barX + 1, barY + barH - fillH + 1, barW - 2, boxHeight);
    } 

    u8g2.sendBuffer();
  }
}