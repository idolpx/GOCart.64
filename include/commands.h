#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdlib.h>

#define ELEMENT_LENGTH 39
#define DIR_NAME_LENGTH 39
#define MAX_ELEMENTS_PAGE 23
#define SEARCH_LENGTH 30

// Use non-breaking spaces as first character in dir name
#define NORMAL_DIR       0x20
#define SEARCH_SUPPORTED 0xa0
#define CLEAR_SEARCH     0xe0

// Use non-breaking spaces as first character for element type
#define NORMAL_ELEMENT   0x20
#define SELECTED_ELEMENT 0xa0
#define TEXT_ELEMENT     0xe0


typedef enum {
    CMD_NONE = 0x00,    // Get reply from C64

    // Menu commands
    CMD_MESSAGE,
    CMD_WARNING,
    CMD_FLASH_MESSAGE,
    CMD_TEXT,
    CMD_TEXT_WAIT,
    CMD_TEXT_READER,

    CMD_MENU,
    CMD_READ_DIR,
    CMD_READ_DIR_PAGE,

    CMD_MOUNT_DISK,
    CMD_WAIT_RESET,     // Disable screen and wait for reset

    // Disk commands
    CMD_NO_DRIVE = 0x10,
    CMD_DISK_ERROR,
    CMD_NOT_FOUND,
    CMD_END_OF_FILE,

    // SYNC commands
    CMD_WAIT_SYNC = 0x50,
    CMD_SYNC = 0x55,

    REPLY_OK = 0x80,

    // Menu replies
    REPLY_DIR,
    REPLY_DIR_ROOT,
    REPLY_DIR_UP,
    REPLY_DIR_PREV_PAGE,
    REPLY_DIR_NEXT_PAGE,

    REPLY_SELECT,
    REPLY_SETTINGS,
    REPLY_BASIC,
    REPLY_KILL,
    REPLY_KILL_C128,

    REPLY_RESET,

    // Disk replies
    REPLY_LOAD = 0x90,
    REPLY_SAVE,

    REPLY_OPEN,
    REPLY_CLOSE,

    REPLY_TALK,
    REPLY_UNTALK,
    REPLY_SEND_BYTE,

    REPLY_LISTEN,
    REPLY_UNLISTEN,
    REPLY_RECEIVE_BYTE
} COMMAND_TYPE;

//bool (*c64_wait_handler)(void);

bool c64_get_reply(uint8_t cmd, uint8_t *reply);
void c64_set_command(uint8_t cmd);
uint8_t c64_receive_byte(void);
void c64_send_byte(uint8_t data);
void c64_receive_data(void *buffer, size_t size);
void c64_send_data(const void *buffer, size_t size);
void c64_wait_for_command(uint8_t cmd);
void c64_send_command(uint8_t cmd);
void c64_interface_sync(void);

char sanitize_char(char c);
char * convert_to_screen_code(char *dest, const char *src);
char petscii_to_ascii(char c);
uint16_t convert_to_ascii(char *dest, const uint8_t *src, uint8_t size);
char ascii_to_petscii(char c);
void c64_send_petscii(const char *text, uint16_t max_len);
void c64_send_petscii_line(const char *text);
void c64_send_cxy_text(uint8_t color, uint8_t x, uint8_t y, const char *text);
void c64_send_text(uint8_t color, uint8_t x, uint8_t y, const char *text);
void c64_send_text_wait(uint8_t color, uint8_t x, uint8_t y, const char *text);
void c64_send_text_reader(const char *text, uint16_t max_len);
void c64_send_message_command(uint8_t cmd, const char *text);
void c64_send_message(const char *text);
void c64_send_warning(const char *text);
void c64_send_prg_message(const char *text);
void c64_receive_string(char *buffer);

#endif
