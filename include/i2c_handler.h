#ifndef I2C_HANDLER_
#define I2C_HANDLER_

#include "pico/i2c_slave.h"

void i2c_init_regspace(void);
void i2c_debug(void);
void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

#endif
