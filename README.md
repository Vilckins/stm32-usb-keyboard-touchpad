# USB Keyboard + Touchpad on STM32F103

Кастомная USB-клавиатура с тачпадом от Sony Vaio VGN-FZ31 на STM32F103 (Blue Pill).

## ✨ Возможности

- USB HID Composite: клавиатура + мышь
- Матрица клавиатуры 16×8 (Open-Drain)
- Тачпад PS/2 (Sony Vaio)
- Скролл по Fn + движение
- CapsLock LED (OUT-эндпоинт)
- Мультимедиа-клавиши (Volume Up/Down/Mute)
- Отправка только при изменениях

## 🛠️ Железо

- STM32F103C8T6 (Blue Pill)
- Матрица клавиатуры 16×8
- Тачпад Alps/Synaptics (PS/2)


## 📁 Структура

Core/
├── Inc/
│   ├── Keyboard.h
│   └── main.h
└── Src/
    ├── Keyboard.c
    ├── main.c
    └── ...
USB_DEVICE/
├── App/
│   └── usbd_hid.c
└── Target/
    └── usbd_conf.c

## 🚀 Сборка

```bash
make clean
make

## 🚀 Прошивка

st-flash write build/usb_keyboard.bin 0x8000000
