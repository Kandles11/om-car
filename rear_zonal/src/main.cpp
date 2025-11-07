#include <Wire.h>
#include "MCP4728.h"
#include <Arduino.h>
#include <Canbus.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <omcar.h>

MCP4728 dac;
MCP4728 dac2;
void printStatus();


// Reads A0 on a 10-bit Arduino (0..1023). Assumes VREF = 5.0V (or change to 3.3).
const int acceleratorPedalPin = A1;
const float VREF = 5.0;         // set to 3.3 for 3.3V boards
const int ADC_MAX = 1023;       // 10-bit
int accelPedalValue = 0;
double accelPedalValueScaled = 0.0;

void setup()
{
    Serial.begin(115200);  // initialize serial interface for print()

    Wire.begin();
    delay(100);
    Serial.println("Starting...");
    dac.attach(Wire, 14);
    dac2.attach(Wire, 14);
    dac.readRegisters();
    dac2.readRegisters();
    dac2.setID(1);


    dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
    dac.selectPowerDown(MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_500KOHM, MCP4728::PWR_DOWN::GND_500KOHM);
    dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
    dac.analogWrite(MCP4728::DAC_CH::A, 111);
    dac.analogWrite(MCP4728::DAC_CH::B, 222);
    dac.analogWrite(MCP4728::DAC_CH::C, 333);
    dac.analogWrite(MCP4728::DAC_CH::D, 444);

    dac2.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
    dac2.selectPowerDown(MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_100KOHM, MCP4728::PWR_DOWN::GND_500KOHM, MCP4728::PWR_DOWN::GND_500KOHM);
    dac2.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
    dac2.analogWrite(MCP4728::DAC_CH::A, 111);
    dac2.analogWrite(MCP4728::DAC_CH::B, 222);
    dac2.analogWrite(MCP4728::DAC_CH::C, 333);
    dac2.analogWrite(MCP4728::DAC_CH::D, 444);

    dac.enable(true);
    dac2.enable(true);

    dac.readRegisters();
    // printStatus();

    Serial.println("Finished setting up DACs!");

    delay(50);
    dac.analogWrite(2458, 2492, 2542, 0);
    dac2.analogWrite(2542, 2458, 0,0);
    delay(50);

    Canbus.init(CANSPEED_500);
    // if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
    //     Serial.println("CAN Init ok");
    //     continue;
    // else
    //     Serial.println("Can't init CAN");
}


void setMotors(double dutyCycle)
{
    int center = 2500;
    int range = 1390;
    int diff = 50;

    int dutyCycleCenter = center + (int)(((double)range)*dutyCycle);

    dac.analogWrite(dutyCycleCenter - diff, 2492, dutyCycleCenter + diff, 0);
}


void loop()
{
    tCAN message;
    if (mcp2515_check_message())
    {
        if (mcp2515_get_message(&message))
        {
            if (message.id == OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_FRAME_ID && message.header.length == OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_LENGTH)
            {
                omcar_requested_motor_power_message_t motor_power_msg;
                omcar_requested_motor_power_message_unpack(&motor_power_msg, message.data, OMCAR_REQUESTED_MOTOR_POWER_MESSAGE_LENGTH);
                accelPedalValue = (motor_power_msg.requested_motor_power);
                accelPedalValueScaled = (double)accelPedalValue/100.0;
                Serial.println(accelPedalValueScaled);
                // Serial.print("Accel pedal value: ");
                // Serial.println(accelPedalValue);
            }
        }
    }
    setMotors(accelPedalValueScaled);
    delay(20);
}

void printStatus()
{
    Serial.println("NAME     Vref  Gain  PowerDown  DACData");
    for (int i = 0; i < 4; ++i)
    {
        Serial.print("DAC");
        Serial.print(i, DEC);
        Serial.print("   ");
        Serial.print("    ");
        Serial.print(dac.getVref(i), BIN);
        Serial.print("     ");
        Serial.print(dac.getGain(i), BIN);
        Serial.print("       ");
        Serial.print(dac.getPowerDown(i), BIN);
        Serial.print("       ");
        Serial.println(dac.getDACData(i), DEC);

        Serial.print("EEPROM");
        Serial.print(i, DEC);
        Serial.print("    ");
        Serial.print(dac.getVref(i, true), BIN);
        Serial.print("     ");
        Serial.print(dac.getGain(i, true), BIN);
        Serial.print("       ");
        Serial.print(dac.getPowerDown(i, true), BIN);
        Serial.print("       ");
        Serial.println(dac.getDACData(i, true), DEC);
    }
    Serial.println(" ");
}