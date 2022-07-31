/** @file platforms.h
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief Header file for platforms.c, defines relevant functions and constants.
*/

#ifndef PLATFORMS_H
#define PLATFORMS_H

#define PHASE_HORIZONTAL_PLATFORMS 0
#define PHASE_VERTICAL_PLATFORMS 1
#define MAX_PHASE_SHIFTS_PER_MINUTE 5

void platforms_init(void);

void create_new_horizontal_wall(void);

void shift_all_rows_down(void);

void create_new_vertical_wall(void);

void create_new_wall(void);

void shift_all_columns_right(void);

void shift_all_walls(void);

uint8_t get_col_pattern(uint8_t);

bool get_phase(void);

void change_phase(void);

void clear_all_walls(void);

uint8_t get_wall_shifts_per_minute(void);
uint8_t get_new_walls_per_minute(void);

void increase_wall_shifts_per_minute(uint8_t);
void increase_new_walls_per_minute(uint8_t);

void walls_reset(void);

#endif
