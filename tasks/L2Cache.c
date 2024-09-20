/*******************************************************************************
*                                                                              *
*                               Task 2 - L2 Cache                              *
*                                                                              *
*******************************************************************************/

#include "L2Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache L1Cache;
Cache L2Cache;


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




































/*******************************************************************************
 L1 cache 
*******************************************************************************/

/*------------------------------------------------------------------------------ 
Initializes L1 Cache.
------------------------------------------------------------------------------*/
void initCache() {
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
    initCache();
  }

  Tag = getTag(address);
  index = getIndex(address);
  offset = getOffset(address);

  // gets line of the right index
  CacheLine *Line = &L1Cache.lines[index];


  /* access cache */

  // if block NOT present - miss
  
  if (!Line->Valid || Line->Tag != Tag) {  
    //printf("MISS:\t");
    //MemAddress = getMemAddress(address); // get address of the block in memory
    //accessDRAM(MemAddress, TempBlock, MODE_READ);

    accessL2(address, &TempBlock, mode); //Cache L2 writes data into the TempBlock
    
    if ((Line->Valid) && (Line->Dirty)) { // valid line with dirty block
      //accessDRAM(MemAddress, Line->Data, MODE_WRITE); // 'write back' old block

      //TODO: IMPLEMENTAR MECÂNICA DE WRITE-BACK
    }

    memcpy(Line->Data, TempBlock, BLOCK_SIZE); // copy new block to cache line
    //OK ESCREVEU O BLOCO NA LINE...E DEPOIS? NÃO TEM DE DEVOLVER LOGO AO PROGRAMA?
    //OU SEJA, NÃO FALTA FAZER UM COPY DESTA LINHA ATUALIZADA PARA O "DATA"?
    //ESPERA PELA PRÓXIMA CHAMADA DE FUNÇÃO?
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  }
  





  // if block IS present - hit */
    //printf("HIT:");
    // copy info from cache line to data
  if (mode == MODE_READ){
    memcpy(data, &(Line->Data[offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  // copy info from data to cache line 
  if (mode == MODE_WRITE){
    memcpy(&(Line->Data[offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    // it's unsynced w main memory
    Line->Dirty = 1;
  }
    
   // if miss, then replaced with the correct block

  }































/*******************************************************************************
 L2 cache 
*******************************************************************************/

/*------------------------------------------------------------------------------ 
Initializes L2 Cache.
------------------------------------------------------------------------------*/
void initCache() {
  L2Cache.init = 1;
  for (int i = 0; i < L2_CACHE_LINES; i++){ //fix error?
    L2Cache.lines[i].Valid = 0;
    L2Cache.lines[i].Dirty = 0;
    L2Cache.lines[i].Tag = 0;

    /* sets words to 0 */
    for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE)
      L2Cache.lines[i].Data[j] = 0;
    
  }
}

/*------------------------------------------------------------------------------
Program's access point to the L2 Cache.

Notes:
- could maybe be optimized by passing tag, index and offset as args instead of
calculating them a second time.
------------------------------------------------------------------------------*/
void accessL2(uint32_t address, uint8_t *block, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];
  memset(TempBlock, 0, BLOCK_SIZE);

  /* init cache */
  if (L2Cache.init == 0) {
    initCache();
  }

  Tag = getTag(address);
  index = getIndex(address);
  offset = getOffset(address);

  // gets line of the right index
  CacheLine *Line = &L2Cache.lines[index];







  /* access Cache*/
  // if block not present - miss
  if (!Line->Valid || Line->Tag != Tag) {  
    //printf("MISS:\t");
    MemAddress = getMemAddress(address) ;  // get address of the block in memory
    accessDRAM(MemAddress, TempBlock, MODE_READ);

    if ((Line->Valid) && (Line->Dirty)) { // valid line w dirty block
      accessDRAM(MemAddress, Line->Data, MODE_WRITE); // then write back old block
    }

    memcpy(Line->Data, TempBlock, BLOCK_SIZE); // copy new block to cache line
    //MESMA DUVIDA, ESPERA PELA PROXIMA CALL OU FALTA "ENVIAR" O VALOR?
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  }







  /* If it's a hit */
  
    //printf("HIT:");
    // copy info from cache line to data
  if (mode == MODE_READ){
    memcpy(block, &(Line->Data[offset]), WORD_SIZE);
    time += L2_READ_TIME;
  }

  // copy info from data to cache line 
  if (mode == MODE_WRITE){
    memcpy(&(Line->Data[offset]), block, WORD_SIZE);
    time += L2_WRITE_TIME;
    // it's unsynced w main memory
    // NÃO ENTENDI ESTA PARTE DO UNSYNC
    Line->Dirty = 1;
  }
    
   // if miss, then replaced with the correct block

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




