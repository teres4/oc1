#include "L1Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache SimpleCache;

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

/*********************** L1 cache *************************/

void initCache() { SimpleCache.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

    uint32_t index, Tag, MemAddress;
    uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
    if (SimpleCache.init == 0) {
    SimpleCache.line.Valid = 0;
    SimpleCache.init = 1;
    }

    CacheLine *Line = &SimpleCache.line;

    MemAddress = address;
    Tag = address >> 18; // Why do I do this? - tag is 3 most significant bits??? doesnt really make sense, should be 18 bits 

    index = address >> 8; // L1SIZE is (256 * BLOCK_SIZE) so index has to be 8 bits
    
    //MemAddress = address >> 3; // again this....! - idk
    //MemAddress = MemAddress << 3; // address of the block in memory

    /* access Cache*/

    if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      MemAddress = Line->Tag << 3;        // get address of the block in memory
      accessDRAM(MemAddress, &(L1Cache[0]), MODE_WRITE); // then write back old block
    }

    memcpy(&(L1Cache[0]), TempBlock,
           BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    if (0 == (address % 8)) { // even word on block
    memcpy(data, &(L1Cache[0]), WORD_SIZE);
    } else { // odd word on block
    memcpy(data, &(L1Cache[WORD_SIZE]), WORD_SIZE);
    }
    time += L1_READ_TIME;
    }

  if (mode == MODE_WRITE) { // write data from cache line
    if (!(address % 8)) {   // even word on block
    memcpy(&(L1Cache[0]), data, WORD_SIZE);
    } else { // odd word on block
    memcpy(&(L1Cache[WORD_SIZE]), data, WORD_SIZE);
    }
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
    }
}

void read(uint32_t address, uint8_t *data) {
    accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
    accessL1(address, data, MODE_WRITE);
}
