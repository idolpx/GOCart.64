#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "board.h"
#include "i2c_handler.h"
#include "c64_interface.h"
#include "filesystem.h"
#include "cartridge.h"
#include "menu.h"
#include "shell.h"

int main(void) {
   
   board_setup();

   c64_set_exrom_game(1, 1);         // <no cartridge>
   c64_reset();

   // mount SD
   if(!filesystem_mount())
      printf("E: SD mount failed\n");

   // setup I2C irq handler
   i2c_slave_init(i2c0, I2C_ADDR, &i2c_slave_handler);
   i2c_init_regspace();

   print_prompt(true);

   while(true) {
      run_launcher();
      menu_loop();
      run_shell();   // keep shell running after menu_loop()
   }

   //run_shell();

   return 0;
}

