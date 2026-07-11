#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

#define PRU0_DRAM 0x00010000
volatile uint32_t *shared = (uint32_t *) (PRU0_DRAM);

extern void START(void);

void main(void) {
    if (shared[0] == 0) {
        shared[0] = 1; 
    }
    if (shared[1] == 0) {
        shared[1] = 1; 
    }
	START();
}
