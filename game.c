/** @file game.c
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief A Hole-In-The-Wall Game designed for the UCFK4, This file contains
          the main game loop which implements the core gameplay logic and utilises supporting modules.
*/

#include "system.h"
#include "pacer.h"
#include "ledmat.h"
#include "display.h"
#include "navswitch.h"
#include "player.h"
#include "platforms.h"
#include "interface.h"
#include "button.h"
#include "powerup.h"
#include "led.h"

#define PACER_RATE 500
#define DISPLAY_RATE 500
#define READ_INPUT_RATE 50

#define GAME_OVER_WAIT_PERIOD 2 /* in seconds */

#define PHASE_CHANGEOVER_DURATION 35 /** in tenths of a second for convenience */
#define PHASE_SWITCHES_PER_MINUTE 3 /** rate at which we switch between horizontal and vertical walls */

#define WALL_SPEED_INCREASE_AMOUNT 10 /* in cols/rows moved per minute */
#define WALL_CREATE_INREASE_AMOUNT 3 /* in new walls per minute */

#define NEW_POWERUPS_PER_MINUTE 3
#define POWERUP_SCREEN_FLASH_SECONDS 1

#define MAX_EIGHT_BIT_VAL 255

/** Following vars are global so we can reset them on game restart */
static uint16_t phase_switch_counter = 0;
static uint16_t new_platform_counter = 0;
static bool player_has_powerup = false;

/** While true, display is overridden to light up the whole screen (during powerup use)*/
static bool screen_is_flashing = false;


/** Returns true if the player is in the same column and row as a piece of a wall */
bool is_player_colliding_with_platform(void)
{
    return get_col_pattern(get_player_col()) & (1 << (get_player_row()));

}

/** Returns true if the player is in the same row and column as a powerup that is active */
bool is_player_colliding_with_powerup(void)
{
    return get_player_col() == get_powerup_col() && get_player_row() == get_powerup_row() && powerup_is_visible();

}

/** Subroutine that handles reading input for the S3 button at READ_INPUT_RATE */
void subroutine_read_button(void)
{
    static uint8_t read_button_counter = 0;

    if (read_button_counter >= PACER_RATE / READ_INPUT_RATE) {
        button_update();
        read_button_counter = 0;
    }

    read_button_counter++;
}

/** Subroutine to display the current game state on the led matrix */
void subroutine_display(void)
{

    static uint8_t display_counter = 0;
    static uint8_t current_render_col = 0;

    if (display_counter >= PACER_RATE / DISPLAY_RATE) {

        /* If something causes the screen to light up, override current column pattern with 0xFF to light all LEDs */
        if (screen_is_flashing) {
            ledmat_display_column(MAX_EIGHT_BIT_VAL, current_render_col);
        } else {

            /* Else render the current column with the pattern representing the current state of the walls */
            ledmat_display_column(get_col_pattern(current_render_col), current_render_col);

            /* override player led row to force it to correct state at time of column rendering*/
            if (current_render_col == get_player_col()) {
                set_player_led();
            }

            /* override powerup led row to force it to correct state at time of column rendering*/
            if (current_render_col == get_powerup_col()) {
                set_powerup_led();
            }
        }

        current_render_col = (current_render_col + 1) % LEDMAT_COLS_NUM;

    }

    display_counter++;

}

/** A subroutine that creates and moves walls at the correct current rate
    @Param in_phase_changeover_period indicates if game phase (wall direction) is currently transitioning
    @Param score pointer whose value we increment every time a new wall is created */
void subroutine_walls(bool in_phase_changeover_period, uint8_t* score)
{

    static uint16_t platform_fall_counter = 0;

    if (platform_fall_counter >= (PACER_RATE * 60 / get_wall_shifts_per_minute())) {
        platform_fall_counter = 0;
        shift_all_walls();
    }

    //Create wall and increment score at correct current rate, unless in a phase transition period.
    if (!in_phase_changeover_period && new_platform_counter >= PACER_RATE * 60 / get_new_walls_per_minute()) {
        new_platform_counter = 0;
        create_new_wall();
        *score += 1;
    }

    platform_fall_counter++;
    new_platform_counter++;
}

