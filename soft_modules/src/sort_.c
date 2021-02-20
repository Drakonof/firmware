#include "sort_.h"

 status swap(void *first, void *second, size_t type_size) {
	size_t size_ = type_size;
	uint8_t buff = 0,
		   *first_ = (uint8_t *)first,
		   *second_ = (uint8_t *)second;

	if ((null_ == first_) && (null_ == second_) && (0 == size_)) {
		return error_;
	}

    while (0 != size_) {
		buff = *first_;
		*first_++ = *second_;
		*second_++ = buff;
		size_--;
	}

	return ok_;
}

status buble_sort(void *p_array, size_t size, size_t type_size,
		          int(*compare)(void *, void*)) {
    boolean is_sort = false_;
	uint32_t i = 0;

	status swap_status = error_;

	void *first = null_, *second = null_;

	if ((null_ == p_array) || (0 == size) || (0 == type_size)) {
		return error_;
	}

    while (false_ == is_sort) {
        is_sort = true_;

        for (i = 0; i < (size - 1); ++i) {
        	first = p_array + (i * type_size);
        	second = p_array + ((i + 1) * type_size);

        	if (0 < compare(first, second)) {
        		swap_status = swap(first, second, type_size);
        		if (ok_ != swap_status) {
        			return swap_status;
        		}

        		is_sort = false_;
        	}
        }
    }

	return ok_;
}
