
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

#include "crt.h"

uint8_t __not_in_flash() crt_buf[CRT_BUFFER_SIZE];
// CRT file bank -> RAM bank
uint8_t crt_bank_table[64];

CRTHandler crt;

bool debug = false;

const char* CRTFileErrorStrings[FILE_ERR_COUNT] = {
   "OK",
   "open error",
   "format not valid"
};

const char* BankTypeStrings[BANK_TYPE_COUNT] = {
   "RAM",
   "ROM",
   "FLASH"
};

void crt_set_buffer(CRTHandler *crt, uint8_t *buffer) {
   crt->rawdata = buffer;
}

CRTFileError crt_file_open(CRTHandler *crt, const char *filename) {

   FRESULT fr = f_open(&crt->fil, filename, FA_READ);

   if(fr != FR_OK)
      return FILE_ERR_NOT_VALID;

   UINT br;
   DWORD file_size = f_size(&crt->fil);

   if(fr != FR_OK)
      return FILE_ERR_NOT_VALID;      

   printf("crt_file_open: read ok\n");

   return crt_build_banks(crt);
}

CRTFileError crt_file_close(CRTHandler *crt) {

   f_close(&crt->fil);

   return FILE_OK;
}

void crt_init(CRTHandler *crt) {
   for(int i=0; i<128; i++) {
      crt->bank[i].init = false;
      crt->bank[i].offset = 0;
      crt->bank[i].length = 0;
      crt->bank[i].type = (BankType) 0;
      crt->bank[i].number = 0;
      crt->bank[i].load_addrl = crt->bank[i].load_addrh = 0;
      crt->bank[i].datal = crt->bank[i].datah = NULL;
   }
   crt->nbanks = 0;
   crt->size = 0;

   memset(crt_bank_table, 0, sizeof(crt_bank_table));
}

void crt_clear_buffer(CRTHandler *crt) {
}

