#include "SetupManager.h"
#include "vqSPIHTFile.h"

char *SetupManager::export_ROI()
{
  int i,j,b,x;
  unsigned char *buf;
  FILE *f;

  f = fopen(ROI_export_filename,"wb");

  if (!f) return "Can't open ROI export file";
  
  fprintf(f,"P4\n# \n%i %i\n",image_width,image_height);

  buf = (unsigned char *) malloc(((image_width+7)>>3)+10);
  for (j=0; j<image_height; j++) {
    for (i=0; i<((image_width+7)& 0xffffff8); i++) {
      if (i<image_width) {
	b = (*(ROI+((j*image_width+i)>>3)) & 
	     (0x80 >> ((j*image_width+i)&7))) ? 1 : 0;
      } else b = 0;
      if (!(i&7)) *(buf+(i>>3)) = 0;
      if (b) *(buf+(i>>3)) |=  0x80 >> (i&7);
    }
    fwrite(buf,1,(image_width+7)>>3,f);
  }
  free( buf);
  fclose(f);

  return NULL;
}

char *SetupManager::loadvqSPIHTImage()
{
  char *errmsg;
  struct vqSPIHTheader h;

  fread((void *) &h, sizeof(struct vqSPIHTheader),1,in);

  if (h.magic != MAGIC) return "Invalid vqSPIHT file header";

  image_bpp = h.image_bpp;
  image_width = h.image_width;
  image_height = h.image_height;
  ct_width = h.ct_width;
  ct_height = h.ct_height;
  wp_bytes = h.wp_bytes;
  wp_mean = h.wp_mean;
  wp_shift = h.wp_shift;
  wp_smoothing = h.wp_smoothing;
  levels = h.levels;
  filesize_trigger = h.filesize_trigger;
  alpha_trigger = h.alpha_trigger;
  dont_use_ROI = h.dont_use_ROI;

  // Initialize coder
  initCoder(7);

  if (!dont_use_ROI) {
    ROI = (unsigned char *) malloc((image_width * image_height)/8+1);
    unsigned char *p;
    int i,j, size = image_width*image_height/8+1;
    for(i=0, p=ROI; i < size; i++) *p++ = 0;
    size = image_width*image_height;
    
    // RLE encode ROI
    for(i=0; i<size;) { 
      i+= inInt();
      j = inInt();
      for(; j>0 && i<size; j--,i++ ) *(ROI + (i>>3)) |= 0x80 >> (i&7);
    }
    //  for(p=ROI, i=0; i<size; i++) 
    //    if (inBit()) *(ROI + (i>>5)) |= 0x80000000 >> (i&31);
  }    

  if (ROI_export_filename) {
    errmsg = export_ROI();
    if (errmsg) return errmsg;
  }

  return NULL;
}

void SetupManager::saveTriggers()
{
  struct vqSPIHTheader h;
  fclose(out);
  out = fopen(out_filename,"rb");
  fread((void *) &h, sizeof(struct vqSPIHTheader),1,out);
  fclose(out);
  out = fopen(out_filename,"r+b");
  rewind(out);
  h.alpha_trigger = alpha_trigger;
  h.filesize_trigger = filesize_trigger;
  fwrite((void *) &h, sizeof(struct vqSPIHTheader),1,out);
  fclose(out);
  out = fopen(out_filename,"ab");
}

char *SetupManager::savevqSPIHTImage()
{
  char *errmsg;
  struct vqSPIHTheader h;

  h.magic = MAGIC;
  h.image_bpp = image_bpp;
  h.image_width = image_width;
  h.image_height = image_height;
  h.ct_width = ct_width;
  h.ct_height = ct_height;
  h.wp_bytes = wp_bytes;
  h.wp_mean = wp_mean;
  h.wp_shift = wp_shift;
  h.wp_smoothing = wp_smoothing;
  h.filesize_trigger = h.alpha_trigger = 0;
  h.levels = levels;
  h.dont_use_ROI = dont_use_ROI;

  fwrite((void *) &h, sizeof(struct vqSPIHTheader),1,out);
  fflush(out);

  // Initialize coder
  initCoder(7);

  if (!dont_use_ROI) {
    // RLE encode ROI
    int i,j,size = image_width*image_height;
    for(i=0; i<size;) { 
      for(j=0; i<size && !(*(ROI + (i>>3)) & (0x80 >> (i&7))); j++,i++ );
      outInt(j);
      for(j=0; i<size && (*(ROI + (i>>3)) & (0x80 >> (i&7))); j++,i++ );
      outInt(j);
      //     outBit((*(ROI + (i>>5)) & (0x80000000 >> (i&31))) ? 1 : 0);
    }
  }

  if (ROI_export_filename) {
    errmsg = export_ROI();
    if (errmsg) return errmsg;
  }

  return NULL;
}