/** Subroutine to move us to a phase transition period at rate of PHASE_SWITCHES_PER_MINUTE
    @Param in_phase_changeover_period pointer, toggled to true at rate defined by PHASE_SWITCHES_PER_MINUTE */
void subroutine_check_phase_switch(bool* in_phase_changeover_period)
{

    if (phase_switch_counter >= (PACER_RATE * 60 / PHASE_SWITCHES_PER_MINUTE)) {
        phase_switch_counter = 0;
        *in_phase_changeover_period = true;
        new_platform_counter = 0;
    }

    phase_switch_counter++;

}

/** Subroutine to blink player LED at PLAYER_LED_BLINK_RATE */
void subroutine_player_blink(void)
{
    static uint8_t player_led_blink_counter = 0;

    if (player_led_blink_counter >= PACER_RATE / PLAYER_LED_BLINK_RATE) {
        toggle_player_led_state();
        player_led_blink_counter = 0;
    }

    player_led_blink_counter++;
}

/** Subroutine to toggle powerup LED rapidly, to make it visually distinct */
void subroutine_powerup_modulate(void)
{
    static uint8_t powerup_led_modulate_counter = 0;

    if (powerup_is_visible() && powerup_led_modulate_counter >= PACER_RATE / POWERUP_LED_MODULATE_RATE) {
        increment_powerup_led_state();
        powerup_led_modulate_counter = 0;
    }

    powerup_led_modulate_counter++;
}

/**  Subroutine to read navswitch input at READ_INPUT_RATE and move player according to this input*/
void subroutine_read_navswitch(void)
{

    static uint8_t read_navswitch_counter = 0;

    if (read_navswitch_counter >= PACER_RATE / READ_INPUT_RATE) {
        read_navswitch_counter = 0;
        navswitch_update ();
        if (navswitch_push_event_p (NAVSWITCH_EAST)) {
            if (get_player_col() < LEDMAT_COLS_NUM - 1) {
                set_player_col(get_player_col() + 1);

            //we only allow east->west wrap in horizontal wall phase
            } else if (get_player_col() == LEDMAT_COLS_NUM - 1 && get_phase() == PHASE_HORIZONTAL_PLATFORMS) {
                set_player_col(0);
            }
        } else if (navswitch_push_event_p (NAVSWITCH_WEST)) {
            if (get_player_col() > 0) {
                set_player_col(get_player_col() - 1);

            //we only allow west->east wrap in horizontal wall phase
            } else if (get_player_col() == 0 && get_phase() == PHASE_HORIZONTAL_PLATFORMS) {
                set_player_col(LEDMAT_COLS_NUM-1);
            }
        } else if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
            if(get_player_row() > 0) {
                set_player_row((get_player_row() - 1));

            //we only allow north->south wrap in vertical wall phase
            } else if (get_player_row() == 0 && get_phase() == PHASE_VERTICAL_PLATFORMS){
                set_player_row(LEDMAT_ROWS_NUM-1);
            }
        } else if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
            if(get_player_row() < LEDMAT_ROWS_NUM - 1) {
                set_player_row((get_player_row() + 1));

            //we only allow south->north wrap in vertical wall phase
            } else if (get_player_row() == LEDMAT_ROWS_NUM - 1 && get_phase() == PHASE_VERTICAL_PLATFORMS){
                set_player_row(0);
            }
        }
    }

    read_navswitch_counter++;
}

/** Subroutine to actually change phase at end of phase transition period. Increases speed of wall movement and creation each time.
    @Param in_phase_changeover_period pointer, indicates if program is currently within phase transition period */
void subroutine_phase_changeover(bool* in_phase_changeover_period)
{

    static uint16_t phase_changeover_counter = 0;

    phase_changeover_counter++;
    if (phase_changeover_counter >= PACER_RATE * PHASE_CHANGEOVER_DURATION / 10) {
        *in_phase_changeover_period = false;
        phase_changeover_counter = 0;

        //clear any vestigial walls
        clear_all_walls();

        change_phase();

        increase_wall_shifts_per_minute(WALL_SPEED_INCREASE_AMOUNT);
        increase_new_walls_per_minute(WALL_CREATE_INREASE_AMOUNT);
    }

}

