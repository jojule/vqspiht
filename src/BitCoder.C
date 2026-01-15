#include "SetupManager.h"

SetupManager::initCoder(int states = 1)
{
  if (direction == DECOMPRESS)
    size_of_codebuffer = 256; 
  else
    size_of_codebuffer = 1; 
  codebuffer = 0;
  nbc=0;
}

int SetupManager::inBit(int state = 0)
{
  nbc++;
  if (size_of_codebuffer > 255) {
    codebuffer = fgetc(in);
    size_of_codebuffer = 1;
  }
  int r = (codebuffer & size_of_codebuffer) ? 1 : 0;
  size_of_codebuffer = size_of_codebuffer << 1;
  return r;
}

SetupManager::outBit(int bit, int state = 0)
{
  nbc++;
  outbytes = nbc >> 3;
  if (bit) codebuffer = codebuffer | size_of_codebuffer;
  size_of_codebuffer = size_of_codebuffer << 1;
  if (size_of_codebuffer > 255) {
    fputc(codebuffer,out);
    codebuffer = 0;
    size_of_codebuffer = 1;
  }
}

int SetupManager::inByte()
{
  unsigned char c = 0;
  int i;
  for(i=0; i<8; i++) c = (c << 1) | inBit();
  return (int) c;
}

SetupManager::outByte(int byte)
{
  unsigned char c = 128;
  int i;
  for(i=0; i<8; i++) {
    if (c & byte) outBit(1); else outBit(0);
    c = c >> 1;
  }
}

int SetupManager::inInt()
{
  int c = 0;
  int i;
  for(i=0; i<32; i++) c = (c << 1) | inBit();
  return c;
}

SetupManager::outInt(int ii)
{
  unsigned int c = 0x80000000;
  int i;
  for(i=0; i<32; i++) {
    if (c & ((unsigned int)ii)) outBit(1); else outBit(0);
    c = c >> 1;
  }
}

SetupManager::flushCoder()
{
  fputc(codebuffer,out);
}
