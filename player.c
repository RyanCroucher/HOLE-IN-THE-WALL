/** @file player.c
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief A player module that defines the players position and contains useful
          functions for toggling the corresponding player LED.
*/

#include "player.h"
#include "pio.h"

/** struct to hold player row and col position*/
static player_pos_t player_pos;

/** whether player led should be on or off */
static bool player_led_state;

/** useful to set player and powerup led on/off */
static const pio_t ledmat_rows[] =
{
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO,
    LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
    LEDMAT_ROW7_PIO
};

/** Initalize player at centre bottom of LEDMAT */
void player_init(void)
{
    player_pos = (player_pos_t) {.row = 6, .col = 2};
    player_led_state = 0;
}

/** Toggle whether player LED should be on or off */
void toggle_player_led_state(void)
{
    player_led_state = !player_led_state;
}

/** Set LED representing player to on or off depending on player_led_state */
void set_player_led(void)
{
    if (player_led_state) {
        pio_output_low (ledmat_rows[get_player_row()]);
    } else {
        pio_output_high (ledmat_rows[get_player_row()]);
    }
}

/* getters and setters */

void set_player_col(uint8_t col)
{
    player_pos.col = col;
}

void set_player_row(uint8_t row)
{
    player_pos.row = row;
}

uint8_t get_player_col(void)
{
    return player_pos.col;
}

uint8_t get_player_row(void)
{
    return player_pos.row;
}
