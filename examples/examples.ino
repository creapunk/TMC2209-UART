#include "src/TMC2209.h"
#include <HardwareSerial.h>

HardwareSerial Serial1(PB_10, PB_11); // Setting up UART in Classic mode

// To simplify the connection, a single microcontroller pin supporting Half-Duplex is sufficient
// HardwareSerial Serial1(PB_11); // Setting up UART in half-duplex mode

// Creating a driver instance for storing settings. If there are multiple chips, the number of instances equals the number of chips
TMC2209_UNIT tmc0(SerialLP1, 0); // UART instance to which the driver is connected

// For HAL operation with the library
TMC2209 driver;

void setup()
{
    Serial1.begin(500000); // Initializing Serial with the required BAUD rate

    driver.setupDefault(&tmc0); // Writing default settings to the driver
    
    for (uint8_t i = 0; i < 5; i++)
    {
        if (driver.available(&tmc0))
        {
            SerialUSB.println("Setup Complete");
            break;
        }
        else
        {
            delay(10);
            driver.setupDefault(&tmc0);
        }
    }
    if (!driver.available(&tmc0))
        SerialUSB.println("Setup Error");

    // Changing the RUN current using LL access to registers
    tmc0.IHOLD_IRUN.REG.irun = 20;
    driver.ll.writeIHOLD_IRUN(&tmc0);
}

void loop() {

}
