#include <stdio.h>      // printf
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "cartridge.h"
#include "cartridge-pio.h"
#include "c64_interface.h"

#include "board.h"

uint8_t crt_buf[CRT_BUFFER_SIZE] = {};
uint8_t crt_map[64] = {};

#define CRT_BANK(bank)          (crt_buf + (uint32_t)(16 * 1024 * bank))

// Fast look-up of 16k ROM bank address
uint8_t *crt_banks[BANKS_NUM] = {
   CRT_BANK(0),
   CRT_BANK(1),
   CRT_BANK(2),
   CRT_BANK(3),
   CRT_BANK(4),
   CRT_BANK(5),
   CRT_BANK(6),
   CRT_BANK(7),
   CRT_BANK(8),
   CRT_BANK(9),
   CRT_BANK(10),
   CRT_BANK(11),
   CRT_BANK(12),
   CRT_BANK(13),
   CRT_BANK(14),
   CRT_BANK(15),
   CRT_BANK(16),
   CRT_BANK(17),
   CRT_BANK(18),
   CRT_BANK(19),
   CRT_BANK(20),
   CRT_BANK(21),
   CRT_BANK(22),
   CRT_BANK(23),
   CRT_BANK(24),
   CRT_BANK(25),
   CRT_BANK(26),
   CRT_BANK(27),
   CRT_BANK(28),
   CRT_BANK(29)
};

typedef struct {
    uint8_t *crt_buf;
    uint8_t **crt_banks;
    uint8_t *crt_map;
} core1_args_t;

uint8_t run_cart(IDataReader &r) {

   multicore_reset_core1();

   // init cart memory
   memset(crt_buf, 0, sizeof(crt_buf));
   memset(crt_map, 0, sizeof(crt_map));

   CRTParser *crt = new CRTParser(r, crt_buf, sizeof(crt_buf), crt_map, sizeof(crt_map));
   if (crt->parse() != true) {
      printf("E: CRT parsing error\n");
      return 1;
   }

   printf("name: %s\n", crt->getName());
   printf("EXROM: %d, GAME: %d\n", crt->getExrom(), crt->getGame());
   c64_set_exrom_game(crt->getExrom(), crt->getGame());
   printf("CRT size: %ld\n", crt->getSize());

   switch(crt->getType()) {

      core1_args_t args;

      case 0:
         // normal cartridge (0)
         printf("cart: 8K, 16K, Ultimax\n");
         args.crt_buf = crt_buf;
         multicore_launch_core1(run_cart_normal);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 5:
         // Ocean (5)
         args.crt_banks = crt_banks;
         printf("cart: Ocean\n");
         multicore_launch_core1(run_cart_ocean);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 7:
         // Fun Play (7)
         args.crt_banks = crt_banks;
         printf("cart: Fun Play\n");
         multicore_launch_core1(run_cart_fun_play);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 8:
         // Super Games (8)
         args.crt_banks = crt_banks;
         printf("cart: Super Games\n");
         multicore_launch_core1(run_cart_super_games);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 17:
         // Dinamic (17)
         args.crt_banks = crt_banks;
         printf("cart: Dinamic\n");
         multicore_launch_core1(run_cart_dinamic);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 18:
         // Zaxxon (18)
         args.crt_banks = crt_banks;
         printf("cart: Zaxxon\n");
         multicore_launch_core1(run_cart_zaxxon);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 19:
         // Magic Desk (19)
         printf("cart: Magic Desk\n");
         args.crt_banks = crt_banks;
         multicore_launch_core1(run_cart_magic_desk);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      case 32:
         // EasyFlash (32)
         args.crt_banks = crt_banks;
         args.crt_map = crt_map;
         printf("cart: EasyFlash\n");
         multicore_launch_core1(run_cart_easyflash);
         multicore_fifo_push_blocking((uint32_t)&args);
         break;

      default:
         printf("E: cartridge type %d - not found\n", crt->getType());
         break;

   } // end switch

   printf("done\n");
   return 0;
}

//

void __time_critical_func(run_cart_normal)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t *rom = p->crt_buf;

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   SET_DATA_MODE_IN
   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if( !(control & ROML_MASK) || ( !(control & ROMH_MASK) ) ) {

         DATA_OUT(rom[addr & 0x3FFF]);
         SET_DATA_MODE_OUT
         if (control & BA_MASK) {
            wait_until(8);
         }
         SET_DATA_MODE_IN
      }
   } // end loop
}

//

void __time_critical_func(run_cart_magic_desk)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;
   volatile uint8_t data;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *rom_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   SET_DATA_MODE_IN
   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if ( (control & PHI2_MASK) )
         SET_DATA_MODE_IN

      if (control & RW_MASK) {
         if( !(control & ROML_MASK) ) {

            DATA_OUT(rom_ptr[addr & 0x1FFF]);
            SET_DATA_MODE_OUT
         }

      } else {
         
         SET_DATA_MODE_IN
         data = DATA_IN;
         if( !(control & IO1_MASK) && !(addr & 0xFF) ) {
            if ( !(data & 0x80)) {
               c64_set_exrom_game(0, 1);
               rom_ptr = banks[(data >> 1) & 0x3f];
               if (data & 0x01) {
                  // Use ROMH location for odd banks
                  rom_ptr += 0x2000;
               }
            } else {
               c64_set_exrom_game(1, 1);
            }
         }
      }
   }  // end loop
}

//

