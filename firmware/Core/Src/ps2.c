// ============================================
// PS/2 Touchpad Driver
// ============================================
#include "ps2.h"
#include "main.h"

// ============================================================
// ==== PS/2: Отправка команды ====
// ============================================================
uint8_t ps2_send_cmd(uint8_t cmd)
{
    uint8_t response = 0;
    uint32_t timeout = 0;
    uint8_t parity = 1;
    uint8_t parity_bit = 0;

    // 1. Bring the Clock line low for at least 100 microseconds.
    HAL_GPIO_WritePin(PS2_PORT, PS2_CLK_PIN, GPIO_PIN_RESET);
    // delay_us(100);

    // 2. Bring the Data line low.
    HAL_GPIO_WritePin(PS2_PORT, PS2_DATA_PIN, GPIO_PIN_RESET);
    // delay_us(10);

    // 3. Release the Clock line.
    HAL_GPIO_WritePin(PS2_PORT, PS2_CLK_PIN, GPIO_PIN_SET);

    // 4. Wait for the device to bring the Clock line low.
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    for (int i = 0; i < 8; i++) {
        // Устанавливаем бит данных
        uint8_t bit = (cmd >> i) & 1;

        // 5. Set/reset the Data line to send the first data bit
        HAL_GPIO_WritePin(PS2_PORT, PS2_DATA_PIN, bit);
        delay_us(10);

        // 6. Wait for the device to bring Clock high.
        timeout = 100000;
        while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
            timeout--;
        if (timeout == 0)
            return 0xFF;

        // 7. Wait for the device to bring Clock low.
        timeout = 100000;
        while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
            timeout--;
        if (timeout == 0)
            return 0xFF;

        // Вычисляем бит четности (odd parity)
        parity ^= bit;
    }

    // Отправляем бит четности

    // 5. Set/reset the Data line to send the first data bit
    HAL_GPIO_WritePin(PS2_PORT, PS2_DATA_PIN, parity);
    delay_us(10);

    // 6. Wait for the device to bring Clock high.
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 7. Wait for the device to bring Clock low.
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 9. Release the Data line.
    HAL_GPIO_WritePin(PS2_PORT, PS2_DATA_PIN, GPIO_PIN_SET);

    // 10. Wait for the device to bring Data low.
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 11. Wait for the device to bring Clock  low.
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 12. Wait for the device to release Data and Clock
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    parity = 1;

    // 6. Ждём start bit  от устройства (DATA = 0, CLK = 0)
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 7. Читаем ответ от устройства (ACK = 0xFA)
    for (int i = 0; i < 8; i++) {
        timeout = 100000;
        while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
            timeout--;
        if (timeout == 0)
            return 0xFF;

        if (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != 0) {
            response |= (1 << i);
            parity_bit = 1;
        } else
            parity_bit = 0;
        timeout = 100000;
        while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
            timeout--;
        if (timeout == 0)
            return 0xFF;
        // Вычисляем бит четности (odd parity)
        parity ^= parity_bit;
    }
    // 8. Ждём бит четности от устройства
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;
    // parity_bit = response;
    if (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != parity) // Проверка на четность
        return 0xFF;
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 9. Ждём стоп-бит от устройства
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;
    // response = response & 0x03;
    return response; // Должен быть 0xFA (ACK)
}

// ============================================================
// ==== PS/2: Чтение байта от устройства ====
// ============================================================
uint8_t ps2_read_byte(void)
{
    uint8_t data = 0;
    uint32_t timeout = 0;
    uint8_t parity = 1;
    uint8_t parity_bit = 0;

    // 1. Ждём start bit  от устройства (DATA = 0, CLK = 0)
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 2. Читаем ответ от устройства
    for (int i = 0; i < 8; i++) {
        timeout = 100000;
        while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
            timeout--;
        if (timeout == 0)
            return 0xFF;

        if (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != 0) {
            data |= (1 << i);
            parity_bit = 1;
        } else
            parity_bit = 0;

        timeout = 100000;
        while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
            timeout--;
        if (timeout == 0)
            return 0xFF;
        // Вычисляем бит четности (odd parity)
        parity ^= parity_bit;
    }
    // if (data == 0xFF) data = 0xFE; // Т.к. 0xFF - это ошибка, то убавляем на 1

    // 8. Ждём бит четности от устройства
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;
    // parity_bit = data;
    if (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) != parity) // Проверка на четность
        return 0xFF;
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) != 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    // 9. Ждём стоп-бит от устройства
    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_DATA_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;

    timeout = 100000;
    while (HAL_GPIO_ReadPin(PS2_PORT, PS2_CLK_PIN) == 0 && timeout)
        timeout--;
    if (timeout == 0)
        return 0xFF;
    // response = response & 0x03;
    return data; // Принятый байт от 0 до 0xFE
}

// ============================================================
// ==== PS/2: Включение потока данных ====
// ============================================================
void ps2_enable_data_report(void)
{
    // Команда 0xF4 = Enable Data Reporting
    uint8_t response = ps2_send_cmd(0xF4);
    HAL_Delay(10);

    if (response == 0xFA) {
        // Устройство включено
    }
}

// ============================================================
// ==== PS/2: Выключение потока данных ====
// ============================================================
void ps2_disable_data_report(void)
{
    // Команда 0xF5 = Disable Data Reporting
    uint8_t response = ps2_send_cmd(0xF5);
    HAL_Delay(10);

    if (response == 0xFA) {
        // Устройство выключено
    }
}

// ============================================================
// ==== PS/2: Сброс устройства ====
// ============================================================
uint8_t ps2_reset_device(void)
{
    // Команда 0xFF = Reset
    // uint8_t response = ps2_send_cmd(0xFF);
    uint8_t response = ps2_send_cmd(0xFF);
    // HAL_Delay(2);

    if (response == 0xFA) {
        // Устройство сброшено
        // HAL_GPIO_WritePin (GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
    return response;
}

uint8_t ps2_request_data()
{
    uint8_t response = ps2_send_cmd(0xEB);
    // HAL_Delay(2);

    if (response == 0xFA) {
        // Устройство сброшено
        // HAL_GPIO_WritePin (GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
    return response;
}

// Инициализация тачпада
void tp_init(void)
{
    uint8_t resp = 0;
    // Сброс тачпада
    resp = ps2_reset_device();
    // ps2_enable_data_report();
    if (resp == 0xFA) {
        // Тачпад ответил на сброс
        // HAL_GPIO_WritePin (GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
    HAL_Delay(1);
    resp = ps2_send_cmd(0xF0);
    if (resp == 0xFA) {
        // Тачпад ответил на сброс
        // HAL_GPIO_WritePin (GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
    HAL_Delay(1);
}
