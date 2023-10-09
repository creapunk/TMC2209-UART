/**
 * @file TMC2209_CMD.cpp
 * @author Anton Khrustalev, 2023
 */

#include "TMC2209_CMD.h"

TMC2209_CMD::TMC2209_CMD(/* args */)
{
}

uint8_t TMC2209_CMD::getStartCRC(uint8_t address)
{
    if (address == ADR_0)
        return CRC_START_ADR_0;
    if (address == ADR_1)
        return CRC_START_ADR_1;
    if (address == ADR_2)
        return CRC_START_ADR_2;
    if (address == ADR_3)
        return CRC_START_ADR_3;
    return 0;
}

uint8_t TMC2209_CMD::get4ByteCRC(uint32_t data, uint8_t crc)
{
    for (uint8_t i = 0; i < 4; ++i)
    {
        crc = get1ByteCRC((data & 0xFFU), crc);
        data >>= 8; // Get #i byte of datagram
    }
    return crc;
}

uint8_t TMC2209_CMD::get1ByteCRC(uint8_t byte, uint8_t crc)
{
    for (uint8_t j = 0; j < 0x8; ++j) // CRC calculation
    {
        if ((crc >> 7) ^ (byte & 0x01))
            crc = (crc << 1) ^ 0x07;
        else
            crc = crc << 1;
        byte = byte >> 1;
    }
    return crc;
}

uint32_t TMC2209_CMD::reverseBytes(uint32_t data)
{
    uint32_t reversed_data = 0;
    uint8_t right_shift;
    uint8_t left_shift;
    for (uint8_t i = 0; i < 4; ++i)
    {
        right_shift = (3 - i) * 8;
        left_shift = i * 8;
        reversed_data |= ((data >> right_shift) & 0xFF) << left_shift;
    }
    return reversed_data;
}

uint64_t TMC2209_CMD::writeDatagram(uint8_t address, uint8_t reg, uint32_t data)
{
    Datagram64 datagram;
    datagram.sync = SYNC;
    datagram.serial_address = address;
    datagram.register_address = reg;
    datagram.data = reverseBytes(data);
    datagram.crc = get4ByteCRC(datagram.data, get1ByteCRC(reg, getStartCRC(address)));
    return datagram.UINT64;
}

uint32_t TMC2209_CMD::requestDatagram(uint8_t address, uint8_t reg)
{
    Datagram32 datagram;
    datagram.sync = SYNC;
    datagram.serial_address = address;
    datagram.register_address = reg;
    datagram.crc = get1ByteCRC(reg, getStartCRC(address));
    return datagram.UINT32;
}

uint32_t TMC2209_CMD::respondData(uint64_t datagram)
{
    if (datagram == UINT64_MAX)
        return datagram;
    uint8_t reg = datagram >>= 16;  // datagr.register_address;
    uint32_t data = datagram >>= 8; // datagr.data;
    uint8_t crc_recieved = datagram >>= 32;
    uint8_t crc_calculated = get4ByteCRC(data, get1ByteCRC(reg, CRC_START_ADR_RESPOND));
    SerialUSB.print("Actual reg: ");
    SerialUSB.print(reg, HEX);
    SerialUSB.print("; Actual crc: ");
    SerialUSB.print(crc_recieved, HEX);
    SerialUSB.print("; Calculated crc: ");
    SerialUSB.println(crc_calculated, HEX);
    if (crc_recieved != crc_calculated)
        return UINT32_MAX;
    return reverseBytes(data);
}

/*
@TODO
Hardware timer interrupt:
1) Starts at sending message,
2) Wait [1000000 / BAUD * (32+64+REPLY_DELAY)] us
3) Recieve [(32+64+REPLY_DELAY)/8] bytes
*/

void TMC2209_CMD::write(HardwareSerial *hserial, uint8_t address, uint8_t reg, uint32_t data)
{
    if (hserial == nullptr)
        return;

    uint64_t datagram = writeDatagram(address, reg, data);

    hserial->flush(); // Wait communication complete

    for (uint8_t i = 0; i < 8; ++i)
        hserial->write((uint8_t)(datagram >> (i * 8U))); // Send message
}

