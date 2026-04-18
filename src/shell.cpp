#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"

#include "board.h"
#include "utils.h"
#include "c64_interface.h"
#include "cartridge.h"
#include "filesystem.h"
#include "menu.h"

#define CMD_BUFFER_SIZE    128

static char path[32];

void print_prompt(bool init=false) {

   if(init)
      printf("\n\n-- GOCart.64 shell --\n\n");

   dir_current(path, 32);
   printf("%s:> ", path);

}

void poll_shell(void) {

   static char cmd_buffer[CMD_BUFFER_SIZE];
   static uint8_t cmd_index = 0;
   static char prev_path[64];

   static DIR dir;
   static FILINFO fno;

   if (uart_is_readable(UART_ID)) {
      char c = uart_getc(UART_ID);

      if (c == '\r') {     // cr + lf
         uart_putc(UART_ID, c);
         uart_putc(UART_ID, '\n');
      } else if (c == '\b') {   // backspace or del
         if (cmd_index > 0) {
            cmd_index--;
            uart_putc(UART_ID, '\b');
            uart_putc(UART_ID, ' ');
            uart_putc(UART_ID, '\b');
         } 
         return;     //
      } else uart_putc(UART_ID, c);    // echo
                                       
      // process command
      if (c == '\r' || c == '\n') {
         cmd_buffer[cmd_index] = '\0';
         trim(cmd_buffer);
         
         char *token = strtok(cmd_buffer, " ");

         if (strcmp(token, "reset") == 0) {
            // reset command
            // parameter: [cpu], [c64]
            token = strtok(NULL, " ");
            if (strcmp(token, "c64") == 0) {
               printf("C64 reset\n");
               c64_set_exrom_game(1, 1);
               launcher_disable(); 
               c64_reset();
            } else if (strcmp(token, "pico") == 0) {
               printf("PICO reset\n");
               launcher_disable(); 
               watchdog_enable(100, 0);
            } else {
               printf("reset [c64 | pico]\n");
            }
         } else if (strcmp(token, "menu") == 0) {
            if (!launcher_running()) {
               run_launcher();
               cmd_index = 0;
               printf("%s:> ", path);
               menu_loop();
               return;     // return when launcher exit
            }
         } else if (strcmp(token, "load") == 0) {
            // load file
            // parameter: <filename>
            token = strtok(NULL, "");
            trim(token);
            if(strlen(path) == 1)
               sprintf(prev_path, "%s%s", path, token);
            else
               sprintf(prev_path, "%s/%s", path, token);
            FileReader fr(prev_path);
            if(fr.eof())
               printf("E: file not found (%s)\n", prev_path);
            else
               run_cart(fr);
         } else if (strcmp(token, "ls") == 0) {
            // list files/directories
            dir_open(&dir, "*");
            while(dir_read(&dir, &fno)) {
               if(fno.fname[0] == 0)
                  break;
               char *name = *fno.fname ? fno.fname : fno.fname;
               if (fno.fattrib & AM_DIR)
                  printf("[DIR] %s\n", name);
               else
                  printf("[FILE] %s (%llu bytes)\n", name, fno.fsize);
            }
            dir_close(&dir);
         } else if (strcmp(token, "cd") == 0) {
            token = strtok(NULL, " ");
            strcpy(prev_path, path);
            if(token[0] == '/') {    // absolute path
               sprintf(path, "%s", token);
            } else {
               if (strcmp(token, ".") == 0) {
                  ;  // do nothing
               } else if (strcmp(token, "..") == 0) {
                  char *last_slash = strrchr(path, '/');
                  if (last_slash != NULL) {
                     *(last_slash) = '\0';
                     if(strlen(path) == 2)   // e.g. "0:"
                        strcpy(path, "/");
                  }
               } else {
                  if(strlen(path) == 1)
                     sprintf(path, "%s%s", prev_path, token); // relative path  (first level)
                  else
                     sprintf(path, "%s/%s", prev_path, token); // relative path
               }
            } 
            if(!dir_change(path)) {
               printf("E: directory %s not found\n", token);
               strcpy(path, prev_path);
            }
         } else if (strcmp(token, "info") == 0) {
            extern char __data_start__;
            extern char __data_end__;
            extern char __bss_start__;
            extern char __bss_end__;
            extern char __StackLimit;
            extern char __StackTop;
            uint32_t data_size = &__data_end__ - &__data_start__;
            uint32_t bss_size  = &__bss_end__ - &__bss_start__;
            uint32_t stack_size = &__StackTop - &__StackLimit;
            printf("DATA:  %lu bytes\n", data_size);
            printf("BSS:   %lu bytes\n", bss_size);
            printf("STACK: %lu bytes reserved\n", stack_size);
            printf("CPU frequency: %.0f MHz\n", clock_get_hz(clk_sys) / 1e6);
            printf("flash size: %d bytes\n", PICO_FLASH_SIZE_BYTES);
            printf("flash sector size: %d bytes\n", FLASH_SECTOR_SIZE);
            printf("flash page size: %d bytes\n", FLASH_PAGE_SIZE);
            printf("flash area address: 0x%X\n", FLASH_AREA_OFFSET);
            printf("XIP_BASE address: 0x%X\n", XIP_BASE);
            extern uint8_t crt_buf[CRT_BUFFER_SIZE];
            printf("max CRT size: %d bytes\n", sizeof(crt_buf));
         } else if (strlen(cmd_buffer) != 0){
            printf("%s: unknown command\n", cmd_buffer);
         }
         cmd_index = 0;
         print_prompt();

      } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
         cmd_buffer[cmd_index++] = c;
      }
   }
}


void run_shell(void) {

   print_prompt(true);

   while(1) {
      poll_shell();      
   }
}
