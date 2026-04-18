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

#ifndef MENU_H_
#define MENU_H_

#include <cstdint>

//static CFG_FILE cfg_file;

typedef struct
{
    void *state;
    uint8_t (*dir)(void *state);
    uint8_t (*dir_up)(void *state, bool root);
    uint8_t (*prev_page)(void *state);
    uint8_t (*next_page)(void *state);
    uint8_t (*select)(void *state, uint8_t flags, uint8_t element);
} MENU;

//const MENU *menu;

void menu_loop(void);
void fail_to_read_sd(void);
uint8_t handle_unsupported(const char *file_name);
uint8_t handle_unsupported_ex(const char *title, const char *message, const char *file_name);
uint8_t handle_unsupported_warning(const char *message, const char *file_name, uint8_t element_no);
uint8_t handle_unsaved_crt(const char *file_name, void (*handle_save)(uint8_t));
uint8_t handle_file_options(const char *file_name, uint8_t file_type, uint8_t element_no);
uint8_t handle_upgrade_menu(const char *firmware, uint8_t element_no);
const char * to_petscii_pad(char *dest, const char *src, uint8_t size);
bool format_path(char *buf, bool include_file);
void send_page_end(void);
uint8_t handle_page_end(void);

#endif