CRTFileError crt_build_banks(CRTHandler *crt) {

   uint8_t buf[64];
   UINT br;
   uint8_t ram_bank_number = 0;
   uint8_t prev_number = 0;

   // clear
   crt->nbanks = 0;
   crt->size = 0;

   // header

   f_read(&crt->fil, buf, strlen(CRT_SIGNATURE), &br);
   buf[strlen(CRT_SIGNATURE)] = '\0';

   if(strncmp((char *)buf, CRT_SIGNATURE, strlen(CRT_SIGNATURE)))
      return FILE_ERR_FORMAT;

   f_read(&crt->fil, buf, 4, &br);
   uint32_t header_length = ((uint32_t)buf[0] << 24) |
    ((uint32_t)buf[1] << 16) |
    ((uint32_t)buf[2] << 8)  |
    ((uint32_t)buf[3]);

   // cartridge version
   f_read(&crt->fil, buf, 2, &br);

   f_read(&crt->fil, buf, 2, &br);
   crt->type = (buf[0] << 8) | (buf[1]);

   f_read(&crt->fil, buf, 1, &br);
   crt->exrom = bool(buf[0]);
   f_read(&crt->fil, buf, 1, &br);
   crt->game = bool(buf[0]);

   // fix Ocean bad EXROM/GAME
   if(crt->type == 5) {
      crt->exrom = crt->game = 0;
   }
   else if(crt->type == 32) {    // fix EasyFlash bad EXROM/GAME
      crt->exrom = 1; 
      crt->game = 0;
   }

   f_read(&crt->fil, buf, 6, &br);

   f_read(&crt->fil, buf, 32, &br);
   strncpy(crt->name, (char *)buf, 32);

   // chip packets

   uint8_t i = 0;
   bool merge_banks = false;
   uint32_t offset;
   uint16_t length;
   BankType type;
   uint16_t load_addr;
   uint16_t size;

   uint32_t total_size = 0;

   while(!f_eof(&crt->fil)) {

      uint32_t offset = f_tell(&crt->fil);

      f_read(&crt->fil, buf, strlen(CHIP_SIGNATURE), &br);
      buf[strlen(CHIP_SIGNATURE)] = '\0';

      if(strcmp((char *)buf, CHIP_SIGNATURE))
         return FILE_ERR_FORMAT;

      uint32_t file_offset = offset;

      f_read(&crt->fil, buf, 4, &br);

      uint16_t length = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]);

      f_read(&crt->fil, buf, 2, &br);
      BankType type = (BankType) ((buf[0] << 8) | buf[1]);

      f_read(&crt->fil, buf, 2, &br);
      uint8_t number = ((buf[0] << 8) | buf[1]); 

      if (number == prev_number) {
         crt_bank_table[number] = ram_bank_number;
      } else if (number > ram_bank_number + 1) {
         crt_bank_table[number] = ram_bank_number + 1;
         ram_bank_number++;
      } else if (number == ram_bank_number + 1) {
         crt_bank_table[number] = ram_bank_number + 1;
         ram_bank_number = number;
      } 

      prev_number = number;

      f_read(&crt->fil, buf, 2, &br);
      uint16_t load_addr = ((buf[0] << 8) | buf[1]);

      f_read(&crt->fil, buf, 2, &br);
      uint16_t size = ((buf[0] << 8) | buf[1]);

      uint32_t ofs = 0;
      uint8_t bank = crt_bank_table[number];
      
      // ROML bank (and ROMH for >8k images)
      if(load_addr == 0x8000 && size <= 16*1024) {

         // Support ROML only cartridges with more than 64 banks
         if(size <= 8*1024 && (crt->type == 7) ) {

            // Fun Play
            ofs = (( (number >> 3) & 0x07 ) | ( (number & 0x01) << 3)) * 16*1024;

         } else if(size <= 8*1024 && (crt->type == 19) ) {

            // Magic Desk
            bool odd_bank = number & 1;
            number >>= 1;
            ofs = number * 16*1024;
            
            // Use ROMH bank location for odd banks
            if(odd_bank)
               ofs += 8*1024;

         } else if( size <= 8*1024 && (crt->type == 32) ) {

            // EasyFlash
            ofs = bank * 16*1024; 

         } else {

            ofs = number * 16*1024;
         }

      // ROMH bank
      } else if ( (load_addr == 0xa000 || load_addr == 0xe000) && size <= 8*1024 ) {
         if (crt->type == 5) {   
            // Ocean
            ofs = (number % 16) * 16*1024 + 8*1024;
         } else if(crt->type == 32) {
            // EasyFlash
            ofs = bank * 16*1024 + 8*1024;
         } else {
            ofs = number * 16*1024 + 8*1024;
         }
      } else if ( load_addr == 0xf000 && size <= 4*1024 ) {
         if(crt->type == 32) {
            // EasyFlash
            ofs = bank * 16*1024 + 8*1024;
         } else { 
            ofs = number * 16*1024 + 8*1024;
         }
      }

      uint8_t *read_buf = crt_buf + ofs;
      f_read(&crt->fil, read_buf, size, &br);
      
      printf("bank %d, load_addr: 0x%X, ofs: %u, crt_bank_table[%d]: %d, br: %d\n", 
            number, load_addr, ofs, number, crt_bank_table[number], br);

      // Mirror 4k image
      if(size <= 4*1024)
         memcpy(&read_buf[4*1024], read_buf, 4*1024);

      total_size += size;
   }

   crt->size = total_size;

   return FILE_OK;
}

void crt_print(CRTHandler *crt) {

   printf("name: %s\n", crt->name);
   printf("hardware type: %d\n", crt->type);
   printf("exrom: %d, game: %d\n", crt->exrom, crt->game);

   for(int i=0; i<MAX_BANKS_NUM; i++) {

      if(crt->bank[i].init) {
         printf("bank: %d\n", crt->bank[i].number);
         printf("\toffset: 0x%X\n", crt->bank[i].offset);
         printf("\tlength: 0x%X\n", crt->bank[i].length);
         printf("\ttype: %s\n", BankTypeStrings[crt->bank[i].type]);
         printf("\tload address L: 0x%X\n", crt->bank[i].load_addrl);
         printf("\tload address H: 0x%X\n", crt->bank[i].load_addrh);
         printf("\tsize: 0x%X\n", crt->bank[i].size);
      }
   }

   printf("Total banks: %d\n", crt->nbanks);
}
