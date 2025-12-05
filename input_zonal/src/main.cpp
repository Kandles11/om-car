#include <Arduino.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <omcar.h>

const byte headlightsPin = 3;
const byte gearSwitchPin = 4;  
const byte rightBlinkerPin = 5;
const byte leftBlinkerPin = 6;
const byte hazardLightsPin = 7;

void setup()
{
  Serial.begin(9600); //start the Serial interface at 9600 baud
  pinMode(headlightsPin, INPUT_PULLUP);  //the switch pin is an input pulled HIGH by an internal resistor
  pinMode(gearSwitchPin, INPUT_PULLUP);  //the switch pin is an input pulled HIGH by an internal resistor
  pinMode(leftBlinkerPin, INPUT_PULLUP);  //the switch pin is an input pulled HIGH by an internal resistor
  pinMode(rightBlinkerPin, INPUT_PULLUP);  //the switch pin is an input pulled HIGH by an internal resistor
  pinMode(hazardLightsPin, INPUT_PULLUP);  //the switch pin is an input pulled HIGH by an internal resistor


  Canbus.init(CANSPEED_500);
  delay(1000);
}

void loop()
{
  byte gear_switch_read = digitalRead(gearSwitchPin);  // REVERSE = HIGH, FORWARD = LOW
  byte left_switch_read = !digitalRead(leftBlinkerPin); 
  byte right_switch_read = !digitalRead(rightBlinkerPin);
  byte hazard_lights_read = !digitalRead(hazardLightsPin);

  Serial.print("Gear: ");
  Serial.print(gear_switch_read ? "REVERSE" : "FORWARD");
  Serial.print(" | Left: ");
  Serial.print(left_switch_read ? "ON" : "OFF");
  Serial.print(" | Right: ");
  Serial.print(right_switch_read ? "ON" : "OFF");
  Serial.print(" | Hazard: ");
  Serial.print(hazard_lights_read ? "ON" : "OFF");
  Serial.println();

  // LIGHT CAN MESSAGE
  tCAN light_can_msg;
  omcar_light_controls_t light_msg;
  light_msg.left_blinker_switch_state = left_switch_read;
  light_msg.right_blinker_switch_state = right_switch_read;
  light_msg.hazard_lights_switch_state = hazard_lights_read;

  light_can_msg.id = OMCAR_LIGHT_CONTROLS_FRAME_ID;
  light_can_msg.header.rtr = 0;
  light_can_msg.header.length = OMCAR_LIGHT_CONTROLS_LENGTH;
  omcar_light_controls_pack(light_can_msg.data, &light_msg, OMCAR_LIGHT_CONTROLS_LENGTH);

  // GEAR CAN MESSAGE
  tCAN gear_can_msg;
  omcar_gear_select_t gear_msg;
  gear_msg.gear_control_switch_state = gear_switch_read;

  gear_can_msg.id = OMCAR_GEAR_SELECT_FRAME_ID;
  gear_can_msg.header.rtr = 0;
  gear_can_msg.header.length = OMCAR_GEAR_SELECT_LENGTH;
  omcar_gear_select_pack(gear_can_msg.data, &gear_msg, OMCAR_GEAR_SELECT_LENGTH);

  // SEND CAN MESSAGES
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&light_can_msg);

  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&gear_can_msg);

  delay(20); 
}