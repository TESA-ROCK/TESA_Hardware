#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_helper.h"
#include "mem_helper.h" // Include the header for memory functions

// Definitions for get_mem_free(), get_mem_total(), get_mem_available() functions...
// (Assuming the functions' implementations are either here or in another file)

int main() {
    const char db_name[] = "/home/pi4TESA/TESA_Hardware/Day1/ex_05/mem.db";

    // Retrieve memory statistics
    int mem_free_size = get_mem_free();
    int mem_total_size = get_mem_total();
    int mem_available_size = get_mem_available();

    // Print memory information
    printf("Memory Information:\n");
    printf("Memory Total: %d kB\n", mem_total_size);
    printf("Memory Free: %d kB\n", mem_free_size);
    printf("Memory Available: %d kB\n", mem_available_size);

    // Store values in the database
    dbase_init(db_name);
    dbase_append(db_name, mem_free_size);
    dbase_append(db_name, mem_total_size);
    dbase_append(db_name, mem_available_size);

    return 0;
}
