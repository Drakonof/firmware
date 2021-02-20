#ifndef INC_SORT__H
#define INC_SORT__H

#include "platform_.h"

status swap(void *first, void *second, size_t type_size);

status buble_sort(void *p_array, size_t byte_array_size,
		          size_t type_size, int(*compare)(void*, void*));



#endif /* INC_SORT__H */
