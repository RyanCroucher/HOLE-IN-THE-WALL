/** @file interface.h
    @authors Ryan Croucher (rcr69) & Jeremy Roberts (jro162)
    @date 18 October 2021
    @brief A header file for interface.c. defines relevant functions.
*/

#ifndef INTERFACE_H
#define INTERFACE_H

#include "system.h"

void interface_init(uint16_t);

void interface_set_welcome_text(void);

void interface_set_gameover_text(uint8_t);

void interface_update(void);

void interface_clear(void);


#endif
