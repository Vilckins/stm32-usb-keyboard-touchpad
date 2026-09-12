/**
 ******************************************************************************
 * @file    usbd_hid.c
 * @author  MCD Application Team
 * @brief   This file provides the HID core functions.
 *
 * @verbatim
 *
 *          ===================================================================
 *                                HID Class  Description
 *          ===================================================================
 *           This module manages the HID class V1.11 following the "Device Class Definition
 *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
 *           This driver implements the following aspects of the specification:
 *             - The Boot Interface Subclass
 *             - The Mouse protocol
 *             - Usage Page : Generic Desktop
 *             - Usage : Joystick
 *             - Collection : Application
 *
 * @note     In HS mode and when the DMA is used, all variables and data structures
 *           dealing with the DMA during the transaction process should be 32-bit aligned.
 *
 *
 *  @endverbatim
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                      www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_ctlreq.h"

volatile uint8_t led_state = 0;
// Внешний указатель на светодиод (если нужен)
extern void set_capslock_led(uint8_t state); // Ваша функция в main.c

/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */

/** @defgroup USBD_HID
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_HID_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup USBD_HID_Private_Defines
 * @{
 */
#define LED_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_13 // Например, встроенный LED на Blue Pill
/**
 * @}
 */

/** @defgroup USBD_HID_Private_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup USBD_HID_Private_FunctionPrototypes
 * @{
 */

static uint8_t USBD_HID_Init(USBD_HandleTypeDef* pdev,
    uint8_t cfgidx);

static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef* pdev,
    uint8_t cfgidx);

static uint8_t USBD_HID_Setup(USBD_HandleTypeDef* pdev,
    USBD_SetupReqTypedef* req);

static uint8_t* USBD_HID_GetFSCfgDesc(uint16_t* length);

static uint8_t* USBD_HID_GetHSCfgDesc(uint16_t* length);

static uint8_t* USBD_HID_GetOtherSpeedCfgDesc(uint16_t* length);

static uint8_t* USBD_HID_GetDeviceQualifierDesc(uint16_t* length);

static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef* pdev, uint8_t epnum);

static uint8_t USBD_HID_DataOut(USBD_HandleTypeDef* pdev, uint8_t epnum);
/**
 * @}
 */

/** @defgroup USBD_HID_Private_Variables
 * @{
 */

USBD_ClassTypeDef USBD_HID = {
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Setup,
    NULL, /*EP0_TxSent*/
    NULL, /*EP0_RxReady*/
    USBD_HID_DataIn, /*DataIn*/
    USBD_HID_DataOut, // NULL, /*DataOut*/
    NULL, /*SOF */
    NULL,
    NULL,
    USBD_HID_GetHSCfgDesc,
    USBD_HID_GetFSCfgDesc,
    USBD_HID_GetOtherSpeedCfgDesc,
    USBD_HID_GetDeviceQualifierDesc,
};

/* USB HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgFSDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09, /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    USB_HID_CONFIG_DESC_SIZ,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01, /*bNumInterfaces: 1 interface*/
    0x01, /*bConfigurationValue: Configuration value*/
    0x00, /*iConfiguration: Index of string descriptor describing
the configuration*/
    0xE0, /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32, /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09, /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
    0x00, /*bInterfaceNumber: Number of Interface*/
    0x00, /*bAlternateSetting: Alternate setting*/
    0x02, /*bNumEndpoints*/
    0x03, /*bInterfaceClass: HID*/
    0x00, /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00, /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0, /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09, /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00, /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    HID_MOUSE_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07, /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

    HID_EPIN_ADDR, /*bEndpointAddress: Endpoint Address (IN)*/
    0x03, /*bmAttributes: Interrupt endpoint*/
    HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
    0x00,
    HID_FS_BINTERVAL, /*bInterval: Polling Interval */
    /* 34 */

    // ===== ENDPOINT OUT (0x02) ДОБАВЛЯЕМ! =====
    0x07,
    USB_DESC_TYPE_ENDPOINT,
    HID_EPOUT_ADDR,
    0x03,
    HID_EPOUT_SIZE,
    0x00,
    HID_FS_BINTERVAL,
    /* 41 */
};

