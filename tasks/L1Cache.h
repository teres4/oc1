#ifndef L1CACHE_H
#define L1CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

#define L1_CACHE_LINES (L1_SIZE / BLOCK_SIZE)

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/***************** Address manipulation **************/
uint32_t getOffset(uint32_t address);

uint32_t getIndex(uint32_t address);

uint32_t getTag(uint32_t address);

uint32_t getMemAddress(uint32_t address);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;  /*Valid bit: 1 = present, 0 = not present*/
  uint8_t Dirty;
  uint32_t Tag;
  uint8_t Data[BLOCK_SIZE];
} CacheLine;

typedef struct Cache {
  uint32_t init;
  CacheLine lines[L1_CACHE_LINES];
} Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
