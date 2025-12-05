#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <Arduino.h>
#include <omcar.h>

const float VREF = 5.0;       
const int ADC_MAX = 1023;  

void setup() {
    Serial.begin(9600);

    digitalWrite(8, LOW);

    Canbus.init(CANSPEED_500);
    delay(1000);
}

void loop() 
{
    int sensorValue = analogRead(A1);
    Serial.println("SENSOR VALUE");
    Serial.println(sensorValue);

    float minVoltage = 0.15;
    float maxVoltage = 0.8;
    float voltage = sensorValue * VREF / ADC_MAX;
    int dutyCycle = round((float)100* (max(0, min(1, (voltage - minVoltage) / (maxVoltage - minVoltage)))));
    Serial.println("DUTY CYCLE");
    Serial.println(dutyCycle);

    tCAN message;
    omcar_requested_motor_power_message_t power_msg;
    power_msg.requested_motor_power = dutyCycle;

    message.id = OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_FRAME_ID;
    message.header.rtr = 0;
    message.header.length = OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_LENGTH;

    omcar_requested_motor_power_message_pack(message.data, &power_msg, OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_LENGTH);

    mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
    mcp2515_send_message(&message);

    delay(20); // keep it if needed for bus pacing
}