/* USB HID device HS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgHSDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09, /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    USB_HID_CONFIG_DESC_SIZ,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01, /*bNumInterfaces: 1 interface*/
    0x01, /*bConfigurationValue: Configuration value*/
    0x00, /*iConfiguration: Index of string descriptor describing
the configuration*/
    0xE0, /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32, /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09, /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
    0x00, /*bInterfaceNumber: Number of Interface*/
    0x00, /*bAlternateSetting: Alternate setting*/
    0x02, /*bNumEndpoints*/
    0x03, /*bInterfaceClass: HID*/
    0x00, /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00, /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0, /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09, /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00, /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    HID_MOUSE_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07, /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

    HID_EPIN_ADDR, /*bEndpointAddress: Endpoint Address (IN)*/
    0x03, /*bmAttributes: Interrupt endpoint*/
    HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
    0x00,
    HID_HS_BINTERVAL, /*bInterval: Polling Interval */
    /* 34 */

    // ===== ENDPOINT OUT (0x02) ДОБАВЛЯЕМ! =====
    0x07,
    USB_DESC_TYPE_ENDPOINT,
    HID_EPOUT_ADDR,
    0x03,
    HID_EPOUT_SIZE,
    0x00,
    HID_HS_BINTERVAL,
    /* 41 */
};

/* USB HID device Other Speed Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_OtherSpeedCfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09, /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    USB_HID_CONFIG_DESC_SIZ,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01, /*bNumInterfaces: 1 interface*/
    0x01, /*bConfigurationValue: Configuration value*/
    0x00, /*iConfiguration: Index of string descriptor describing
the configuration*/
    0xE0, /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32, /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09, /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
    0x00, /*bInterfaceNumber: Number of Interface*/
    0x00, /*bAlternateSetting: Alternate setting*/
    0x02, /*bNumEndpoints*/
    0x03, /*bInterfaceClass: HID*/
    0x00, /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00, /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0, /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09, /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00, /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    HID_MOUSE_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07, /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

    HID_EPIN_ADDR, /*bEndpointAddress: Endpoint Address (IN)*/
    0x03, /*bmAttributes: Interrupt endpoint*/
    HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
    0x00,
    HID_FS_BINTERVAL, /*bInterval: Polling Interval */
    /* 34 */

    // ===== ENDPOINT OUT (0x02) ДОБАВЛЯЕМ! =====
    0x07,
    USB_DESC_TYPE_ENDPOINT,
    HID_EPOUT_ADDR,
    0x03,
    HID_EPOUT_SIZE,
    0x00,
    HID_FS_BINTERVAL,
    /* 41 */
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ] __ALIGN_END = {
    /* 18 */
    0x09, /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00, /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    HID_MOUSE_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC,
    USB_DESC_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
};

