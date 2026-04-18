#include <stdint.h>
#include <cstring>

#include "commands.h"
#include "cartridge.h"

// KFF commands

bool (*c64_wait_handler)(void);

bool c64_get_reply(uint8_t cmd, uint8_t *reply) {
   return kff_get_reply(cmd, reply);
}


void c64_set_command(uint8_t cmd) {
   kff_set_command(cmd);
}

uint8_t c64_receive_byte(void) {
   return kff_receive_byte();
}

void c64_send_byte(uint8_t data) {
   kff_send_byte(data);
}

void c64_receive_data(void *buffer, size_t size) {
   uint8_t *buf_ptr = (uint8_t *)buffer;

   while (size--) {
      uint8_t c = c64_receive_byte();
      *buf_ptr++ = c;
   }
   //printf("c64_receive_data - buffer: %s, size: %d\n", (char *)buffer, size);
}

void c64_send_data(const void *buffer, size_t size) {
   const uint8_t *buf_ptr = (const uint8_t *)buffer;

   while (size--) {
      c64_send_byte(*buf_ptr++);
   }
}

void c64_wait_for_command(uint8_t cmd) {
   //uint16_t cnt = 0;       // debug
   uint8_t reply;
   //while (!c64_get_reply(cmd, &reply) && cnt++ < 350) {
   while (!c64_get_reply(cmd, &reply)) {
      if (c64_wait_handler && !c64_wait_handler()) {
         c64_set_command(CMD_NONE);
         break;
      }
      //// debug
      //printf("cmd: 0x%X, reply: 0x%X\n", cmd, reply);
    }
    //if(cnt >= 350)
    //   printf("TIMEOUT!\n");
    //printf("cmd: 0x%X, reply: 0x%X\n", cmd, reply);
}

void c64_send_command(uint8_t cmd) {
   c64_set_command(cmd);
   c64_wait_for_command(cmd);
}

void c64_interface_sync(void) {
   c64_set_command(CMD_SYNC);
   //c64_interface(true);     // FIXME
   c64_wait_for_command(CMD_SYNC);
}

char sanitize_char(char c) {
   if (c == '\r' || c == '\n') {
      c = '_';
   }

   return c;
}

char * convert_to_screen_code(char *dest, const char *src) {
   while (*src) {
      uint8_t c = *src++;
      if (c <= 0x1f) {
         c |= 0x80;
      }
      else if (c <= 0x3f) {
         // No conversion
      }
      else if (c <= 0x5f) {
         c &= ~0x40;
      }
      else if (c <= 0x7f) {
         c &= ~0x20;
      }
      else if (c <= 0x9f) {
         c |= 0x40;
      }
      else if (c <= 0xbf) {
         c &= ~0x40;
      }
      else if (c <= 0xfe) {
         c &= ~0x80;
      } else {
         c = 0x5e;
      }

      *dest++ = c;
   }

   return dest;
}

char petscii_to_ascii(char c) {
   if (c & 0x80) {
      c &= 0x7f;
   } else if (c >= 'A' && c <= 'Z') {
      c += 0x20;
   }

   return c;
}

uint16_t convert_to_ascii(char *dest, const uint8_t *src, uint8_t size) {
   uint16_t i;
   for (i=0; i<size-1; i++) {
      char c = *src++;
      if (c == 0) {
         break;
      }

      dest[i] = petscii_to_ascii(c);
   }

   dest[i] = 0;
   return i + 1;
}

char ascii_to_petscii(char c) {
   if (c >= 'A' && c <= 'Z') {
      c += 0x20;
   } else if (c >= 'a' && c <= 'z') {
      c -= 0x20;
   } else if (c == '_') {
      c = 0xa4;
   }

   return c;
}

void c64_send_petscii(const char *text, uint16_t max_len) {
   uint16_t text_len = strlen(text) + 1;
   if (text_len > max_len) {
      text_len = max_len;
   }
   c64_send_data(&text_len, 2);

   for (int i=0; i<(text_len-1); i++) {
      char c = ascii_to_petscii(text[i]);
      c64_send_byte(c);
   }
   c64_send_byte(0);
}

void c64_send_petscii_line(const char *text) {
   c64_send_petscii(text, 800);
}

void c64_send_cxy_text(uint8_t color, uint8_t x, uint8_t y, const char *text) {
   c64_send_byte(color);
   c64_send_byte(x);
   c64_send_byte(y);
   c64_send_petscii_line(text);
}

void c64_send_text(uint8_t color, uint8_t x, uint8_t y, const char *text) {
   c64_send_cxy_text(color, x, y, text);
   c64_send_command(CMD_TEXT);
}

void c64_send_text_wait(uint8_t color, uint8_t x, uint8_t y, const char *text) {
   c64_send_cxy_text(color, x, y, text);
   c64_send_command(CMD_TEXT_WAIT);
}

void c64_send_text_reader(const char *text, uint16_t max_len) {
   c64_send_petscii(text, max_len);
   c64_send_command(CMD_TEXT_READER);
}

void c64_send_message_command(uint8_t cmd, const char *text) {
   c64_send_petscii_line(text);
   c64_send_command(cmd);
}

void c64_send_message(const char *text) {
   c64_send_message_command(CMD_MESSAGE, text);
}

void c64_send_warning(const char *text) {
   c64_send_message_command(CMD_WARNING, text);
}

void c64_send_prg_message(const char *text) {
   c64_send_message_command(CMD_FLASH_MESSAGE, text);
}

void c64_receive_string(char *buffer) {
   uint8_t size = c64_receive_byte();
   c64_receive_data(buffer, size);
   buffer[size] = 0;
}

