#ifndef CACHE_H
#define CACHE_H

#include "common.h"
// Function prototypes
void init_cache(void);
// Cache block structure
typedef struct cache
{
    uint32_t tag;
    uint8_t block[64];
    bool valid;
    bool dirty;
} block;

// L1 cache definitions
#define L1_line 8
#define L1_set_bit 7
#define L1_set (1024 / L1_line)
block L1_cache[L1_set][L1_line];
uint64_t L1_time;
// L1 cache operations
int32_t L1_read(hwaddr_t address);
void L1_write(hwaddr_t address, size_t length, uint32_t val);

// L2 cache definitions
#define L2_line 16
#define L2_set_bit 12
#define L2_set (64 * 1024 / L2_line)
block L2_cache[L2_set][L2_line];
uint64_t L2_time;
// L2 cache operations
int32_t L2_read(hwaddr_t address);
void L2_write(hwaddr_t address, size_t length, uint32_t val);

#endif // CACHE_H