void TMC2209_CMD::request(HardwareSerial *hserial, uint8_t address, uint8_t reg)
{
    if (hserial == nullptr)
        return;

    uint32_t datagram = requestDatagram(address, reg);

    hserial->flush(); // Wait communication complete
    for (uint8_t i = 0; i < 4; ++i)
        hserial->write((uint8_t)(datagram >> (i * 8U))); // Send message
}

uint64_t TMC2209_CMD::respond(HardwareSerial *hserial)
{
    if (hserial == nullptr)
        return UINT64_MAX;

    uint64_t reply = 0;
    uint16_t reply_delay = 250;

    hserial->flush(); // Wait communication complete

    while (hserial->available() > 0)
        hserial->read(); // Clean rx buffer

    delayMicroseconds(16); // 8bit delay

    while (hserial->available() < 8 and reply_delay)
    { // Wait reply
        delayMicroseconds(1);
        reply_delay--;
    }
    if (reply_delay == 0)
        return UINT64_MAX;

    for (uint8_t i = 0; i < 8U; ++i)
        reply |= (uint64_t)(hserial->read()) << (i * 8U);

    return reply;
}

uint32_t TMC2209_CMD::read(HardwareSerial *hserial, uint8_t address, uint8_t reg)
{
    request(hserial, address, reg);
    uint64_t datagram = respond(hserial);
    return respondData(datagram);
}

void TMC2209_CMD::writeGCONF(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->GCONF.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_GCONF, config->GCONF.UINT32);
}

void TMC2209_CMD::writeIHOLD_IRUN(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->IHOLD_IRUN.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_IHOLD_IRUN, config->IHOLD_IRUN.UINT32);
}

void TMC2209_CMD::writeCHOPCONF(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->CHOPCONF.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_CHOPCONF, config->CHOPCONF.UINT32);
}

void TMC2209_CMD::writePWMCONF(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->PWMCONF.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_PWMCONF, config->PWMCONF.UINT32);
}

void TMC2209_CMD::writeCOOLCONF(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->COOLCONF.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_COOLCONF, config->COOLCONF.UINT32);
}

void TMC2209_CMD::writeTCOOLTHRS(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->TCOOLTHRS.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_TCOOLTHRS, config->TCOOLTHRS.UINT32);
}

void TMC2209_CMD::writeTPWMTHRS(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->TPWMTHRS.UINT32 = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_TPWMTHRS, config->TPWMTHRS.UINT32);
}

void TMC2209_CMD::writeSGTHRS(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->SGTHRS = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_SGTHRS, config->SGTHRS);
}

void TMC2209_CMD::writeTPOWERDOWN(TMC2209_UNIT *config, uint32_t value)
{
    if (value != UINT32_MAX)
        config->TPOWERDOWN = value;
    write(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_TPOWERDOWN, config->TPOWERDOWN);
}

uint32_t TMC2209_CMD::readIOIN(TMC2209_UNIT *config)
{
    uint32_t data = read(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_IOIN);
    if (data != UINT32_MAX)
        config->IOIN.UINT32 = data;
    return data;
}

uint32_t TMC2209_CMD::readSG_RESULT(TMC2209_UNIT *config)
{
    uint32_t data = read(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_SG_RESULT);
    if (data != UINT32_MAX)
        config->SG_RESULT = data;
    return data;
}

uint32_t TMC2209_CMD::readIFCNT(TMC2209_UNIT *config)
{
    uint32_t data = read(config->HSERIAL, config->ADDRESS, TMC2209_REG_ADDRESS::ADDRESS_IFCNT);
    if (data != UINT32_MAX)
        config->IFCNT = data;
    return data;
}

// void TMC2209CMD::makeStep(TMC2209_UNIT *config, bool direction)
// {
//     if (config->STEP_PIN and config->DIR_PIN)
//     {
//         digitalWrite(config->STEP_PIN, direction);
//         digitalWrite(config->STEP_PIN, LOW);
//         if (direction)
//             config->POSITION++;
//         else
//             config->POSITION--;
//     }
// }