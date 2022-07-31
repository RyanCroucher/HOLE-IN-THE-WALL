/** @file interface.c
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief The interface module is for managing the display when welcome and
          gameover text is shown on the Led Matrix. There is a generic welcome
          message and a game over message which displays score.
*/

#include "interface.h"
#include "tinygl.h"
#include "../fonts/font5x7_1.h"
#include "uint8toa.h"
#include <string.h>

#define TEXT_SCROLL_SPEED 15
#define GREETING_TEXT "Welcome. Press button to start."

static char gameover_text[] = "GAME OVER. SCORE: ";
static uint8_t gameover_text_length = 18;

/** true: displaying greeting, false: displaying gameover text */
static bool displaying_greeting = false;

/** Initalize the interface
    @Param pacer_rate, pacer rate needed for tinygl_init */
void interface_init(uint16_t pacer_rate)
{
    tinygl_init(pacer_rate);
    tinygl_font_set(&font5x7_1);
    tinygl_text_speed_set(TEXT_SCROLL_SPEED);
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
}

/** Sets the text that scrolls across the screen to welcome message */
void interface_set_welcome_text(void)
{
    if (!displaying_greeting) {
        tinygl_text(GREETING_TEXT);
        displaying_greeting = true;
    }
}

/** Sets the text that scrolls across the screen to game over text
    @Param score the current players score, is appended onto the game over message*/
void interface_set_gameover_text(uint8_t score)
{
    if (displaying_greeting) {
        uint8_t score_text_length = 3;
        char new_gameover_string[gameover_text_length + score_text_length + 1];

        strcpy(new_gameover_string, gameover_text);
        uint8toa(score, &new_gameover_string[gameover_text_length], false);

        tinygl_text(new_gameover_string);

        displaying_greeting = false;
    }
}

/** Updates the interface*/
void interface_update(void)
{
    tinygl_update();
}

/** Clears the interface*/
void interface_clear(void)
{
    tinygl_clear();
}
