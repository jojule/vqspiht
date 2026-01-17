#include "SetupManager.h"

const char *SetupManager::loadPGMImage()
{
  // Check image type (only RAW PGM files supported)
  char str[256];
  char ch;
  fscanf(in,"%s\n", str);
  if (*str != 'P' || *(str+1) != '5') return "Invalid format of input image";
  
  // Skip comments
  while ((ch=fgetc(in)) == '#') while(fgetc(in) != '\n');
  ungetc(ch,in);
  
  // Get geometry information
  fscanf(in,"%i %i\n",&image_width, &image_height);
  
  // Get bpp
  int i;
  fscanf(in,"%i\n",&i);
  switch (i) {
  case 4095 : image_bpp = 12; break;
  case 255 : image_bpp = 8; break;
  default: return "Unknown image depth in PGM file";
  }
  
  // Allocate memory
  if (image_data) return "Memory for image is already allocated. Weird. Bug in the code. :-)";
  image_data = (unsigned short int *) malloc(sizeof(unsigned short int) * image_width * 
					   image_height);
  
  // Read image data
  int j, c;
  unsigned short int *i_d = image_data;
  for (j = 0; j < image_height; j++) 
    for (i = 0; i < image_width; i++) {
      if ((c = getc(in)) == EOF) return "unexpected end of infile";
      *i_d = c;
      if (image_bpp > 8) {
	if ((c = getc(in)) == EOF) return "unexpected end of infile";
	*i_d |= c << 8;
      }
      i_d++;
    }
  
  // Input file is not needed anymore
  fclose(in); in = (FILE *) NULL;
    
  return NULL;
}

const char *SetupManager::savePGMImage()
{
  // Write RAW PGM header
  int i,j; for(j=image_bpp, i=1; j>0 ; j--) i *= 2;
  fprintf(out,"P5\n#\n%i %i\n%i\n",image_width,image_height,i-1);

  // Write image data
  int c;
  int pc;
  int max = 1;
  for(i=image_bpp; i>1; i--) max = (max << 1) | 1 ;
  unsigned short int *i_d = image_data;
  for (j = 0; j < image_height; j++) 
    for (i = 0; i < image_width; i++) {
      pc = *i_d > max ? max : *i_d;
      putc(255 & pc,out);
      if (image_bpp > 8) putc((pc>>8) & 255,out);
      i_d++;
    }
  
  // Output file is not needed anymore
  fclose(out); in = (FILE *) NULL;

  return NULL;
}