__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END = {

    // ===== МЫШЬ (Report ID 2) =====
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x02, // Usage (Mouse)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x02, //   Report ID (2)  <-- ДОБАВЛЯЕМ!
    0x09, 0x01, //   Usage (Pointer)
    0xA1, 0x00, //   Collection (Physical)
    0x05, 0x09, //     Usage Page (Button)
    0x19, 0x01, //     Usage Minimum (1)
    0x29, 0x03, //     Usage Maximum (3)
    0x15, 0x00, //     Logical Minimum (0)
    0x25, 0x01, //     Logical Maximum (1)
    0x75, 0x01, //     Report Size (1)
    0x95, 0x03, //     Report Count (3)
    0x81, 0x02, //     Input (Data,Var,Abs)
    0x75, 0x01, //     Report Size (1)
    0x95, 0x05, //     Report Count (5)
    0x81, 0x01, //     Input (Const)
    0x05, 0x01, //     Usage Page (Generic Desktop)
    0x09, 0x30, //     Usage (X)
    0x09, 0x31, //     Usage (Y)
    0x15, 0x81, //     Logical Minimum (-127)
    0x25, 0x7F, //     Logical Maximum (127)
    0x75, 0x08, //     Report Size (8)
    0x95, 0x02, //     Report Count (2)
    0x81, 0x06, //     Input (Data,Var,Rel)
    0x09, 0x38, //     Usage (Wheel)
    0x15, 0x81, //     Logical Minimum (-127)
    0x25, 0x7F, //     Logical Maximum (127)
    0x75, 0x08, //     Report Size (8)
    0x95, 0x01, //     Report Count (1)
    0x81, 0x06, //     Input (Data,Var,Rel)
    0xC0, //   End Collection (Physical)
    0xC0, // End Collection (Application)
    // 64 байта
    // ===== КЛАВИАТУРА (Report ID 1) =====
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x01, //   Report ID (1)  <-- ДОБАВЛЯЕМ!
    0x05, 0x07, //   Usage Page (Keyboard)
    0x19, 0xE0, //   Usage Minimum (224)
    0x29, 0xE7, //   Usage Maximum (231)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x08, //   Report Count (8)
    0x81, 0x02, //   Input (Data,Var,Abs) - Modifier
    0x75, 0x08, //   Report Size (8)
    0x95, 0x01, //   Report Count (1)
    0x81, 0x01, //   Input (Const) - Reserved
    0x19, 0x00, //   Usage Minimum (0)
    0x29, 0x65, //   Usage Maximum (101)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x65, //   Logical Maximum (101)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x06, //   Report Count (6)
    0x81, 0x00, //   Input (Data,Array) - Key codes
    // ===== LED ИНДИКАТОРЫ (OUTPUT) =====
    0x05, 0x08, //   Usage Page (LEDs)
    0x19, 0x01, //   Usage Minimum (1)
    0x29, 0x03, //   Usage Maximum (3)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x03, //   Report Count (3)
    0x91, 0x02, //   Output (Data,Var,Abs) - LEDs

    0x75, 0x01, //   Report Size (1)
    0x95, 0x05, //   Report Count (5)
    0x91, 0x01, //   Output (Const) - Padding
    0xC0, // End Collection (Application)
    // 67 байт
    // ===== МУЛЬТИМЕДИЙНЫЕ КЛАВИШИ (Report ID 3) =====
    0x05, 0x0C, // Usage Page (Consumer Devices)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x03, //   Report ID (3)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x03, //   Report Count (3) — три бита: Volume Up, Volume Down, Mute
    0x09, 0xE9, //   Usage (Volume Increment)
    0x09, 0xEA, //   Usage (Volume Decrement)
    0x09, 0xE2, //   Usage (Mute) — добавили сюда
    0x81, 0x02, //   Input (Data,Var,Abs)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x05, //   Report Count (5) — оставшиеся 5 бит не используются
    0x81, 0x03, //   Input (Const,Var,Abs)
    0xC0 // End Collection
    // 29 байт
};

/**
 * @}
 */

/** @defgroup USBD_HID_Private_Functions
 * @{
 */

/**
 * @brief  USBD_HID_Init
 *         Initialize the HID interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_HID_Init(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
{
    /* Open EP IN */
    USBD_LL_OpenEP(pdev, HID_EPIN_ADDR, USBD_EP_TYPE_INTR, HID_EPIN_SIZE);
    pdev->ep_in[HID_EPIN_ADDR & 0xFU].is_used = 1U;

    // ===== ОТКРЫВАЕМ OUT-ЭНДПОИНТ ДЛЯ LED =====
    USBD_LL_OpenEP(pdev, HID_EPOUT_ADDR, USBD_EP_TYPE_INTR, HID_EPOUT_SIZE);

    // ===== ПОДГОТАВЛИВАЕМСЯ К ПРИЁМУ ДАННЫХ ОТ ХОСТА =====
    // Переменная для хранения состояния LED
    static uint8_t led_state = 0;
    USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, &led_state, HID_EPOUT_SIZE);

    pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

    if (pdev->pClassData == NULL) {
        return USBD_FAIL;
    }
    // Инициализируем структуру
    ((USBD_HID_HandleTypeDef*)pdev->pClassData)->state = HID_IDLE;
    //((USBD_HID_HandleTypeDef *)pdev->pClassData)->IsReportAvailable = 0;
    ((USBD_HID_HandleTypeDef*)pdev->pClassData)->Protocol = 0x01; // <--- REPORT PROTOCOL!
    ((USBD_HID_HandleTypeDef*)pdev->pClassData)->IdleState = 0;
    ((USBD_HID_HandleTypeDef*)pdev->pClassData)->AltSetting = 0;
    ((USBD_HID_HandleTypeDef*)pdev->pClassData)->state = HID_IDLE;

    return USBD_OK;
}

/**
 * @brief  USBD_HID_Init
 *         DeInitialize the HID layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef* pdev,
    uint8_t cfgidx)
{
    /* Close HID EPs */
    USBD_LL_CloseEP(pdev, HID_EPIN_ADDR);
    pdev->ep_in[HID_EPIN_ADDR & 0xFU].is_used = 0U;

    /* FRee allocated memory */
    if (pdev->pClassData != NULL) {
        USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }

    return USBD_OK;
}

