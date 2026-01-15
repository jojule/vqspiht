#include <iostream>
using namespace std;
#include "SetupManager.h"
#include "qm.h"

void SetupManager::initCoder(int states) //  = 1)
{
  nbc=0;
  outbytes = 0;
  InitModelQM(states);
  if(direction == COMPRESS) {
    InitEncodeQM();
  } else {
    InitDecodeQM(in);
  }
}

int SetupManager::inBit(int state) // = 0)
{
  int r = DecodeBitByQM(in, state);
  nbc++;
  return r;
}

void SetupManager::outBit(int bit, int state) // = 0)
{
  EncodeBitByQM( out,state,bit);
  outbytes = BytesOutQM;
  nbc++;
}

int SetupManager::inByte()
{
  unsigned char c = 0;
  int i;
  for(i=0; i<8; i++) c = (c << 1) | inBit();
  return (int) c;
}

void SetupManager::outByte(int byte)
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

void SetupManager::outInt(int ii)
{
  unsigned int c = 0x80000000;
  int i;
  for(i=0; i<32; i++) {
    if (c & ((unsigned int)ii)) outBit(1); else outBit(0);
    c = c >> 1;
  }
}

void SetupManager::flushCoder()
{
  if (direction == COMPRESS) {
    FlushEncodeQM(out);
  }
  DoneQM();
}
