#ifndef COMPRESS_H
#define COMPRESS_H
#include <cstdlib>
#include <iostream>
#include "SetupManager.h"

// This is library module, that controls compression and decompression

// Public routines
char *Compress(SetupManager *sm);     // Controls compression process
char *DeCompress(SetupManager *sm);   // Controls decompression process

// Private routines
char *vqSPIHTsort(SetupManager *sm);  // The (de)compression algorithm
int influence(int i, int j, SetupManager *sm);

#endif
