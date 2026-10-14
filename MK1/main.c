#include "main.h"
#include "tm1637.h"
#include "keyboard.h"

volatile uint32_t tickCount = 0;
char lastKey = '\0';
uint32_t lastScanTime = 0;

void osSystickHandler(void) {
  tickCount++;
}

void initGPIO(void) {
  // Включаем тактирование GPIOA и GPIOB
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

  // PA5 оставляем выходом, как в исходной схеме со светодиодом
  GPIOA->MODER = (GPIOA->MODER & ~(3U << 10)) | (1U << 10);
  GPIOA->OTYPER &= ~(1U << 5);
  GPIOA->OSPEEDR |= (1U << 10);
}

void initUSART2(void) {
  // Включаем тактирование USART2
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // PA2 и PA3 в альтернативный режим AF1
  GPIOA->MODER = (GPIOA->MODER & ~(0xFU << 4)) | (0xAU << 4);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFFU << 8)) | (1U << 8) | (1U << 12);

  // 48 MHz / 115200
  USART2->BRR = 417;
  USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void initSysTick(void) {
  SysTick->LOAD = 47999; // 1 мс при 48 MHz
  SysTick->VAL = 0;
  SysTick->CTRL = (1U << 2) | (1U << 1) | (1U << 0);
}

int _write(int file, uint8_t *ptr, int len) {
  (void)file;

  for (int i = 0; i < len; i++) {
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = ptr[i];
  }

  return len;
}

int main(void) {
  initGPIO();
  initUSART2();
  initSysTick();
  initKeyboard();
  tm1637_init();

  printf("Calculator ready\n");

  while (1) {
    scanKeyboard();
  }

  return 0;
}
