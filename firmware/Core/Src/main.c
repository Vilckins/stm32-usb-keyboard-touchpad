/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Keyboard.h"
#include "ps2.h"
#include "usb_device.h"
#include "usbd_hid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
TIM_HandleTypeDef htim2;
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t empty_sent = 0;
uint8_t current_keys[6] = { 0 };
uint8_t last_keys[6] = { 0 };
uint8_t report[9] = { 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */
    HAL_Delay(1000);

    tp_init(); // Инициализация тачпада

    while (1) {
        // ===== СКАНИРУЕМ МАТРИЦУ =====
        uint8_t count = keyboard_scan(current_keys, 6);
        uint8_t need_send = 0;

        // Где-то в keyboard_scan() или после него:
        if (fn_pressed) {
            // Проверяем, нажаты ли F5 или F6
            for (int i = 0; i < count; i++) {
                if (current_keys[i] == KEY_F5) {

                    // Volume Down
                    USBD_HID_SendMedia(&hUsbDeviceFS, 0x02);
                    HAL_Delay(100);
                    USBD_HID_SendMedia(&hUsbDeviceFS, 0x00); // Отпустить
                    current_keys[i] = 0; // Убираем F5 из обычного отчёта
                } else if (current_keys[i] == KEY_F6) {
                    // Volume Up
                    USBD_HID_SendMedia(&hUsbDeviceFS, 0x01);
                    HAL_Delay(100);
                    USBD_HID_SendMedia(&hUsbDeviceFS, 0x00); // Отпустить
                    current_keys[i] = 0; // Убираем F6 из обычного отчёта
                } else if (current_keys[i] == KEY_F2) {
                    // Volume Mute
                    USBD_HID_SendMedia(&hUsbDeviceFS, 0x04);
                    HAL_Delay(100);
                    USBD_HID_SendMedia(&hUsbDeviceFS, 0x00); // Отпустить
                    current_keys[i] = 0; // Убираем F6 из обычного отчёта
                }
            }
        }

        // ===== 1. ПРОВЕРЯЕМ МОДИФИКАТОРЫ (ОТДЕЛЬНО!) =====
        if (current_modifiers != last_modifiers) {
            need_send = 1;
            last_modifiers = current_modifiers;
        }

        // ===== 2. ПРОВЕРЯЕМ КЛАВИШИ =====
        for (int i = 0; i < 6; i++) {
            if (current_keys[i] != last_keys[i]) {
                need_send = 1;
                last_keys[i] = current_keys[i];
            }
        }

        // ===== 3. ОТПРАВЛЯЕМ, ЕСЛИ ЕСТЬ ИЗМЕНЕНИЯ =====
        if (need_send) {
            keyboard_build_report(current_keys, count, report);
            USBD_HID_SendKeyboard(&hUsbDeviceFS, report, 9);
        }
        // ===== 4. ЕСЛИ ВСЁ ОТПУЩЕНО — ОДИН ПУСТОЙ ОТЧЁТ =====
        else if (count == 0 && current_modifiers == 0) {
            static uint8_t empty_sent = 0;
            if (!empty_sent) {
                uint8_t empty[9] = { 0 };
                empty[0] = 0x01;
                USBD_HID_SendKeyboard(&hUsbDeviceFS, empty, 9);
                empty_sent = 1;
            }
        } else {
            empty_sent = 0;
        }

        HAL_Delay(10);

        tp_process();
        HAL_Delay(10);
    }
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 48;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 65535;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_Base_Start(&htim2); // запуск таймера 2
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */

static void MX_GPIO_Init(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE(); // Включаем тактирование AFIO
    __HAL_AFIO_REMAP_SWJ_NOJTAG(); // Отключаем JTAG, но оставляем SWD

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // ============================================
    // СТРОКИ — OUTPUT (Push-Pull) (Open Drain)
    // ============================================
    // PA1, PA2, PA4, PA5, PA6, PA7, PA10
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PB3, PB4, PB5, PB10, PB11, PB12, PB13, PB14, PB15
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // ============================================
    // СТОЛБЦЫ — INPUT с подтяжкой (Pull-Up)
    // ============================================
    // PA0, PA3, PA8, PA9
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PB0, PB1, PB8, PB9
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // ============================================
    // LED (PC13)
    // ============================================
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

    // ============================================
    // Настройка для PB6 (DATA) PB7 (CLK)
    // ============================================

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Режим: выход (Open-Drain)
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Без внутренней подтяжки (внешняя у вас уже есть)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Устанавливаем высокий уровень (Idle)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
}

/* USER CODE BEGIN 4 */

void delay_us(uint16_t n)
{
    TIM2->CNT = 0;
    while (TIM2->CNT < n) { } // пауза n мкс
}

// Главная функция чтения и отправки данных
void tp_process(void)
{
    static uint8_t last_buttons = 0;
    static int16_t accumulated_y = 0;
    static uint32_t last_movement_time = 0;
    uint8_t buttons = 0;
    uint8_t resp = 0;
    // Запрос данных
    resp = ps2_request_data();
    if (resp == 0xFA) {
        // Тачпад ответил на запрос успешно
    }

    uint8_t data[3];
    data[0] = ps2_read_byte();
    data[1] = ps2_read_byte();
    data[2] = ps2_read_byte();

    buttons = data[0] & 0x03;

    int8_t dx = (data[0] & 0x10) ? (data[1] - 256) : data[1];
    int8_t dy = (data[0] & 0x20) ? (data[2] - 256) : data[2];
    dx = dx * 2;
    dy = dy * 2;
    dy = -dy;

    // ===== ЕСЛИ FN НАЖАТА — СКРОЛЛ =====
    if (fn_pressed) {
        // Накапливаем Y для скролла
        if (dy != 0) {
            accumulated_y += dy;
            last_movement_time = HAL_GetTick();
        }

        // Сбрасываем накопление по таймауту
        if (HAL_GetTick() - last_movement_time > 150) {
            accumulated_y = 0;
        }

        // Формируем wheel из накопленного Y
        int8_t wheel = 0;
        if (accumulated_y > 10) {
            wheel = -1; // Скролл вниз
            accumulated_y = 0;
        } else if (accumulated_y < -10) {
            wheel = 1; // Скролл вверх
            accumulated_y = 0;
        }
        // Отправляем только скролл, без движения курсора
        if (wheel || (buttons != last_buttons)) {
            USBD_HID_SendMouse(&hUsbDeviceFS, 0, 0, wheel, buttons);
            last_buttons = buttons;
        }
    }
    // ===== ОБЫЧНЫЙ РЕЖИМ =====
    else {
        // ===== ОТПРАВЛЯЕМ ТОЛЬКО ПРИ ИЗМЕНЕНИИ =====
        if (dx || dy || (buttons != last_buttons)) {
            USBD_HID_SendMouse(&hUsbDeviceFS, dx, dy, 0, buttons);
            last_buttons = buttons;
        }
    }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
