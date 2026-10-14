#include "tm1637.h"

static const uint8_t digit_codes[] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F  // 9
};

#define SEGMENT_MINUS 0x40

void delay_us(uint32_t us) {
  // Эмпирическая задержка для 48 MHz, как в исходной заготовке.
  volatile uint32_t cycles = us * 24U;
  while (cycles-- > 0) {
    __asm__ volatile ("nop");
  }
}

void tm1637_init(void) {
  // PA6 и PA7 — выходы с открытым стоком.
  GPIOA->MODER = (GPIOA->MODER & ~(0xFU << (TM1637_CLK_PIN * 2))) |
                 (0x5U << (TM1637_CLK_PIN * 2));
  GPIOA->OTYPER |= (1U << TM1637_CLK_PIN) | (1U << TM1637_DIO_PIN);
  GPIOA->OSPEEDR |= (0x3U << (TM1637_CLK_PIN * 2)) |
                    (0x3U << (TM1637_DIO_PIN * 2));

  GPIOA->BSRR = (1U << TM1637_CLK_PIN) | (1U << TM1637_DIO_PIN);

  tm1637_start();
  tm1637_write_byte(TM1637_CMD1);
  tm1637_stop();

  tm1637_start();
  tm1637_write_byte(TM1637_CMD3);
  tm1637_stop();

  tm1637_clear();
  tm1637_display_number(0);
}

void tm1637_start(void) {
  GPIOA->BSRR = (1U << TM1637_DIO_PIN) | (1U << TM1637_CLK_PIN);
  delay_us(2);
  GPIOA->BRR = (1U << TM1637_DIO_PIN);
  delay_us(2);
  GPIOA->BRR = (1U << TM1637_CLK_PIN);
  delay_us(2);
}

void tm1637_stop(void) {
  GPIOA->BRR = (1U << TM1637_CLK_PIN);
  delay_us(2);
  GPIOA->BRR = (1U << TM1637_DIO_PIN);
  delay_us(2);
  GPIOA->BSRR = (1U << TM1637_CLK_PIN);
  delay_us(2);
  GPIOA->BSRR = (1U << TM1637_DIO_PIN);
  delay_us(2);
}

void tm1637_write_byte(uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    GPIOA->BRR = (1U << TM1637_CLK_PIN);
    delay_us(2);

    if (byte & 0x01U) {
      GPIOA->BSRR = (1U << TM1637_DIO_PIN);
    } else {
      GPIOA->BRR = (1U << TM1637_DIO_PIN);
    }

    delay_us(2);
    GPIOA->BSRR = (1U << TM1637_CLK_PIN);
    delay_us(2);

    byte >>= 1;
  }

  // Такт ACK. Проверку ACK оставляем пропущенной, как в исходном коде.
  GPIOA->BRR = (1U << TM1637_CLK_PIN);
  GPIOA->BSRR = (1U << TM1637_DIO_PIN);
  delay_us(2);
  GPIOA->BSRR = (1U << TM1637_CLK_PIN);
  delay_us(2);
  GPIOA->BRR = (1U << TM1637_CLK_PIN);
  delay_us(2);
}

void tm1637_display_digit(uint8_t digit, uint8_t data) {
  tm1637_start();
  tm1637_write_byte(TM1637_CMD2 | digit);
  tm1637_write_byte(data);
  tm1637_stop();

  tm1637_start();
  tm1637_write_byte(TM1637_CMD3);
  tm1637_stop();
}

void tm1637_display_number(int number) {
  uint8_t segments[4] = {0, 0, 0, 0};
  uint32_t value;
  int position = 3;
  int negative = number < 0;

  if (negative) {
    value = (uint32_t)(-(int64_t)number);
  } else {
    value = (uint32_t)number;
  }

  // Минимум одна цифра должна отображаться даже для нуля.
  do {
    segments[position] = digit_codes[value % 10U];
    value /= 10U;
    position--;
  } while (value > 0 && position >= 0);

  if (negative && position >= 0) {
    segments[position] = SEGMENT_MINUS;
  }

  tm1637_start();
  tm1637_write_byte(TM1637_CMD1);
  tm1637_stop();

  tm1637_start();
  tm1637_write_byte(TM1637_CMD2);

  for (uint8_t i = 0; i < 4; i++) {
    tm1637_write_byte(segments[i]);
  }

  tm1637_stop();

  tm1637_start();
  tm1637_write_byte(TM1637_CMD3);
  tm1637_stop();
}

void tm1637_clear(void) {
  tm1637_start();
  tm1637_write_byte(TM1637_CMD1);
  tm1637_stop();

  tm1637_start();
  tm1637_write_byte(TM1637_CMD2);

  for (uint8_t i = 0; i < 4; i++) {
    tm1637_write_byte(0x00);
  }

  tm1637_stop();
}
