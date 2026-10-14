#include "keypad.h"
#include "i2c.h"

#define KEYPAD_ADDR              0xE2
#define PCA9538_REG_INPUT        0x00
#define PCA9538_REG_OUTPUT       0x01
#define PCA9538_REG_POLARITY     0x02
#define PCA9538_REG_CONFIG       0x03

#define KEYPAD_DEBOUNCE_MS       30U
#define KEYPAD_SCAN_PERIOD_MS    10U

static const char keymap[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

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

    /*
     * В покое все 8 линий - входы.
     * Во время сканирования одна строка P0..P3 временно становится выходом 0.
     */
    value = 0xFF;
    if (pca_write(PCA9538_REG_CONFIG, value) != HAL_OK) {
        return 0;
    }

    return 1;
}

/*
 * Сканирование по методичке:
 * P0..P3 - строки;
 * P4..P6 - столбцы;
 * для каждой строки выдаём 0 только на неё,
 * остальные строки оставляем входами, затем читаем столбцы.
 */
static char keypad_read_raw(void)
{
    char detected = 0;
    uint8_t pressed_count = 0;

    for (uint8_t row = 0; row < 4; row++) {
        uint8_t output = (uint8_t)(0xFFU & ~(1U << row));
        uint8_t config = (uint8_t)(0xFFU & ~(1U << row));
        uint8_t input = 0xFF;

        /* Заранее задаём выбранной строке уровень 0. */
        if (pca_write(PCA9538_REG_OUTPUT, output) != HAL_OK) {
            (void)pca_write(PCA9538_REG_CONFIG, 0xFF);
            return 0;
        }

        /* Только выбранная строка - выход, все остальные линии - входы. */
        if (pca_write(PCA9538_REG_CONFIG, config) != HAL_OK) {
            (void)pca_write(PCA9538_REG_CONFIG, 0xFF);
            return 0;
        }

        /*
         * Даём уровню на матрице установиться.
         * Для пользователя эта задержка незаметна: полный скан занимает ~4 мс.
         */
        HAL_Delay(1);

        if (pca_read(PCA9538_REG_INPUT, &input) != HAL_OK) {
            (void)pca_write(PCA9538_REG_CONFIG, 0xFF);
            return 0;
        }

        for (uint8_t col = 0; col < 3; col++) {
            uint8_t bit = (uint8_t)(4U + col);

            if ((input & (1U << bit)) == 0U) {
                detected = keymap[row][col];
                pressed_count++;
            }
        }
    }

    /* После скана освобождаем все линии. */
    (void)pca_write(PCA9538_REG_CONFIG, 0xFF);

    return (pressed_count == 1U) ? detected : 0;
}

char Keypad_GetKey(void)
{
    static uint32_t last_scan_ms = 0;
    static uint32_t changed_at_ms = 0;
    static char last_raw = 0;
    static uint8_t reported = 0;

    uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - last_scan_ms) < KEYPAD_SCAN_PERIOD_MS) {
        return 0;
    }
    last_scan_ms = now;

    char raw = keypad_read_raw();

    if (raw != last_raw) {
        last_raw = raw;
        changed_at_ms = now;
        return 0;
    }

    if ((uint32_t)(now - changed_at_ms) < KEYPAD_DEBOUNCE_MS) {
        return 0;
    }

    if (raw == 0) {
        reported = 0;
        return 0;
    }

    if (!reported) {
        reported = 1;
        return raw;
    }

    return 0;
}
