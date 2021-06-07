//i2c init requires
#ifndef INC_DS1307_H
#define INC_DS1307_H

#include "i2c_.h"

#define DS1307_BUS_ADDRESS 0x68U

typedef enum {
    rate_1Hz,
    rate_4KHz,
    rate_8KHz,
    rate_32KHz
} square_wave_rate;

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
    boolean do_unblocking_mode;
    boolean do_square_wave;
    uint32_t i2c_id;
    square_wave_rate square_rate;

    //for writing and reading regarding a data direction.
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    boolean is_am;
    boolean do_am;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;

    boolean is_out_high; //-
} ds1307_handler;

status ds1307_init();
status ds1307_write(ds1307_handler *p_handler, ds1307_param param);
status ds1307_read(boolean do_unblocking_mode, ds1307_param param);
status ds1307_get_data(ds1307_handler *p_handler, ds1307_param param);
boolean ds1307_get_ready(boolean is_tx_complete);

//todo: stop/start functions
//todo: reinit
//todo: release


#endif /* INC_DS1307_H */
