#ifndef L2CACHE2W_H
#define L2CACHE2W_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

#define L1_CACHE_LINES (L1_SIZE / BLOCK_SIZE)
#define WAYS 2
#define L2_CACHE_SETS (L2_SIZE / BLOCK_SIZE / WAYS)
void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/***************** Address manipulation **************/
uint32_t getOffset(uint32_t);

uint32_t getIndex(uint32_t);

uint32_t getTag(uint32_t);

uint32_t getMemAddress(uint32_t);

/*********************** Cache *************************/

void initCache();

void initCacheL1();
void accessL1(uint32_t, uint8_t *, uint32_t);

void initCacheL2();
void accessL2(uint32_t , uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;  /*Valid bit: 1 = present, 0 = not present*/
  uint8_t Dirty;
  uint32_t Tag;
  uint32_t Time; /*LRU*/
  uint8_t Data[BLOCK_SIZE];
} CacheLine;

typedef struct CacheL1 {
  uint32_t init;
  CacheLine lines[L1_CACHE_LINES];
} CacheL1;

typedef struct Set{
  CacheLine lines[WAYS];
} Set;

typedef struct CacheL2 {
  uint32_t init;
  Set sets[L2_CACHE_SETS];
} CacheL2;


/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif