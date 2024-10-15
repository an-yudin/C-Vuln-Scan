#include <stdio.h>
#include <stdint.h>

#define TABLE_SIZE 1024

struct record {
    int8_t* address;
    int64_t size;
};

struct record table[TABLE_SIZE] = {0};
int t_index = 0;

void logAlloca(int32_t* address) {
    struct record newRecord = {(int8_t*)address, 0};
    table[t_index] = newRecord;
    t_index++;
}

void logMalloc(int8_t* address, int64_t size) {
    struct record newRecord = {address, size};
    table[t_index] = newRecord;
    t_index++;
}

void logFree(int8_t* address, int64_t ln, int64_t col) {
    struct record zero = {0, 0};
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (table[i].address == address) {
            table[i] = zero;
            return;
        }
    }
}

void logQuery(int32_t* address, int64_t size, int64_t ln, int64_t col) {
    int8_t* startAddress = (int8_t*)address;
    int8_t* endAddress = startAddress + size;
    int8_t* allocStartAddress, * allocEndAddress;
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (table[i].address != 0 || table[i].size != 0) {
            allocStartAddress = table[i].address;
            allocEndAddress = table[i].address + table[i].size;
            if (table[i].size == 0 && allocStartAddress == startAddress) {
                return;
            }
            if (allocStartAddress <= startAddress && allocEndAddress >= startAddress && allocStartAddress <= endAddress && allocEndAddress >= endAddress) {
                return;
            }
        }
    }
    printf("Use After Free / ln: %li, col: %li\n", ln, col);
}

