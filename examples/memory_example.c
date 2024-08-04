/**
 * Created by jraynor on 8/3/2024.
 */
#include "clib/memory_lib.h"

#include <stdint.h>
#include <stdio.h>

int main()
{
    // Create a platform allocator
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);

    // Allocate 100 bytes
    void *ptr = cl_mem_alloc(allocator, 100);
    // Set all bytes to incrementing values looping from 0-99
    for (size_t i = 0; i < 100; ++i)
    {
        ((uint8_t *)ptr)[i] = (uint8_t)i;
    }
    // Log the values as hex
    for (size_t i = 0; i < 100; ++i)
    {
        printf("0x%02X ", ((uint8_t *)ptr)[i]);
        if ((i + 1) % 10 == 0)
        {
            printf("\n");
        }
    }

    // Free the memory
    cl_mem_free(allocator, ptr);
    cl_allocator_destroy(allocator);



    return 0;
}
