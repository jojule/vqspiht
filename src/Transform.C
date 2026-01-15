#include <stdlib.h>
#include "Transform.h"
#include "image_bw.h"

char *transformToWavelet(SetupManager *sm)
{
  Image_Coord ic;
  Image_BW ibw;
  
  // Allocate memory for ibw object
  ic.x = sm->image_width; ic.y = sm->image_height;
  ibw.reset(ic,(sm->image_bpp > 8 ? 2 : 1));
  
  // Copy image data to ibw object
  unsigned short int *p = sm->image_data;
  int i,j;
  for (j=0; j<sm->image_height; j++)
    for (i=0; i<sm->image_width; i++)
      ibw(i,j) = *p++;

  // Do the transformation
  ibw.extend();
  ibw.transform();

  // Set pyramid dimensions
  sm->ct_width = ibw.pyramid_dim().x;
  sm->ct_height = ibw.pyramid_dim().y;
  
  // Allocate memory for coefficient table
  sm->coefftable = (int *) malloc(sizeof(int)*sm->ct_height*sm->ct_width);
  
  // Copy transformed image to coefficient table
  int* cp = sm->coefftable;
  for (j=0; j<sm->ct_height; j++)
    for (i=0; i<sm->ct_width; i++)
      *cp++ = (int) ibw(i,j);

  // Set wavelet parameters
  sm->wp_bytes = ibw.pixel_bytes();
  sm->wp_mean = ibw.transform_mean();
  sm->wp_shift = ibw.mean_shift();
  sm->wp_smoothing = ibw.smoothing_factor();
  sm->levels = ibw.pyramid_levels();

  return NULL;
}

char *transformFromWavelet(SetupManager *sm)
{
  Image_Coord ic;
  Image_BW ibw;
  
  // Allocate memory for ibw object
  ic.x = sm->image_width; ic.y = sm->image_height;
  ibw.reset(ic,sm->wp_bytes,sm->wp_mean,sm->wp_shift,sm->wp_smoothing);
  
  // Copy transformed image to ibw object
  int* cp = sm->coefftable;
  int i,j;
  for (j=0; j<sm->ct_height; j++)
    for (i=0; i<sm->ct_width; i++)
      ibw(i,j)= *cp++;

  // Do the transformation
  ibw.recover();

  // Allocate memory for image
  sm->image_data = (unsigned short int *) 
    malloc(sizeof(unsigned short int) * sm->image_height * sm->image_width);
  
  // Copy image data from ibw object
  int pc;
  unsigned short int *p = sm->image_data;
  for (j=0; j<sm->image_height; j++)
    for (i=0; i<sm->image_width; i++) {
      pc = (int) ibw(i,j);
      if (pc < 0) pc = 0;
      *p++ = (unsigned short int) pc;
    }

  return NULL;
}