/**
 * @brief  USBD_HID_Setup
 *         Handle the HID specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_HID_Setup(USBD_HandleTypeDef* pdev,
    USBD_SetupReqTypedef* req)
{
    USBD_HID_HandleTypeDef* hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;
    uint16_t len = 0U;
    uint8_t* pbuf = NULL;
    uint16_t status_info = 0U;
    USBD_StatusTypeDef ret = USBD_OK;

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
        switch (req->bRequest) {
        case HID_REQ_SET_PROTOCOL:
            hhid->Protocol = (uint8_t)(req->wValue);
            break;

        case HID_REQ_GET_PROTOCOL:
            USBD_CtlSendData(pdev, (uint8_t*)(void*)&hhid->Protocol, 1U);
            break;

        case HID_REQ_SET_IDLE:
            hhid->IdleState = (uint8_t)(req->wValue >> 8);
            break;

        case HID_REQ_GET_IDLE:
            USBD_CtlSendData(pdev, (uint8_t*)(void*)&hhid->IdleState, 1U);
            break;

        default:
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
        }
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest) {
        case USB_REQ_GET_STATUS:
            if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                USBD_CtlSendData(pdev, (uint8_t*)(void*)&status_info, 2U);
            } else {
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
            }
            break;

        case USB_REQ_GET_DESCRIPTOR:
            if (req->wValue >> 8 == HID_REPORT_DESC) {
                len = MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
                pbuf = HID_MOUSE_ReportDesc;
            } else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE) {
                pbuf = USBD_HID_Desc;
                len = MIN(USB_HID_DESC_SIZ, req->wLength);
            } else {
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
                break;
            }
            USBD_CtlSendData(pdev, pbuf, len);
            break;

        case USB_REQ_GET_INTERFACE:
            if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                USBD_CtlSendData(pdev, (uint8_t*)(void*)&hhid->AltSetting, 1U);
            } else {
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
            }
            break;

        case USB_REQ_SET_INTERFACE:
            if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                hhid->AltSetting = (uint8_t)(req->wValue);
            } else {
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
        }
        break;

    default:
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
        break;
    }

    return ret;
}

/**
 * @brief  USBD_HID_SendReport
 *         Send HID Report
 * @param  pdev: device instance
 * @param  buff: pointer to report
 * @retval status
 */
uint8_t USBD_HID_SendReport(USBD_HandleTypeDef* pdev,
    uint8_t* report,
    uint16_t len)
{
    USBD_HID_HandleTypeDef* hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;

    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        if (hhid->state == HID_IDLE) {
            hhid->state = HID_BUSY;
            USBD_LL_Transmit(pdev,
                HID_EPIN_ADDR,
                report,
                len);
        }
    }
    return USBD_OK;
}

/**
 * @brief  USBD_HID_GetPollingInterval
 *         return polling interval from endpoint descriptor
 * @param  pdev: device instance
 * @retval polling interval
 */
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef* pdev)
{
    uint32_t polling_interval = 0U;

    /* HIGH-speed endpoints */
    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Sets the data transfer polling interval for high speed transfers.
         Values between 1..16 are allowed. Values correspond to interval
         of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
        polling_interval = (((1U << (HID_HS_BINTERVAL - 1U))) / 8U);
    } else /* LOW and FULL-speed endpoints */
    {
        /* Sets the data transfer polling interval for low and full
        speed transfers */
        polling_interval = HID_FS_BINTERVAL;
    }

    return ((uint32_t)(polling_interval));
}

/**
 * @brief  USBD_HID_GetCfgFSDesc
 *         return FS configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_HID_GetFSCfgDesc(uint16_t* length)
{
    *length = sizeof(USBD_HID_CfgFSDesc);
    return USBD_HID_CfgFSDesc;
}

/**
 * @brief  USBD_HID_GetCfgHSDesc
 *         return HS configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_HID_GetHSCfgDesc(uint16_t* length)
{
    *length = sizeof(USBD_HID_CfgHSDesc);
    return USBD_HID_CfgHSDesc;
}

/**
 * @brief  USBD_HID_GetOtherSpeedCfgDesc
 *         return other speed configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_HID_GetOtherSpeedCfgDesc(uint16_t* length)
{
    *length = sizeof(USBD_HID_OtherSpeedCfgDesc);
    return USBD_HID_OtherSpeedCfgDesc;
}

/**
 * @brief  USBD_HID_DataIn
 *         handle data IN Stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef* pdev,
    uint8_t epnum)
{

    /* Ensure that the FIFO is empty before a new transfer, this condition could
    be caused by  a new transfer before the end of the previous transfer */
    ((USBD_HID_HandleTypeDef*)pdev->pClassData)->state = HID_IDLE;
    return USBD_OK;
}

