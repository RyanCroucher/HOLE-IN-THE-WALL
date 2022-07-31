/** @file platforms.c
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief Module for moving platforms. This module controls the creation of
          platforms with a gap for a player to navigate through. Verticle or
          horizontal platforms can be generated, and the rate they shift can be
          increased as the game progresses.
*/

#include "ledmat.h"
#include <stdlib.h>
#include "platforms.h"

#define INITIAL_WALL_SHIFTS_PER_MINUTE 90
#define INITIAL_NEW_WALLS_PER_MINUTE 30

#define MAX_WALL_SHIFTS_PER_MINUTE 180
#define MAX_NEW_WALLS_PER_MINUTE 50

#define VERTICAL_WALL_SPEED_DIVISOR 1.2 //to balance game
#define VERTICAL_WALL_CREATION_SPEED_DIVISOR 1.5 //to balance game

static bool phase;

static uint8_t wall_shifts_per_minute = INITIAL_WALL_SHIFTS_PER_MINUTE; //one 'shift' is one row or one col

static uint8_t new_walls_per_minute = INITIAL_NEW_WALLS_PER_MINUTE;



/* The matrix representing the state of the display in regards to the position of walls */
static uint8_t display_matrix[LEDMAT_ROWS_NUM][LEDMAT_COLS_NUM] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}
};

/* Initalize platforms, set phase to horizontal platforms */
void platforms_init(void)
{

    //init random number generator with a seed
    srandom(1);
    phase = PHASE_HORIZONTAL_PLATFORMS;

}

/* Creates a new wall in the top row, with a hole in a random column */
void create_new_horizontal_wall(void)
{
    uint8_t col_with_hole = random() % LEDMAT_COLS_NUM;

    for (uint8_t col = 0; col < LEDMAT_COLS_NUM; col++) {
        display_matrix[0][col] = (col != col_with_hole);
    }

}

/* Creates either horizontal or vertical wall depending on the current phase */
void create_new_wall(void)
{

    if (phase == PHASE_HORIZONTAL_PLATFORMS) {
        create_new_horizontal_wall();
    } else {
        create_new_vertical_wall();
    }

}

/* Shifts every row in the matrix down, and clears the top row. */
void shift_all_rows_down(void)
{
    for (uint8_t row = LEDMAT_ROWS_NUM-1; row > 0; row--) {
        for (uint8_t col = 0; col < LEDMAT_COLS_NUM; col++) {
            display_matrix[row][col] = display_matrix[row-1][col];
        }
    }


    for (uint8_t col = 0; col < LEDMAT_COLS_NUM; col++) {
        display_matrix[0][col] = 0;
    }
}

/* Creates a new vertical wall on the left column with a hole in random row. */
void create_new_vertical_wall(void) {

    uint8_t row_with_hole = random() % LEDMAT_ROWS_NUM;

    //adjacent to first whole, or on opposite side, so player is always close to a hole
    uint8_t second_row_with_hole = (row_with_hole + 1) % LEDMAT_ROWS_NUM;

    for (uint8_t row = 0; row < LEDMAT_ROWS_NUM; row++) {
        display_matrix[row][0] = !(row == row_with_hole || row == second_row_with_hole);
    }
}

/* Shifts every column in the matrix to the right, and clears the leftmost column */
void shift_all_columns_right(void) {
    for (uint8_t col = LEDMAT_COLS_NUM-1; col > 0; col--) {
        for (uint8_t row = 0; row < LEDMAT_ROWS_NUM; row++) {
            display_matrix[row][col] = display_matrix[row][col-1];
        }
    }

    for (uint8_t row = 0; row < LEDMAT_ROWS_NUM; row++) {
        display_matrix[row][0] = 0;
    }
}

/* Shifts walls down/right depending on the current phase */
void shift_all_walls(void)
{

    if (phase == PHASE_HORIZONTAL_PLATFORMS) {
        shift_all_rows_down();
    } else {
        shift_all_columns_right();
    }

}

/*
Returns a number representing the given column of the platform display matrix.
@Param col the column number to get the current state of
*/
uint8_t get_col_pattern(uint8_t col)
{
    uint8_t pattern = 0;

    for (uint8_t i = 0; i < LEDMAT_ROWS_NUM; i++) {
        pattern |= display_matrix[i][col] << i;
    }

    return pattern;
}

/* Switches the current mode of wall generation between horizontal and vertical walls. */
void change_phase(void)
{
    if (phase == PHASE_HORIZONTAL_PLATFORMS) {
        phase = PHASE_VERTICAL_PLATFORMS;
    } else {
        phase = PHASE_HORIZONTAL_PLATFORMS;
    }
}

/* Returns the current phase */
bool get_phase(void)
{
    return phase;
}

/* Clear LED matrix, every row and every column set to 0 (off).*/
void clear_all_walls(void)
{
    for (uint8_t i = 0; i < LEDMAT_ROWS_NUM; i++) {
        for (uint8_t j = 0; j < LEDMAT_COLS_NUM; j++) {
            display_matrix[i][j] = 0;
        }
    }
}

/** Returns number of rows/cols each platform moves per minute modified by a factor of VERTICAL_WALL_SPEED_DIVISOR
    to improve the gameplay experience */
uint8_t get_wall_shifts_per_minute(void)
{
    if (phase == PHASE_HORIZONTAL_PLATFORMS) {
        return wall_shifts_per_minute;
    } else {
        return wall_shifts_per_minute / VERTICAL_WALL_SPEED_DIVISOR;
    }
}

/** Returns number of walls to create per minute reduced for vertical walls by a factor of VERTICAL_WALL_CREATION_SPEED_DIVISOR
    to improve the gameplay experience */
uint8_t get_new_walls_per_minute(void)
{
    if (phase == PHASE_HORIZONTAL_PLATFORMS) {
        return new_walls_per_minute;
    } else {
        return new_walls_per_minute / VERTICAL_WALL_CREATION_SPEED_DIVISOR;
    }
}

/*
Increases the rate at which walls shift per minute, cannot exceed MAX_WALL_SHIFTS_PER_MINUTE
@Param extra_shift_rate the amount to increment the current shift rate (in shifts per minute)
*/
void increase_wall_shifts_per_minute(uint8_t extra_shift_rate)
{
    wall_shifts_per_minute += extra_shift_rate;
    if (wall_shifts_per_minute > MAX_WALL_SHIFTS_PER_MINUTE)
        wall_shifts_per_minute = MAX_WALL_SHIFTS_PER_MINUTE;
}

/*
Increases the number of walls created, cannot exceed MAX_NEW_WALLS_PER_MINUTE
@Param extra_creation_rate the amount to increase current creation rate by in new walls per minute
*/
void increase_new_walls_per_minute(uint8_t extra_creation_rate)
{
    new_walls_per_minute += extra_creation_rate;
    if (new_walls_per_minute > MAX_NEW_WALLS_PER_MINUTE)
        new_walls_per_minute = MAX_NEW_WALLS_PER_MINUTE;
}

/**
    Returns wall movement and creation speeds to their start values.
    Primarily to reset game after a gameover.
*/
void walls_reset(void)
{
    clear_all_walls();
    wall_shifts_per_minute = INITIAL_WALL_SHIFTS_PER_MINUTE;
    new_walls_per_minute = INITIAL_NEW_WALLS_PER_MINUTE;
    phase = PHASE_HORIZONTAL_PLATFORMS;

}
