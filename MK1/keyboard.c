#include "keyboard.h"
#include "tm1637.h"

static int firstOperand = -1;
static int secondOperand = -1;
static char operation = '\0';

static void setColumnHigh(uint8_t column) {
  switch (column) {
    case 0: GPIOB->BSRR = (1U << 10); break; // C1 -> PB10
    case 1: GPIOB->BSRR = (1U << 4);  break; // C2 -> PB4
    case 2: GPIOB->BSRR = (1U << 5);  break; // C3 -> PB5
    case 3: GPIOA->BSRR = (1U << 15); break; // C4 -> PA15
  }
}

static void setColumnLow(uint8_t column) {
  switch (column) {
    case 0: GPIOB->BRR = (1U << 10); break; // C1 -> PB10
    case 1: GPIOB->BRR = (1U << 4);  break; // C2 -> PB4
    case 2: GPIOB->BRR = (1U << 5);  break; // C3 -> PB5
    case 3: GPIOA->BRR = (1U << 15); break; // C4 -> PA15
  }
}

static int rowIsLow(uint8_t row) {
  switch (row) {
    case 0: return (GPIOB->IDR & (1U << 7)) == 0;  // R1 -> PB7
    case 1: return (GPIOB->IDR & (1U << 6)) == 0;  // R2 -> PB6
    case 2: return (GPIOA->IDR & (1U << 10)) == 0; // R3 -> PA10
    case 3: return (GPIOB->IDR & (1U << 3)) == 0;  // R4 -> PB3
    default: return 0;
  }
}

static int isOperation(char key) {
  return key == '+' || key == '-' || key == '*' || key == '/';
}

static void clearCalculator(void) {
  firstOperand = -1;
  secondOperand = -1;
  operation = '\0';
  tm1637_display_number(0);
  printf("Clear\n");
}

static int calculate(void) {
  int result = 0;

  switch (operation) {
    case '+':
      result = firstOperand + secondOperand;
      break;

    case '-':
      result = firstOperand - secondOperand;
      break;

    case '*':
      result = firstOperand * secondOperand;
      break;

    case '/':
      if (secondOperand == 0) {
        printf("Division by zero\n");
        return 0;
      }
      result = firstOperand / secondOperand;
      break;

    default:
      return 0;
  }

  printf("%d %c %d = %d\n", firstOperand, operation, secondOperand, result);
  tm1637_display_number(result);
  return 1;
}

void initKeyboard(void) {
  // По методичке поочередно активируем столбцы и считываем строки.
  // Столбцы C1-C4: PB10, PB4, PB5, PA15 — выходы с открытым стоком.
  GPIOB->MODER = (GPIOB->MODER & ~(3U << (10 * 2))) | (1U << (10 * 2));
  GPIOB->MODER = (GPIOB->MODER & ~(3U << (4 * 2)))  | (1U << (4 * 2));
  GPIOB->MODER = (GPIOB->MODER & ~(3U << (5 * 2)))  | (1U << (5 * 2));
  GPIOA->MODER = (GPIOA->MODER & ~(3U << (15 * 2))) | (1U << (15 * 2));

  GPIOB->OTYPER |= (1U << 10) | (1U << 4) | (1U << 5);
  GPIOA->OTYPER |= (1U << 15);

  // Строки R1-R4: PB7, PB6, PA10, PB3 — входы с подтяжкой к питанию.
  GPIOB->MODER &= ~((3U << (7 * 2)) | (3U << (6 * 2)) | (3U << (3 * 2)));
  GPIOA->MODER &= ~(3U << (10 * 2));

  GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (7 * 2))) | (1U << (7 * 2));
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (6 * 2))) | (1U << (6 * 2));
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (3 * 2))) | (1U << (3 * 2));
  GPIOA->PUPDR = (GPIOA->PUPDR & ~(3U << (10 * 2))) | (1U << (10 * 2));

  // Неактивное состояние всех столбцов — высокий уровень.
  GPIOB->BSRR = (1U << 10) | (1U << 4) | (1U << 5);
  GPIOA->BSRR = (1U << 15);

  lastKey = '\0';
  lastScanTime = 0;
}

char readKey(void) {
  const char keymap[4][4] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '*'},
    {'C', '0', '=', '/'}
  };

  for (uint8_t column = 0; column < 4; column++) {
    setColumnLow(column);

    // Небольшая задержка для стабилизации сигнала.
    for (volatile int d = 0; d < 100; d++);

    for (uint8_t row = 0; row < 4; row++) {
      if (rowIsLow(row)) {
        setColumnHigh(column);
        return keymap[row][column];
      }
    }

    setColumnHigh(column);
  }

  return '\0';
}

void scanKeyboard(void) {
  // Проверяем клавиатуру раз в 100 мс.
  if ((tickCount - lastScanTime) < 100) {
    return;
  }

  lastScanTime = tickCount;
  char currentKey = readKey();

  if (currentKey == '\0') {
    lastKey = '\0';
    return;
  }

  // Пока клавиша удерживается, обрабатываем ее только один раз.
  if (currentKey == lastKey) {
    return;
  }

  lastKey = currentKey;
  printf("Pressed: %c\n", currentKey);

  if (currentKey == 'C') {
    clearCalculator();
    return;
  }

  if (currentKey >= '0' && currentKey <= '9') {
    if (operation == '\0') {
      firstOperand = currentKey - '0';
    } else {
      secondOperand = currentKey - '0';
    }
    return;
  }

  if (isOperation(currentKey)) {
    if (firstOperand >= 0) {
      operation = currentKey;
      secondOperand = -1;
    }
    return;
  }

  if (currentKey == '=') {
    // Если введено только одно число, просто выводим его.
    // Например: 7 = -> 7
    if (firstOperand >= 0 && operation == '\0') {
      tm1637_display_number(firstOperand);
      printf("%d = %d\n", firstOperand, firstOperand);

      firstOperand = -1;
      secondOperand = -1;
      operation = '\0';
      return;
    }

    if (firstOperand >= 0 && secondOperand >= 0 && operation != '\0') {
      if (calculate()) {
        // После успешного вычисления ждем новое выражение.
        firstOperand = -1;
        secondOperand = -1;
        operation = '\0';
      }
    }
  }
}