/**
 * @brief  DeviceQualifierDescriptor
 *         return Device Qualifier descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_HID_GetDeviceQualifierDesc(uint16_t* length)
{
    *length = sizeof(USBD_HID_DeviceQualifierDesc);
    return USBD_HID_DeviceQualifierDesc;
}

// ===== МЫШЬ (5 байт) =====
uint8_t USBD_HID_SendMouse(USBD_HandleTypeDef* pdev, int8_t x, int8_t y, int8_t wheel, uint8_t buttons)
{
    USBD_HID_HandleTypeDef* hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;

    if (hhid == NULL)
        return USBD_FAIL;
    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;
    if (hhid->state != HID_IDLE)
        return USBD_BUSY;

    uint8_t report[5] = { 0 };
    report[0] = 0x02; // Report ID 2 (мышь)
    report[1] = buttons & 0x07;
    report[2] = x;
    report[3] = y;
    report[4] = wheel;

    hhid->state = HID_BUSY;
    USBD_StatusTypeDef status = USBD_LL_Transmit(pdev, HID_EPIN_ADDR, report, 5);

    if (status != USBD_OK) {
        hhid->state = HID_IDLE;
        return USBD_FAIL;
    }

    return USBD_OK;
}

// ===== КЛАВИАТУРА (8 байт) =====
// uint8_t USBD_HID_SendKeyboard(USBD_HandleTypeDef *pdev, uint8_t modifier, uint8_t key_code)
uint8_t USBD_HID_SendKeyboard(USBD_HandleTypeDef* pdev, uint8_t* keyboard_report, uint16_t len)
{
    USBD_HID_HandleTypeDef* hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;

    if (hhid == NULL)
        return USBD_FAIL;
    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;
    if (hhid->state != HID_IDLE)
        return USBD_BUSY;
    /*
        uint8_t report[9] = {0};
        report[0] = 0x01;         // Report ID 1 (клавиатура)
        report[1] = modifier;     // Модификаторы
        report[2] = 0x00;         // Зарезервировано
        report[3] = key_code;     // Код клавиши
    */
    hhid->state = HID_BUSY;
    USBD_StatusTypeDef status = USBD_LL_Transmit(pdev, HID_EPIN_ADDR, keyboard_report, 9);

    if (status != USBD_OK) {
        hhid->state = HID_IDLE;
        return USBD_FAIL;
    }

    return USBD_OK;
}

static uint8_t USBD_HID_DataOut(USBD_HandleTypeDef* pdev, uint8_t epnum)
{
    if (epnum == (HID_EPOUT_ADDR & 0x0F)) {
        // ===== АНАЛИЗИРУЕМ СОСТОЯНИЕ LED =====
        // Биты: 0 = NumLock, 1 = CapsLock, 2 = ScrollLock
        if (led_state & 0x02) // Бит 1 = CapsLock
        {
            // Включить CapsLock LED
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        } else {
            // Выключить CapsLock LED
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        }

        // ===== ПОДГОТАВЛИВАЕМСЯ К ПРИЁМУ СЛЕДУЮЩЕГО ПАКЕТА =====
        USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, (uint8_t*)&led_state, HID_EPOUT_SIZE);
    }

    return USBD_OK;
}

uint8_t USBD_HID_SendMedia(USBD_HandleTypeDef* pdev, uint8_t media_code)
{
    USBD_HID_HandleTypeDef* hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;

    if (hhid == NULL)
        return USBD_FAIL;
    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;
    if (hhid->state != HID_IDLE)
        return USBD_BUSY;

    uint8_t report[2] = { 0 };
    report[0] = 0x03; // Report ID 3 (мультимедиа)
    report[1] = media_code; // 0x01 = Volume Up, 0x02 = Volume Down

    hhid->state = HID_BUSY;
    USBD_StatusTypeDef status = USBD_LL_Transmit(pdev, HID_EPIN_ADDR, report, 2);

    if (status != USBD_OK) {
        hhid->state = HID_IDLE;
        return USBD_FAIL;
    }
    return USBD_OK;
}
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
