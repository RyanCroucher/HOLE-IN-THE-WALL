/** @file powerup.h
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief Header file for powerup.c, declares the powerup position struct,
          useful constants and functions
*/

#ifndef POWERUP_H
#define POWERUP_H

#include "system.h"

#define POWERUP_LED_MODULATE_RATE 500
#define POWERUP_STATE_ON 0

/** The powerup position struct, row and col correlate to the LED matrix */
typedef struct
{
    uint8_t row;
    uint8_t col;

} powerup_pos_t;

void powerup_init(void);

void increment_powerup_led_state(void);

void create_powerup(void);

void destroy_powerup(void);

bool powerup_is_visible(void);

uint8_t get_powerup_state(void);

uint8_t get_powerup_col(void);

uint8_t get_powerup_row(void);

#endif
