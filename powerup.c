/** @file powerup.c
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief A Powerup module to handle powerup position, creation and visibility
*/

#include "powerup.h"
#include "pio.h"
#include <stdlib.h>

//number of states the powerup led state will go through
//when state is 0, it is on, otherwise it's off
#define NUM_STATES 31

static powerup_pos_t powerup_pos;

static uint8_t powerup_led_state = 0;
static bool powerup_visible = false;

/** useful to set powerup led on/off */
static const pio_t ledmat_rows[] =
{
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO,
    LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
    LEDMAT_ROW7_PIO
};

/** initialises the powerup position and random seed */
void powerup_init(void)
{
    srand(1);
    powerup_pos = (powerup_pos_t) {.row = 3, .col = 2};
}

/** will set the row that the powerup led is in to high or low, to produce a
    modulated flash, only if the powerup is visible */
void set_powerup_led(void)
{

    if (!powerup_visible) {
        return;
    }

    if (powerup_led_state == POWERUP_STATE_ON) {
        pio_output_low (ledmat_rows[get_powerup_row()]);
    } else {
        pio_output_high (ledmat_rows[get_powerup_row()]);
    }
}

/** Increments the powerup_led_state, then applies a modulus operation
    this ensures the led is only on 1/31 of the time */
void increment_powerup_led_state(void) {
    powerup_led_state = (powerup_led_state + 1) % NUM_STATES;
}

/** Creates a new powerup in a random position and makes it visible */
void create_powerup(void)
{
    powerup_pos.row = random() % LEDMAT_ROWS_NUM;
    powerup_pos.col = random() % LEDMAT_COLS_NUM;

    powerup_visible = true;
}

/** destroys the power up (by hiding it) */
void destroy_powerup(void)
{
    powerup_visible = false;
    powerup_led_state = 0;
}

/** Return whether powerup is visible (whether it 'exists') */
bool powerup_is_visible(void)
{
    return powerup_visible;
}

//getters

uint8_t get_powerup_state(void)
{
    return powerup_led_state;
}

uint8_t get_powerup_col(void)
{
    return powerup_pos.col;
}

uint8_t get_powerup_row(void)
{
    return powerup_pos.row;
}
