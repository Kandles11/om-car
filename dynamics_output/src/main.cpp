#include <Wire.h>
#include "MCP4728.h"
#include <Arduino.h>
#include <Canbus.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

MCP4728 dac;
MCP4728 dac2;
void printStatus();

uint16_t voltageToCode(uint8_t ch, float voltage);

uint16_t channel_a_neutral; 
uint16_t channel_b_neutral; 
uint16_t channel_a_forward;
uint16_t channel_b_forward;

// Reads A0 on a 10-bit Arduino (0..1023). Assumes VREF = 5.0V (or change to 3.3).
const int acceleratorPedalPin = A1;
const float VREF = 5.0;         // set to 3.3 for 3.3V boards
const int ADC_MAX = 1023;       // 10-bit
int accelPedalValue = 0;

void setup()
{
    // Serial.begin(115200);  // initialize serial interface for print()

    Wire.begin();
    // delay(100);
    // Serial.println("Starting...");
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

    channel_a_neutral = voltageToCode(0, 2.475);
    channel_b_neutral = voltageToCode(1, 0.24675);    
    channel_a_forward = voltageToCode(0, 3.8);
    channel_b_forward = voltageToCode(1, 0.380);

    // Serial.println("Finished setting up DACs!");

    delay(50);
    dac.analogWrite(2458, 2492, 2542, 0);
    dac2.analogWrite(2542, 2458, 0,0);
    delay(50);

    Canbus.init(CANSPEED_500);
    // if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
        // Serial.println("CAN Init ok");
        // continue;
    // else
        // Serial.println("Can't init CAN");
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
            if (message.id == 0x100 && message.header.length == 8)
            {
                accelPedalValue = ((uint16_t) message.data[6] << 8) | message.data[7];
                // Serial.print("Accel pedal value: ");
                // Serial.println(accelPedalValue);
            }
        }
    }
    // const int samples = 8;
    // long sum = 0;
    // for (int i = 0; i < samples; ++i) {
    //     sum += analogRead(acceleratorPedalPin);
    //     delay(2);
    // }
    // float avg = (float)sum / samples;
    float minVoltage = 0.15;
    float maxVoltage = 0.8;
    float voltage = accelPedalValue * VREF / ADC_MAX;
    // Serial.print("Raw: ");
    // Serial.print(accelPedalValue);
    // Serial.print("  Voltage: ");
    // Serial.print(voltage, 3);
    // Serial.println(" V");
    setMotors(max(0, min(1, (voltage - minVoltage) / (maxVoltage - minVoltage))));
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

// Convert a desired output voltage to a 12-bit DAC code for channel ch.
uint16_t voltageToCode(uint8_t ch, float voltage)
{
    // Determine Vref and gain for the channel from the driver
    float vrefV = (dac.getVref(ch) == 0) ? 5.0f : 2.8f; // VREF::VDD == 0, INTERNAL_2_8V == 1
    float gain = (dac.getGain(ch) == 0) ? 1.0f : 2.0f;  // GAIN::X1 == 0, X2 == 1

    float fullScale = vrefV * gain;
    if (voltage <= 0.0f) return 0;
    if (voltage >= fullScale) return 4095;

    float ratio = voltage / fullScale;
    uint16_t code = (uint16_t)round(ratio * 4095.0f);
    if (code > 4095) code = 4095;
    return code;
}
