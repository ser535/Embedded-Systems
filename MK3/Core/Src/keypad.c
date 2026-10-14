#include "keypad.h"
#include "i2c.h"

#define KEYPAD_ADDR              0xE2
#define PCA9538_REG_INPUT        0x00
#define PCA9538_REG_OUTPUT       0x01
#define PCA9538_REG_POLARITY     0x02
#define PCA9538_REG_CONFIG       0x03

#define KEYPAD_DEBOUNCE_MS       30U
#define KEYPAD_SCAN_PERIOD_MS    10U

static HAL_StatusTypeDef pca_write(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c1,
                             KEYPAD_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &value,
                             1,
                             50);
}

static HAL_StatusTypeDef pca_read(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c1,
                            KEYPAD_ADDR,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            value,
                            1,
                            50);
}

uint8_t Keypad_Init(void)
{
    uint8_t value;

    if (HAL_I2C_IsDeviceReady(&hi2c1, KEYPAD_ADDR, 3, 100) != HAL_OK) {
        return 0;
    }

    /* Без инверсии входов */
    value = 0x00;
    if (pca_write(PCA9538_REG_POLARITY, value) != HAL_OK) {
        return 0;
    }


    value = 0xFF;
    if (pca_write(PCA9538_REG_CONFIG, value) != HAL_OK) {
        return 0;
    }

    return 1;
}


uint8_t Keypad_ReadMask(uint16_t *mask)
{
    uint16_t detected = 0;
    *mask = 0;

    for (uint8_t row = 0; row < 4; row++) {
        uint8_t output = (uint8_t)(0xFFU & ~(1U << row));
        uint8_t config = (uint8_t)(0xFFU & ~(1U << row));
        uint8_t input = 0xFF;

        // Заранее задаём выбранной строке уровень 0
        if (pca_write(PCA9538_REG_OUTPUT, output) != HAL_OK) {
            (void)pca_write(PCA9538_REG_CONFIG, 0xFF);
            return 0;
        }

        // Только выбранная строка - выход, все остальные линии - входы
        if (pca_write(PCA9538_REG_CONFIG, config) != HAL_OK) {
            (void)pca_write(PCA9538_REG_CONFIG, 0xFF);
            return 0;
        }


        // Даём уровню на матрице установиться

        HAL_Delay(1);

        if (pca_read(PCA9538_REG_INPUT, &input) != HAL_OK) {
            (void)pca_write(PCA9538_REG_CONFIG, 0xFF);
            return 0;
        }

        for (uint8_t col = 0; col < 3; col++) {
            uint8_t bit = (uint8_t)(4U + col);

            if ((input & (1U << bit)) == 0U) {
                detected |= (uint16_t)(1U << (row * 3U + col));
            }
        }
    }

    // После скана освобождаем все линии
    if (pca_write(PCA9538_REG_CONFIG, 0xFF) != HAL_OK) return 0;
    *mask = detected;
    return 1;
}

