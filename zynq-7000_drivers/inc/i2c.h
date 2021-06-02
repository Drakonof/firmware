#ifndef INC_I2C_H
#define INC_I2C_H

#include "types.h"

typedef struct
{
	uint32_t sclk_rate;
} i2c_inition;

typedef struct
{
	volatile boolean ready;
//	boolean          do_unblocking_mode;
	uint32_t         id;
	uint8_t          i2c_address;
	volatile status  init;
} i2c_handle;

status i2c_init(i2c_handle *p_handle, i2c_inition *init);

#endif /* INC_I2C_H */
