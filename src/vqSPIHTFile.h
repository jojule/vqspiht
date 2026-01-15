#ifndef VQSPIHTFILE_H
#define VQSPIHTFILE_H

#define MAGIC 38426938

struct vqSPIHTheader {
  int magic;

  int image_bpp;

  int image_width, image_height;
  int ct_width, ct_height;
  int levels;

  int alpha_trigger;
  int filesize_trigger;
  int dont_use_ROI;

  int wp_bytes;
  int wp_mean;
  int wp_shift;
  int wp_smoothing;

  int reserved[6];
};

#endif
