#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "FileReader.h"
#include "BufferReader.h"
#include "Reader.h"
#include "CRTParser.h"

#define COMPILER_BARRIER() asm volatile("" ::: "memory")

uint8_t run_cart(IDataReader &r);
uint8_t run_launcher(void);

void run_cart_normal(void);
void run_cart_magic_desk(void);
void run_cart_ocean(void);
void run_cart_fun_play(void);
void run_cart_super_games(void);
void run_cart_easyflash(void);
void run_cart_dinamic(void);
void run_cart_zaxxon(void);
void run_cart_kff(void);

void kff_set_command(uint8_t cmd);
bool kff_get_reply(uint8_t cmd, uint8_t *reply);
uint8_t kff_receive_byte(void);
void kff_send_byte(uint8_t data);
void kff_init(void);
void kff_dump(void);

#endif
