#include <Arduino.h>
#include <Wire.h>
#include <Canbus.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <omcar.h>

void setup() {
  Serial.begin(9600);
  Canbus.init(CANSPEED_500);
  Serial.println("setup complete");
  delay(50);
}

void loop() {
  tCAN message;
  omcar_light_controls_t lights_msg;
  lights_msg.left_blinker_switch_state = 1;
  message.id = OMCAR_LIGHT_CONTROLS_FRAME_ID;
  message.header.rtr = 0;
  message.header.length = OMCAR_LIGHT_CONTROLS_LENGTH;
  omcar_light_controls_pack(message.data, &lights_msg, OMCAR_LIGHT_CONTROLS_LENGTH);
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&message);
  Serial.println("Sent Message!");
  delay(500);

  lights_msg.left_blinker_switch_state = 0;
  omcar_light_controls_pack(message.data, &lights_msg, OMCAR_LIGHT_CONTROLS_LENGTH);
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&message);
  Serial.println("Sent Message!");
  delay(500);

  lights_msg.right_blinker_switch_state = 1;
  omcar_light_controls_pack(message.data, &lights_msg, OMCAR_LIGHT_CONTROLS_LENGTH);
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&message);
  Serial.println("Sent Message!");
  delay(500);

  lights_msg.right_blinker_switch_state = 0;
  omcar_light_controls_pack(message.data, &lights_msg, OMCAR_LIGHT_CONTROLS_LENGTH);
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&message);
  Serial.println("Sent Message!");
  delay(500);

}
