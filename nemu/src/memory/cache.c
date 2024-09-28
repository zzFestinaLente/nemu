#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

void dram_write(hwaddr_t addr, size_t len, uint32_t data);
void cache_ddr3_read(hwaddr_t addr, void* data);
void cache_ddr3_write(hwaddr_t addr, void *data, uint8_t *mask);

static void initialize_cache_blocks(block *cache, int total_blocks, bool set_dirty) {
    int i;
    for (i = 0; i < total_blocks; i++) {
        cache[i].valid = false;
        if (set_dirty) cache[i].dirty = false;
    }
}

void init_cache() {
    L1_time = 0;
    L2_time = 0;
    srand((unsigned int)time(NULL));
    initialize_cache_blocks((block *)L1_cache, L1_set * L1_line, false);
    initialize_cache_blocks((block *)L2_cache, L2_set * L2_line, true);
}

static int32_t find_or_allocate_cache_block(hwaddr_t address, block *cache, int set_size, int line_size, int set_bit, uint64_t *time) {
    int32_t tag = address >> (set_bit + 6);
    int32_t set = (address >> 6) & (set_size - 1);
    int block_index = set * line_size;
    int i, free_index;
    
    for (i = 0; i < line_size; i++) {
        if (cache[block_index + i].valid && cache[block_index + i].tag == tag) {
            *time += 2;
            return i;
        }
    }
    
    free_index = 0;
    while (free_index < line_size && cache[block_index + free_index].valid) {
        free_index++;
    }
    if (free_index == line_size) free_index = rand() % line_size;
    
    cache[block_index + free_index].tag = tag;
    cache[block_index + free_index].valid = true;
    *time += 200;
    return free_index;
}

static void update_cache_block(hwaddr_t address, size_t length, uint32_t val, block *cache, int set_size, int line_size, int set_bit, uint64_t *time, bool is_l2) {
    int32_t tag = address >> (set_bit + 6);
    int32_t set = (address >> 6) & (set_size - 1);
    int32_t offset = address & 63;
    int block_index = set * line_size;
    int i, bytes_to_write;
    
    for (i = 0; i < line_size; i++) {
        if (cache[block_index + i].valid && cache[block_index + i].tag == tag) {
            *time += 2;
            if (is_l2) cache[block_index + i].dirty = true;
            
            bytes_to_write = (offset + length <= 64) ? length : (64 - offset);
            memcpy(cache[block_index + i].block + offset, &val, bytes_to_write);
            
            if (bytes_to_write < length) {
                update_cache_block(address + bytes_to_write, length - bytes_to_write, val >> (8 * bytes_to_write), cache, set_size, line_size, set_bit, time, is_l2);
            }
            return;
        }
    }
    
    if (!is_l2) {
        *time += 200;
    } else {
        L2_read(address);
        L2_write(address, length, val);
    }
}

int32_t L1_read(hwaddr_t address) {
    int l1_index = find_or_allocate_cache_block(address, (block *)L1_cache, L1_set, L1_line, L1_set_bit, &L1_time);
    int l2_index = L2_read(address);
    int l2_set = (address >> 6) & (L2_set - 1);
    int l1_set = (address >> 6) & (L1_set - 1);
    memcpy(L1_cache[l1_set][l1_index].block, L2_cache[l2_set][l2_index].block, 64);
    return l1_index;
}

void L1_write(hwaddr_t address, size_t length, uint32_t val) {
    update_cache_block(address, length, val, (block *)L1_cache, L1_set, L1_line, L1_set_bit, &L1_time, false);
    L2_write(address, length, val);
}

int32_t L2_read(hwaddr_t address) {
    int32_t l2_set = (address >> 6) & (L2_set - 1);
    int l2_index = find_or_allocate_cache_block(address, (block *)L2_cache, L2_set, L2_line, L2_set_bit, &L2_time);
    uint8_t mask[BURST_LEN * 2];
    uint32_t base_addr, aligned_addr;
    int j, k;
    
    if (L2_cache[l2_set][l2_index].dirty && L2_cache[l2_set][l2_index].valid) {
        memset(mask, 1, sizeof(mask));
        base_addr = (L2_cache[l2_set][l2_index].tag << (L2_set_bit + 6)) | (l2_set << 6);
        for (j = 0; j < 64 / BURST_LEN; j++) {
            cache_ddr3_write(base_addr + BURST_LEN * j, L2_cache[l2_set][l2_index].block + BURST_LEN * j, mask);
        }
    }
    
    L2_cache[l2_set][l2_index].dirty = false;
    aligned_addr = (address >> 6) << 6;
    for (k = 0; k < 64 / BURST_LEN; k++) {
        cache_ddr3_read(aligned_addr + BURST_LEN * k, L2_cache[l2_set][l2_index].block + BURST_LEN * k);
    }
    
    return l2_index;
}

void L2_write(hwaddr_t address, size_t length, uint32_t val) {
    update_cache_block(address, length, val, (block *)L2_cache, L2_set, L2_line, L2_set_bit, &L2_time, true);
}