/** Subroutine to handle the text displayed on screen either before or after the game.
    @Param game_over indicates whether game is finished so we display the right text.
    @Param score the final score of the game, to display in game over text.
*/
void subroutine_interface(bool game_over, uint8_t score)
{
    if (!game_over) {
        interface_set_welcome_text();
    } else {
        interface_set_gameover_text(score);
    }

    interface_update();
}

/** Subroutine to handle picking up, creating and using powerups.
    While player has a powerup, blue LED is on. When powerup is used, blue LED turns off.
*/
void subroutine_powerup(void)
{

    static uint16_t powerup_creation_counter = 0;
    static uint16_t powerup_flash_screen_counter = 0;

    /** collect powerup */
    if (is_player_colliding_with_powerup() && !player_has_powerup) {
        player_has_powerup = true;
        led_set(LED1, 1);
        destroy_powerup();
    }

    /** use powerup. lights up the screen*/
    if (button_push_event_p(0) && player_has_powerup) {
        player_has_powerup = false;
        led_set(LED1, 0);
        clear_all_walls();
        screen_is_flashing = true;
    }

    /** Create powerup at rate of NEW_POWERUPS_PER_MINUTE, if player doesn't already have one */
    if (powerup_creation_counter >= (PACER_RATE * 60) / NEW_POWERUPS_PER_MINUTE) {
        if (!player_has_powerup) {
            create_powerup();
        }
        powerup_creation_counter = 0;
    }

    powerup_creation_counter++;

    /** Stop lighting up screen after POWERUP_SCREEN_FLASH_SECONDS */
    if (screen_is_flashing) {
        powerup_flash_screen_counter++;

        if (powerup_flash_screen_counter >= PACER_RATE / POWERUP_SCREEN_FLASH_SECONDS) {
            powerup_flash_screen_counter = 0;
            screen_is_flashing = false;
        }
        new_platform_counter = 0;
    }

}

/** Returns game board to its initial position */
void reset_game(void)
{
    walls_reset();
    player_init();
    phase_switch_counter = 0;
    
    //remove any status of powerup
    led_set(LED1, 0);
    player_has_powerup = false;
}

/** initialisation and main game loop */
int main (void)
{
    //initialise all modules
    system_init ();
    ledmat_init();
    pacer_init(PACER_RATE);
    interface_init(PACER_RATE);
    player_init();
    platforms_init();
    led_init();
    led_set(LED1, 0);

    bool in_phase_changeover_period = false;
    bool game_over = false;
    bool interface_mode = true;

    uint16_t game_over_wait_timer = 0;
    uint8_t score = 0;

    //counter to help us avoid polling buttons during funkit power on
    uint8_t first_startup_counter = 0;

    while (1)
    {

        pacer_wait();

        if (first_startup_counter < MAX_EIGHT_BIT_VAL)
            first_startup_counter++;

        subroutine_read_button();

        /** Locks us into the interface mode until we press the button to continue */
        if (interface_mode) {
            subroutine_interface(game_over, score);

            //ignore button push until funkit has initialised and we've counted about half a second
            if (button_push_event_p(0) && first_startup_counter == MAX_EIGHT_BIT_VAL) {
                score = 0;
                game_over = false;
                interface_mode = false;
                interface_clear();
                subroutine_interface(game_over, score);
                reset_game();
            } else {
                continue;
            }
        }

        /** game ends on player collision with a wall*/
        game_over = is_player_colliding_with_platform();

        /** game ended, and we've rubbed it in for long enough, reset game state and skip rest of loop */
        if (game_over && game_over_wait_timer >= GAME_OVER_WAIT_PERIOD * PACER_RATE) {
            interface_mode = true;
            game_over_wait_timer = 0;
            reset_game();
            continue;
        }

        subroutine_display();
        subroutine_player_blink();
        subroutine_powerup_modulate();

        if (!game_over) {
            subroutine_walls(in_phase_changeover_period, &score);
            subroutine_check_phase_switch(&in_phase_changeover_period);
            subroutine_read_navswitch();

            if (in_phase_changeover_period)
                subroutine_phase_changeover(&in_phase_changeover_period);

            subroutine_powerup();

        }

        //increment the timer that controls how long we wait until we switch to game over screen
        if (game_over) {
            game_over_wait_timer++;
        }

    }
}
