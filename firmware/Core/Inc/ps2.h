// ps2_driver.h
#ifndef PS2_H
#define PS2_H

#include <stdbool.h>
#include <stdint.h>

uint8_t ps2_send_cmd(uint8_t cmd);
uint8_t ps2_read_byte(void);
bool ps2_read_packet(uint8_t* buffer, uint8_t size);
void ps2_enable_data_report(void);
void ps2_disable_data_report(void);
uint8_t ps2_reset_device(void);
uint8_t ps2_request_data(void);
void tp_init(void);

#endif
