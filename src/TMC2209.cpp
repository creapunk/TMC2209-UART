/**
 * @file TMC2209.сpp
 * @author Anton Khrustalev, 2023
 */

#include "TMC2209.h"

TMC2209::TMC2209()
{
    TMC2209_CMD newCmd;
    cmd = &newCmd;
}

TMC2209::TMC2209(TMC2209_CMD *CMD)
{
    cmd = CMD;
}

void TMC2209::setupDefault(TMC2209_UNIT *config)
{
    cmd->writeGCONF(config, TMC2209_REG_DEFAULT::GCONF_DEFAULT.UINT32);
    cmd->writeIHOLD_IRUN(config, TMC2209_REG_DEFAULT::IHOLD_IRUN_DEFAULT.UINT32);
    cmd->writeCHOPCONF(config, TMC2209_REG_DEFAULT::CHOPCONF_DEFAULT.UINT32);
    cmd->writePWMCONF(config, TMC2209_REG_DEFAULT::PWMCONF_DEFAULT.UINT32);
    cmd->writeTPWMTHRS(config, TMC2209_REG_DEFAULT::TPWMTHRS_DEFAULT.UINT32);
    cmd->writeTCOOLTHRS(config, TMC2209_REG_DEFAULT::TCOOLTHRS_DEFAULT.UINT32);
    // if (avalible(config)) {
        
    // }
}

bool TMC2209::avalible(TMC2209_UNIT *config)
{
    if (cmd->readIOIN(config) != UINT32_MAX) return true;
    return false;
}

