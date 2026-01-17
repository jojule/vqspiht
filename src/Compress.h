#ifndef COMPRESS_H
#define COMPRESS_H
#include <cstdlib>
#include <iostream>
#include "SetupManager.h"

// This is library module, that controls compression and decompression

// Public routines
const char *Compress(SetupManager *sm);     // Controls compression process
const char *DeCompress(SetupManager *sm);   // Controls decompression process

// Private routines
const char *vqSPIHTsort(SetupManager *sm);  // The (de)compression algorithm
int influence(int i, int j, SetupManager *sm);

#endif
