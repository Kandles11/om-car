#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <Arduino.h>

void setup() {
    // Serial.begin(9600);
    // Serial.println("CAN Write - Testing transmission of CAN Bus messages");
    // delay(1000);

    Canbus.init(CANSPEED_500);

    // if(Canbus.init(CANSPEED_500))  
    //     //Initialise MCP2515 CAN controller at the specified speed
    //     Serial.println("CAN Init ok");
    // else
    //     Serial.println("Can't init CAN");
    delay(1000);
}

void loop() 
{
    int sensorValue = analogRead(A1);
    // Serial.print("Sending value: ");
    // Serial.println(sensorValue);

    tCAN message;

    message.id = 0x100; //formatted in HEX
    message.header.rtr = 0;
    message.header.length = 8; //formatted in DEC
    message.data[0] = 0x00;
    message.data[1] = 0x00;
    message.data[2] = 0x00;
    message.data[3] = 0x00; //formatted in HEX
    message.data[4] = 0x00;
    message.data[5] = 0x00;
    message.data[6] = (sensorValue >> 8) & 0x03;
    message.data[7] = sensorValue & 0xFF;

    mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
    mcp2515_send_message(&message);
    delay(20);
}