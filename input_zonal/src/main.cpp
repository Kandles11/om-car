#include <Arduino.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <omcar.h>

const byte switchPin0 = 4;  

void setup()
{
  Serial.begin(9600); //start the Serial interface at 9600 baud
  pinMode(switchPin0, INPUT_PULLUP);  //the switch pin is an input pulled HIGH by an internal resistor

  Canbus.init(CANSPEED_500);
  delay(1000);
}

void loop()
{
  byte gear_switch_read = digitalRead(switchPin0);  //read the state of the input pin (HIGH or LOW)

  tCAN message;
  omcar_gear_select_t gear_msg;
  gear_msg.gear_control_switch_state = gear_switch_read;

  message.id = OMCAR_GEAR_SELECT_FRAME_ID;
  message.header.rtr = 0;
  message.header.length = OMCAR_GEAR_SELECT_LENGTH;

  omcar_gear_select_pack(message.data, &gear_msg, OMCAR_GEAR_SELECT_LENGTH);

  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&message);

  delay(20); 
}