void __time_critical_func(run_cart_ocean)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;
   volatile uint8_t data;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *rom_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   SET_DATA_MODE_IN
   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if ( (control & PHI2_MASK) )
         SET_DATA_MODE_IN

      if (control & RW_MASK) {
         if( !(control & ROML_MASK) || !(control & ROMH_MASK) ) {

            DATA_OUT(rom_ptr[addr & 0x3FFF]);
            SET_DATA_MODE_OUT
         }

      } else {
         
         SET_DATA_MODE_IN
         data = DATA_IN;
         if( !(control & IO1_MASK) && (addr == 0xDE00) )
            rom_ptr = banks[data % 16];
      }
   }  // end loop
}

//

void __time_critical_func(run_cart_fun_play)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;
   volatile uint8_t data;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *rom_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   SET_DATA_MODE_IN
   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if ( (control & PHI2_MASK) )
         SET_DATA_MODE_IN

      if (control & RW_MASK) {
         if( !(control & ROML_MASK) ) {

            DATA_OUT(rom_ptr[addr & 0x3FFF]);
            SET_DATA_MODE_OUT
         }

      } else {
         
         SET_DATA_MODE_IN
         data = DATA_IN;
         if( !(control & IO1_MASK) && (addr == 0xDE00) ) {
            if ( !(data & 0x80)) {
               c64_set_exrom_game(0, 1);
               rom_ptr = banks[( (data >> 3) & 0x07 ) | ( (data & 0x01) << 3)];
            } else {
               c64_set_exrom_game(1, 1);
            }
         }
      }
   }  // end loop
}

//

void __time_critical_func(run_cart_super_games)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;
   volatile uint8_t data;
   bool disable = false;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *rom_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   SET_DATA_MODE_IN
   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if ( (control & PHI2_MASK) )
         SET_DATA_MODE_IN

      if (control & RW_MASK) {

         if( !(control & ROML_MASK) || !(control & ROMH_MASK) ) {

            DATA_OUT(rom_ptr[addr & 0x3FFF]);
            SET_DATA_MODE_OUT
         } 

      } else {
         
         SET_DATA_MODE_IN
         data = DATA_IN;
         if( !(control & IO2_MASK) && !disable) {
            rom_ptr = banks[data & 0x03];
            if( (data & 0x04) ) {
               c64_set_exrom_game(1, 1);
            } else {
               c64_set_exrom_game(0, 0);
            }

            if(data & 0x08)
               disable = true;
         }
      }
   }  // end loop
}

//

__attribute__((optimize("O3"), hot))
void __time_critical_func(run_cart_easyflash)(void) {
   
   volatile uint32_t control;
   volatile uint32_t addr;
   volatile uint8_t data;
   volatile uint8_t ram_buf[256] = {};
   volatile bool exrom, game, dout;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *crt_map = p->crt_map;
   uint8_t *rom_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   while(1) {

      wait_high(PHI2);

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if (control & RW_MASK) {

         dout = false;

         if ((control & (ROML_MASK|ROMH_MASK)) != (ROML_MASK|ROMH_MASK)) {

            SET_DATA_MODE_OUT
            DATA_OUT(rom_ptr[addr & 0x3FFF]);
            dout = true;

         } else if ( !(control & IO2_MASK) ) {
            
            SET_DATA_MODE_OUT
            DATA_OUT(ram_buf[addr & 0xFF]);
            dout = true;
         }

         if (dout) {
            wait_until(2);
            SET_DATA_MODE_IN
            dout = false;
         }

      } else {
        
         data = DATA_IN;
         if( !(control & IO1_MASK) ) {

            switch (addr & 0xff) {

               case 0x00:
                  rom_ptr = banks[crt_map[data & 0x3F]];
                  break;

               case 0x02:
                  exrom = (~data & 0x2) >> 1;
                  game = ~data & 0x1;
                  c64_set_exrom_game(exrom, game);
                  break;
            }

         } else if (!(control & IO2_MASK)) {
            
            ram_buf[addr & 0xff] = data;
         }
      }

   }  // end loop
}

//

void __time_critical_func(run_cart_dinamic)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *rom_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   SET_DATA_MODE_IN
   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if ( (control & PHI2_MASK) )
         SET_DATA_MODE_IN

      if( !(control & IO1_MASK) ) {
         rom_ptr = banks[addr & 0xF];
         wait_high(IO1);
      }

      if( !(control & ROML_MASK) ) {

         DATA_OUT(rom_ptr[addr & 0x1fff]);
         SET_DATA_MODE_OUT
      }  
   }  // end loop
}

//

void __time_critical_func(run_cart_zaxxon)(void) {

   volatile uint32_t control;
   volatile uint32_t addr;

   core1_args_t *p = (core1_args_t*) multicore_fifo_pop_blocking();
   uint8_t **banks = p->crt_banks;
   uint8_t *rom_ptr = banks[0];
   uint8_t *rom0_ptr = banks[0];

   c64_reset();

   /* uint32_t irqstatus = */ save_and_disable_interrupts();

   while(1) {

      GPIO_GET_LOW_32(control);
      addr = (control & ADDR_GPIO_MASK);
      COMPILER_BARRIER();

      if( control & PHI2_MASK) {

         SET_DATA_MODE_IN

         if( !(control & ROML_MASK) ) {

            DATA_OUT(rom0_ptr[addr & 0x0fff]);
            SET_DATA_MODE_OUT

            rom_ptr = banks[addr & 0x1000 ? 1 : 0];

         } else if( !(control & ROMH_MASK) ) {

            DATA_OUT(rom_ptr[addr & 0x3fff]);
            SET_DATA_MODE_OUT
         }
      }
   }  // end loop
}

//
