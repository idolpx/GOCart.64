/*
 * Copyright (c) 2019-2024 Kim Jørgensen
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef MENU_OPTIONS_H
#define MENU_OPTIONS_H

#include <cstdint>

#include "menu.h"
#include "commands.h"

typedef struct
{
    const MENU *prev_menu;

    const char *title;
    uint8_t selected_element;
    uint8_t no_of_elements;
} OPTIONS_STATE;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static OPTIONS_STATE options_state;

#pragma GCC diagnostic pop

typedef struct OPTIONS_ELEMENT_s OPTIONS_ELEMENT;
typedef uint8_t (*options_func)(OPTIONS_STATE *, OPTIONS_ELEMENT *, uint8_t);

struct OPTIONS_ELEMENT_s
{
    char text[ELEMENT_LENGTH];
    options_func callback;
    uint8_t flags;
    uint8_t element_no;
    void (*user_state)(uint8_t);
};

typedef enum
{
    SELECT_FLAG_ACCEPT      = 0x01,
    SELECT_FLAG_MOUNT       = 0x02,
    SELECT_FLAG_VIC         = 0x04,
    SELECT_FLAG_OVERWRITE   = 0x08,
    SELECT_FLAG_DELETE      = 0x10,
    // See SELECT_FLAGS
} SELECT_FLAGS_EXTRA;

OPTIONS_STATE * options_init(const char *title);
OPTIONS_ELEMENT * options_add_element(OPTIONS_STATE *state, options_func callback);
void options_add_empty(OPTIONS_STATE *state);
void options_add_text(OPTIONS_STATE *state, const char *text);
void options_add_text_block(OPTIONS_STATE *state, const char *text);
void options_add_select(OPTIONS_STATE *state, const char *text, uint8_t flags, uint8_t element_no);
void options_add_dir(OPTIONS_STATE *state, const char *text);
uint8_t handle_options(void);

const char * options_element_text(OPTIONS_ELEMENT *element, const char *text);

#endif
