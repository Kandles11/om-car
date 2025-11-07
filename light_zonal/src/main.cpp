#include <Arduino.h>
#include <Wire.h>
#include <Canbus.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <omcar.h>

int left_blinker = 7;
int left_brake = 6;
int left_tail = 5;
int right_tail = 4;
int right_brake = 3;
int right_blinker = 2;

bool left_blinker_status = false;
bool left_brake_status = false;
bool left_tail_status = false;
bool right_tail_status = false;
bool right_brake_status = false;
bool right_blinker_status = false;

void setup() {
  Serial.begin(9600);
  pinMode(left_blinker, OUTPUT);
  pinMode(left_brake, OUTPUT);
  pinMode(left_tail, OUTPUT);
  pinMode(right_tail, OUTPUT);
  pinMode(right_brake, OUTPUT);
  pinMode(right_blinker, OUTPUT);

  Canbus.init(CANSPEED_500);
  Serial.println("setup complete");
}

void loop() {
  tCAN message;
  if (mcp2515_check_message())
    {
        if (mcp2515_get_message(&message))
        {
            Serial.println("i got some message");
            if (message.id == OMCAR_LIGHT_CONTROLS_FRAME_ID && message.header.length == OMCAR_LIGHT_CONTROLS_LENGTH)
            {
                Serial.println("i got a lighting message");
                omcar_light_controls_t light_controls_msg;
                omcar_light_controls_unpack(&light_controls_msg, message.data, OMCAR_LIGHT_CONTROLS_LENGTH);
                Serial.print("Setting Left to ");
                Serial.println(light_controls_msg.left_blinker_switch_state);
                Serial.print("Setting Right to ");
                Serial.println(light_controls_msg.right_blinker_switch_state);
                left_blinker_status = light_controls_msg.left_blinker_switch_state;
                right_blinker_status = light_controls_msg.right_blinker_switch_state;
            }
        }
    }
    digitalWrite(left_blinker, left_blinker_status? HIGH : LOW);
    digitalWrite(right_blinker, right_blinker_status? HIGH : LOW);
    delay(20);
}