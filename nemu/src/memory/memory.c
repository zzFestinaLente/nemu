#include "common.h"
#include"memory/cache.h"
#include "cpu/reg.h"
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
    uint32_t L1set = (addr >> 6) & (L1_set - 1);
    uint32_t L1imm = addr & 0x3F;  
    uint32_t cache_index = L1_read(addr);
    uint8_t buffer[128]; 

    uint32_t first_chunk = 64 - L1imm;
    uint32_t bytes_to_copy = (len <= first_chunk) ? len : first_chunk;

    memcpy(buffer, L1_cache[L1set][cache_index].block + L1imm, bytes_to_copy);

    if (len > bytes_to_copy) {
        uint32_t remaining = len - bytes_to_copy;
        uint32_t next_addr = addr + bytes_to_copy;
        uint32_t next_set = (next_addr >> 6) & (L1_set - 1);
        uint32_t next_cache_index = L1_read(next_addr);
        
        memcpy(buffer + bytes_to_copy, L1_cache[next_set][next_cache_index].block, remaining);
    }

    uint32_t result = 0;
    size_t j;
    for (j = 0; j < len; j++) {
        result |= (uint32_t)buffer[j] << (j * 8);
    }

    return result;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
    L1_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

