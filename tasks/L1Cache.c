#include "L1Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache L1Cache;


/**************** Notes ***************/
/*A 'Write-Back Policy' refers to a caching policy in computer science
where data written into the cache is written to the backing store
only when there is a cache conflict with that block*/



/*Quando os dados são atualizados apenas em memória e em cache só é atualizada mais tarde, 
temos a política write-back. Os dados são atualizados em memória somente quando a linha da 
cache está pronta para ser trocada, o que significa que a atualização do armazenamento acontece 
assincronamente numa sequência à parte! É possível começar uma sequência de diferentes maneiras 
antes do retorno da nossa resposta, periodicamente ou integrada na cache baseado numa entrada da 
cache chamada dirty state. Quando este é trocado, voltamos a escrever em memória e 
podemos usar um buffer para permitir a troca de blocos que têm que ser lidos primeiro.*/



/*'Least Recently Used Replacement' is a cache replacement policy in computer science 
where the block that has been least recently accessed is evicted when the cache is full.*/


/* #Blocks are a
power of 2
– Use low-order
address bits*/



/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
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


/** Extracting block's offset (6 least significant bits) 
 * block size = 16 * word size = 64 bytes = 2^6
*/
uint32_t getOffset(uint32_t address){
  return address & 0x3F;  // 0x3F = 0011 1111
}


/** Extract index (8 bits)
 * line size = 256 = 2^8
 */
uint32_t getIndex(uint32_t address){
  return(address >> 6) & 0xFF; // 0xFF = 1111 1111
}

/**
 * Extracting tag (18 most significant bits).
 * 32 - 8 - 6 = 18
 */
uint32_t getTag(uint32_t address){
  return address >> 14; // removing the 14 (6 + 8) least significant bits
}

/* Removes the last 6 bits (offset) */
uint32_t getMemAddress(uint32_t address){
  return address - getOffset(address); 
}

/*********************** L1 cache *************************/

/* Initialize L1 Cache */
void initCache() {
  L1Cache.init = 1;
  for (int i = 0; i < L1_CACHE_LINES; i++ ){
    L1Cache.lines[i].Valid = 0;
    L1Cache.lines[i].Dirty = 0;
    L1Cache.lines[i].Tag = 0;

    /* sets words to 0 */
    for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE)
      L1Cache.lines[i].Data[j] = 0;
    
  }
}


void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];
  memset(TempBlock, 0, BLOCK_SIZE);

  /* init cache */
  if (L1Cache.init == 0) {
    initCache();
  }

  Tag = getTag(address);
  index = getIndex(address);
  offset = getOffset(address);

  // gets line of the right index
  CacheLine *Line = &L1Cache.lines[index];

  /* access Cache*/

  // if block not present - miss
  if (!Line->Valid || Line->Tag != Tag) {  
    MemAddress = getMemAddress(address) ;  // get address of the block in memory
    accessDRAM(MemAddress, TempBlock, MODE_READ);

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      accessDRAM(MemAddress, Line->Data, MODE_WRITE); // write back old block
    }

    memcpy(Line->Data, TempBlock, BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  }  // if miss, then replaced with the correct block

  if (mode == MODE_READ){ // read data from cache line
    memcpy(data, &(Line->Data[offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE){ // write data from cache line
    memcpy(&(Line->Data[offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    // it's unsynced w main memory
    Line->Dirty = 1;
  }
}


void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
