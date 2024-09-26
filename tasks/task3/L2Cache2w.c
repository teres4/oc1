/*******************************************************************************
*                                                                              *
*                            Task 3 - 2 way set associative cache                             *
*                                                                              *
*******************************************************************************/

/*------------------------------------------------------------------------------
Contém 2 entradas em cada set
O número do bloco determina o set
Procura todas as entradas de um determinado set de uma vez
N comparadores 

------------------------------------------------------------------------------*/

#include "L2Cache2w.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;

CacheL1 L1Cache;
CacheL2 L2Cache;


/*******************************************************************************
Time Manipulation 
*******************************************************************************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }



/*******************************************************************************
DRAM memory (byte addressable) 
*******************************************************************************/

/*------------------------------------------------------------------------------
Access DRAM (L2 Cache <-> DRAM).
------------------------------------------------------------------------------*/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}



/*******************************************************************************
Address processing functions.
*******************************************************************************/

/*------------------------------------------------------------------------------
Extracts the block's offset (6 least significant bits).
Block size = 16 * word size = 64 bytes = 2^6 bytes.
Block offset = 6 bits.
------------------------------------------------------------------------------*/
uint32_t getOffset(uint32_t address){
  return address & 0x3F;  // 0x3F = 0b 0000 0011 1111
}

/*------------------------------------------------------------------------------
Returns line index (8 bits).
Line size = 256 = 2^8.
Line index = 8 bits.
------------------------------------------------------------------------------*/
uint32_t getIndex(uint32_t address){
  return(address >> 6) & 0xFF; // 0xFF = 1111 1111
}

/*------------------------------------------------------------------------------
Extracts the tag, 18 (= 32 - 6 - 8) most significant bits of address.
------------------------------------------------------------------------------*/
uint32_t getTag(uint32_t address){
  return address >> 14; // removing the 14 (6 + 8) least significant bits
}

/*------------------------------------------------------------------------------
Removes the last 6 bits (offset).
------------------------------------------------------------------------------*/
uint32_t getMemAddress(uint32_t address){
  return address - getOffset(address); 
}


void initCache() {
  initCacheL1();
  initCacheL2();
}



/*******************************************************************************
 L1 cache 
*******************************************************************************/

/*------------------------------------------------------------------------------ 
Initializes L1 Cache.
------------------------------------------------------------------------------*/
void initCacheL1() {
  L1Cache.init = 1;
  for (int i = 0; i < L1_CACHE_LINES; i++ ){
    L1Cache.lines[i].Valid = 0;
    L1Cache.lines[i].Dirty = 0;
    L1Cache.lines[i].Tag = 0;

    // set all words to 0
    for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE)
      L1Cache.lines[i].Data[j] = 0;
    
  }
}

/*------------------------------------------------------------------------------
Program's access point to the L1 Cache.
------------------------------------------------------------------------------*/
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];
  memset(TempBlock, 0, BLOCK_SIZE);

  // init cache
  if (L1Cache.init == 0) {
    initCacheL1();
  }

  Tag = getTag(address);
  index = getIndex(address);
  offset = getOffset(address);
  
  // gets line of the right index
  CacheLine *Line = &L1Cache.lines[index];

  /* access cache */

  // if block NOT present - miss
  if (!Line->Valid || Line->Tag != Tag) {  
    
      MemAddress = getMemAddress(address); // get address of the block in memory
      accessL2(MemAddress, TempBlock, MODE_READ); // reads new block from L2

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      accessL2(MemAddress, Line->Data, MODE_WRITE); // write back old block to L2
    }

    memcpy(Line->Data, TempBlock, BLOCK_SIZE); // copy new block to cache line

    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  }

  if (mode == MODE_READ){ // read data from cache line
    memcpy(data, &(Line->Data[offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE){ // write data from cache line
    memcpy(&(Line->Data[offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}



/*******************************************************************************
 L2 cache 
*******************************************************************************/

/*------------------------------------------------------------------------------ 
Initializes L2 Cache.
------------------------------------------------------------------------------*/
void initCacheL2() {
  L2Cache.init = 1;
  for (int i = 0; i < L2_CACHE_SETS; i++){
    for(int j = 0; j < WAYS; j++){
      L2Cache.sets->lines[i].Valid = 0;
      L2Cache.sets->lines[i].Dirty = 0;
      L2Cache.sets->lines[i].Tag = 0;
      L2Cache.sets->lines[i].Time = 0;

      /* sets words to 0 */
      for (int f = 0; f < BLOCK_SIZE; f += WORD_SIZE){
        L2Cache.sets[i].lines[j].Data[f] = 0;

      }
    }
  }
}


/*------------------------------------------------------------------------------
Program's access point to the L2 Cache.

2 way set associative cache with LRU
------------------------------------------------------------------------------*/
void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];
  memset(TempBlock, 0, BLOCK_SIZE);

  /* init cache */
  if (L2Cache.init == 0) {
    initCacheL2();
  }

  Tag = getTag(address);
  index = getIndex(address);
  offset = getOffset(address);

  // gets Set of the right index
  Set *Set= &L2Cache.sets[index];

  /* access Cache*/
  for(int i = 0; i < WAYS; i++){

    /*its a hit*/
    if(Set->lines[i].Valid && Set->lines[i].Tag == Tag){
      if (mode == MODE_READ){ // read data from cache line
        memcpy(data, &(Set->lines[i].Data[offset]), WORD_SIZE);
        time += L2_READ_TIME;
        Set->lines[i].Time = time;
        return;
      }

      if (mode == MODE_WRITE){ // write data from cache line
        memcpy(&(Set->lines[i].Data[offset]), data, WORD_SIZE);
        time += L2_WRITE_TIME;
        Set->lines[i].Dirty = 1;
        Set->lines[i].Time = time;
        return;
      }
    }
  }

  /*its a miss*/
  /*determine which line from set to replace*/
  uint32_t leastrecent = 0;
  int way = 0;
  for(int i = WAYS; i > 0; i--){
    if(Set->lines[i].Time < leastrecent || !leastrecent || !Set->lines[i].Valid ){
      leastrecent = Set->lines[i].Time;
      way = i;
    }
  }
  MemAddress = getMemAddress(address) ;  // get address of the block in memory
  accessDRAM(MemAddress, TempBlock, MODE_READ); // access memory and get block

  if ((Set->lines[way].Valid) && (Set->lines[way].Dirty)) { // valid line w dirty block
    accessDRAM(MemAddress, Set->lines[way].Data, MODE_WRITE); // then write back old block
  }

  memcpy(Set->lines[way].Data, TempBlock, BLOCK_SIZE); // copy new block to cache line

  Set->lines[way].Valid = 1;
  Set->lines[way].Tag = Tag;
  Set->lines[way].Dirty = 0;

  if (mode == MODE_READ){ // read data from cache line
    memcpy(data, &(Set->lines[way].Data[offset]), WORD_SIZE);
    time += L2_READ_TIME;
  }

  // copy info from data to cache line 
  if (mode == MODE_WRITE){ // write data from cache line
    memcpy(&(Set->lines[way].Data[offset]), data, WORD_SIZE);
    time += L2_WRITE_TIME;
    // it's unsynced w main memory
    Set->lines[way].Dirty = 1;
  }
  Set->lines[way].Time = time;
}



/*******************************************************************************
 Others.
*******************************************************************************/
void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}




