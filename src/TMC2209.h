/**
 * @file TMC2209.h
 * @author Anton Khrustalev, 2023
 * @brief Highly abstracted library for TMC2209.
 */

#pragma once

#ifndef TMC2209_H
#define TMC2209_H
#include "TMC2209_CMD.h"

class TMC2209
{
private:
    static constexpr uint32_t COOLCONF_DEFAULT = 0;
    static constexpr uint8_t SGTHRS_DEFAULT = 0;
    static constexpr uint8_t TPOWERDOWN_DEFAULT = 20;

public:
    TMC2209_CMD *cmd;
    
    TMC2209();
    TMC2209(TMC2209_CMD *CMD);

    /**
     * @brief Устанавливает параметры по умолчанию для драйвера
     * @param config - ссылка на экземпляр настроек драйвера
     */
    void setupDefault(TMC2209_UNIT *config);

    /**
     * @brief Проверяет, доступен ли драйвер, отправляя запрос в регистр IOIN.
     * Если коммуникации нет, будет получен ответ UINT32_MAX
     * @param config - ссылка на экземпляр настроек драйвера
     */
    bool avalible(TMC2209_UNIT *config);
};

#endif