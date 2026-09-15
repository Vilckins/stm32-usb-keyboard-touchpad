#include "Keyboard.h"
#include "main.h" // если нужен доступ к HAL_GPIO

// ============================================
// Keyboard Matrix Definitions
// ============================================

#define ROWS 16
#define COLS 8

uint8_t current_modifiers = 0;
uint8_t last_modifiers = 0;

// ===== ГЛОБАЛЬНЫЙ ФЛАГ FN =====
volatile uint8_t fn_pressed = 0;

// ===== СТРУКТУРА ДЛЯ МАТРИЦЫ =====
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} KeyPin_t;

// ===== СТРОКИ (16) =====
static const KeyPin_t ROW_Pins[ROWS] = {
    { GPIOA, GPIO_PIN_1 },
    { GPIOA, GPIO_PIN_2 },
    { GPIOB, GPIO_PIN_5 },
    { GPIOB, GPIO_PIN_4 },
    { GPIOA, GPIO_PIN_4 },
    { GPIOB, GPIO_PIN_3 },
    { GPIOA, GPIO_PIN_5 },
    { GPIOA, GPIO_PIN_6 },
    { GPIOA, GPIO_PIN_7 },
    { GPIOA, GPIO_PIN_10 },
    { GPIOB, GPIO_PIN_10 },
    { GPIOB, GPIO_PIN_11 },
    { GPIOB, GPIO_PIN_15 },
    { GPIOB, GPIO_PIN_14 },
    { GPIOB, GPIO_PIN_13 },
    { GPIOB, GPIO_PIN_12 },
};

// ===== СТОЛБЦЫ (8) =====
static const KeyPin_t COL_Pins[COLS] = {
    { GPIOB, GPIO_PIN_9 },
    { GPIOA, GPIO_PIN_0 },
    { GPIOB, GPIO_PIN_8 },
    { GPIOA, GPIO_PIN_3 },
    { GPIOB, GPIO_PIN_0 },
    { GPIOB, GPIO_PIN_1 },
    { GPIOA, GPIO_PIN_9 },
    { GPIOA, GPIO_PIN_8 },
};

// ===== ОБЫЧНЫЕ КЛАВИШИ =====
static const uint8_t normal[ROWS][COLS] = {
    { 0, KEY_BACKSPACE, KEY_INSERT, KEY_DELETE, KEY_ENTER, 0, KEY_BACKSLASH, 0 },
    { KEY_PAGEUP, KEY_END, KEY_PAGEDOWN, KEY_HOME, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT },
    //{ 0, KEY_EQUAL, KEY_NUMLOCK, KEY_PRTSCR, KEY_BRACKET_RIGHT, KEY_QUOTE, 0, 0 },
    { 0, KEY_EQUAL, 0, KEY_PRTSCR, KEY_BRACKET_RIGHT, KEY_QUOTE, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { KEY_BRACKET_LEFT, KEY_P, KEY_F12, KEY_MINUS, 0, 0, KEY_SLASH, KEY_MENU },
    { KEY_CAPSLOCK, KEY_TAB, KEY_ESC, KEY_TILDA, KEY_SEMICOLON, KEY_SPACE, 0, 0 },
    { KEY_2, KEY_1, KEY_F1, KEY_F2, KEY_Q, KEY_W, KEY_S, KEY_Z },
    { KEY_4, KEY_3, KEY_F3, KEY_F4, KEY_E, KEY_D, KEY_A, KEY_X },
    { KEY_R, KEY_5, KEY_F5, KEY_F6, KEY_T, KEY_F, KEY_C, KEY_V },
    { KEY_Y, KEY_7, KEY_F7, KEY_6, KEY_G, KEY_H, KEY_K, KEY_N },
    { KEY_U, KEY_8, KEY_F8, KEY_F9, KEY_I, KEY_J, KEY_B, KEY_M },
    { KEY_0, KEY_9, KEY_F10, KEY_F11, KEY_O, KEY_L, KEY_DECIMAL, KEY_COMMA },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

// ===== МОДИФИКАТОРЫ =====
static const uint8_t modifier[ROWS][COLS] = { // <-- ИСПРАВЛЕНО!
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, KEY_ALT_LEFT, 0, 0, KEY_ALT_RIGHT },
    { KEY_WIN, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, KEY_FN, 0, 0, 0, 0 },
    { 0, 0, KEY_CTRL_LEFT, 0, 0, 0, KEY_CTRL_RIGHT, 0 },
    { 0, KEY_SHIFT_LEFT, 0, 0, 0, KEY_SHIFT_RIGHT, 0, 0 }
};

// ===== СОСТОЯНИЯ =====
// static uint8_t current_keys[6] = { 0 };
// static uint8_t last_keys[6] = { 0 };

// ============================================
// Keyboard Driver Functions
// ============================================

// ===== СКАНИРОВАНИЕ МАТРИЦЫ =====
uint8_t keyboard_scan(uint8_t* keys, uint8_t max_keys)
{
    uint8_t count = 0;
    uint8_t modifier_mask = 0;

    // ===== СБРАСЫВАЕМ ФЛАГ FN =====
    fn_pressed = 0;
    // Очищаем массив клавиш
    for (int i = 0; i < max_keys; i++) {
        keys[i] = 0;
    }

    for (int row = 0; row < ROWS; row++) {
        HAL_GPIO_WritePin(ROW_Pins[row].port, ROW_Pins[row].pin, GPIO_PIN_RESET);

        // for (int debounce = 0; debounce < 5; debounce++)
        delay_us(10);

        for (int col = 0; col < COLS; col++) {
            if (HAL_GPIO_ReadPin(COL_Pins[col].port, COL_Pins[col].pin) == GPIO_PIN_RESET) {

                // ===== ПРОВЕРЯЕМ FN =====
                if (modifier[row][col] == KEY_FN) {
                    fn_pressed = 1; // <-- FN НАЖАТА!
                } else if (modifier[row][col] != 0) {
                    modifier_mask |= modifier[row][col];
                }
                // ===== ОБЫЧНЫЕ КЛАВИШИ =====
                else if (normal[row][col] != 0) {
                    if (count < max_keys) {
                        keys[count++] = normal[row][col];
                    }
                }
            }
        }

        HAL_GPIO_WritePin(ROW_Pins[row].port, ROW_Pins[row].pin, GPIO_PIN_SET);
    }

    // ===== СОХРАНЯЕМ МОДИФИКАТОРЫ (ГЛОБАЛЬНО!) =====
    current_modifiers = modifier_mask;

    return count;
}

// ===== ПРОВЕРКА ИЗМЕНЕНИЙ =====
uint8_t keyboard_has_changed(uint8_t* current, uint8_t* previous, uint8_t size)
{
    // Сначала проверяем модификаторы
    if (current_modifiers != last_modifiers) {
        return 1;
    }

    // Потом клавиши
    for (int i = 0; i < size; i++) {
        if (current[i] != previous[i]) {
            return 1;
        }
    }
    return 0;
}

// ===== ФОРМИРОВАНИЕ ОТЧЁТА =====
void keyboard_build_report(uint8_t* keys, uint8_t count, uint8_t* report)
{
    // Очищаем отчёт
    for (int i = 0; i < 9; i++) {
        report[i] = 0;
    }

    report[0] = 0x01; // Report ID
    report[1] = current_modifiers; // ← Модификаторы (даже если нет клавиш!)
    report[2] = 0x00; // Зарезервировано

    // Копируем клавиши
    for (int i = 0; i < 6 && i < count; i++) {
        report[3 + i] = keys[i];
    }
}
