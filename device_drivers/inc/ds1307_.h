//i2c init requires
#ifndef INC_DS1307_H
#define INC_DS1307_H

#include "i2c_.h"

#define DS1307_BUS_ADDRESS 0x68U

typedef enum {
    wr_ready_flag,
    rd_ready_flag
} ds1307_ready_flags;

typedef enum {
    rate_1Hz,
    rate_4KHz,
    rate_8KHz,
    rate_32KHz
} ds1307_square_wave_rate;

typedef enum {
    seconds,
    minutes,
    hours, // take both hours and am parameters
    day,
    date,
    month,
    year,
    control,
    all
} ds1307_param;

typedef struct {
    _Bool do_unblocking_mode;
    _Bool do_square_wave;
    uint32_t i2c_id;
    ds1307_square_wave_rate square_rate;

    //for writing and reading regarding a data direction.
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    _Bool is_am;
    _Bool do_am;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;

    _Bool is_out_high; //-
} ds1307_handler;

XStatus ds1307_init();
XStatus ds1307_write(ds1307_handler *p_handler, ds1307_param param);
XStatus ds1307_read(_Bool do_unblocking_mode, ds1307_param param);
XStatus ds1307_get_data(ds1307_handler *p_handler, ds1307_param param);
_Bool ds1307_get_ready(ds1307_ready_flags ready_flag);

//todo: stop/start functions
//todo: reinit
//todo: release


#endif /* INC_DS1307_H */
