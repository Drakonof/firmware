#ifndef INC_PLATFORM__H
#define INC_PLATFORM__H

#include "xparameters.h"

#include "config.h"
#include "types_.h"


#define M_write_reg(reg, value)      \
    do {                             \
       *(uint32_t *)(reg) = (value); \
    }while (0);

#define M_read_reg(reg, value)        \
    do {                              \
        (value) = *(uint32_t *)(reg); \
    }while (0);

#define M_or_write_reg(reg, value)     \
    do {                               \
        *(uint32_t *)(reg) |= (value); \
    }while (0)

#define M_and_write_reg(reg, value)    \
    do {                               \
        *(uint32_t *)(reg) &= (value); \
    }while (0)

#define M_status_return_if(exp) \
	do {                        \
        if (exp) {              \
            return error_;      \
        }                       \
    } while(0)

#define M_user_return_if(exp, return_exp) \
    do {                                  \
        if (exp) {                        \
            return return_exp;            \
        }                                 \
    } while(0)

#define M_user_return_assert_if(exp, assert) \
    do {                                     \
        if (exp) {                           \
        	assert = error_;                 \
            return error_;                   \
        }                                    \
    } while(0)

#endif /* INC_PLATFORM__H */
