/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "platform.h"

//Use the timeout from the config, the default timeout is -1
#if CONFIG_VL53L5CX_I2C_TIMEOUT == false
#define VL53L5CX_I2C_TIMEOUT (-1)
#else
#define VL53L5CX_I2C_TIMEOUT CONFIG_VL53L5CX_I2C_TIMEOUT_VALUE
#endif

//Define the reset scheme
#ifdef CONFIG_VL53L5CX_RESET_PIN_HIGH
#define VL53L5CX_RESET_LEVEL 1
#elif CONFIG_VL53L5CX_RESET_PIN_LOW
#define VL53L5CX_RESET_LEVEL 0
#endif

uint8_t VL53L5CX_WrMulti(VL53L5CX_Platform *p_platform, uint16_t RegisterAddress, uint8_t *p_values, uint32_t size)
{
    //uint8_t buffer[size + 2];
    uint8_t *buffer = (uint8_t*)malloc(size + 2); // allocate in heap to avoid stack overflow
    if (!buffer) 
    {
        return 1; // status not ok, failed to allocate
    }

    // Register address (big endian)
    buffer[0] = RegisterAddress >> 8;
    buffer[1] = RegisterAddress & 0xFF;

    memcpy(&buffer[2], p_values, size);

    esp_err_t ret = i2c_master_write_to_device(
        p_platform->port,
        p_platform->address,
        buffer,
        size + 2,
        pdMS_TO_TICKS(VL53L5CX_I2C_TIMEOUT)
    );

    free(buffer); // added

    return (ret == ESP_OK) ? 0 : 1;
}

uint8_t VL53L5CX_WrByte(VL53L5CX_Platform *p_platform, uint16_t RegisterAddress, uint8_t value) {

    //Write a single byte
    return VL53L5CX_WrMulti(p_platform, RegisterAddress, &value, 1);
}

uint8_t VL53L5CX_RdMulti(VL53L5CX_Platform *p_platform, uint16_t RegisterAddress, uint8_t *p_values, uint32_t size)
{
    uint8_t reg[2];

    reg[0] = RegisterAddress >> 8;
    reg[1] = RegisterAddress & 0xFF;

    esp_err_t ret = i2c_master_write_read_device(
        p_platform->port,
        p_platform->address,
        reg,
        2,
        p_values,
        size,
        pdMS_TO_TICKS(VL53L5CX_I2C_TIMEOUT)
    );

    return (ret == ESP_OK) ? 0 : 1;
}

uint8_t VL53L5CX_RdByte(VL53L5CX_Platform *p_platform, uint16_t RegisterAddress, uint8_t *p_value) {

    //Read a single byte
    return VL53L5CX_RdMulti(p_platform, RegisterAddress, p_value, 1);
}

uint8_t VL53L5CX_Reset_Sensor(VL53L5CX_Platform* p_platform)
{
    gpio_set_direction(p_platform->reset_gpio, GPIO_MODE_OUTPUT);

    gpio_set_level(p_platform->reset_gpio, VL53L5CX_RESET_LEVEL);
    VL53L5CX_WaitMs(p_platform, 100);

    gpio_set_level(p_platform->reset_gpio, !VL53L5CX_RESET_LEVEL);
    VL53L5CX_WaitMs(p_platform, 100);

    return ESP_OK;
}

void VL53L5CX_SwapBuffer(uint8_t *buffer, uint16_t size) {
    uint32_t i;
    uint8_t tmp[4] = {0};

    for (i = 0; i < size; i = i + 4) {

        tmp[0] = buffer[i + 3];
        tmp[1] = buffer[i + 2];
        tmp[2] = buffer[i + 1];
        tmp[3] = buffer[i];

        memcpy(&(buffer[i]), tmp, 4);
    }
}

uint8_t VL53L5CX_WaitMs(VL53L5CX_Platform *p_platform, uint32_t TimeMs) {
    vTaskDelay(pdMS_TO_TICKS(TimeMs));

    return ESP_OK;
}
