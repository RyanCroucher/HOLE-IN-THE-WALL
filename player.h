/** @file player.h
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief The header file for the player.c module, defines constants and a player
          position struct
*/

#ifndef PLAYER_H
#define PLAYER_H

#include "system.h"

#define PLAYER_LED_BLINK_RATE 8

/** The player position struct, row and col correlate to the LED matrix */
typedef struct
{
    uint8_t row;
    uint8_t col;

} player_pos_t;

void player_init(void);
void set_powerup_led(void);

void toggle_player_led_state(void);
void set_player_led(void);

void set_player_col(uint8_t);
uint8_t get_player_col(void);

void set_player_row(uint8_t);
uint8_t get_player_row(void);

#endif
