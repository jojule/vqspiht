#include "CreateROI.h"
#include <cmath>
#include <iostream>
#include <ctime>
using namespace std;

const char *CreateROI(SetupManager *sm)
{
  int i,j;
  const char *errmsg;

  // Allocate space for the ROI map
  unsigned char *p = sm->ROI = 
    (unsigned char *) malloc(((sm->image_height * sm->image_width)>>3) + 8);

  // Clear ROI
  for(i=0; i<((sm->image_height * sm->image_width)>>3)+1; i++) *p++ = 0;

  // Import ROI map, if user wants to
  if (sm->ROI_import_filename) {
    int i,j,b,x;
    unsigned char *buf;
    FILE *f;
    
    f = fopen(sm->ROI_import_filename,"rb");
    
    if (f == NULL) return "Could not import ROI-map (file open error)";

    char str[512];
    fscanf(f,"%s\n", str);
    if (*str != 'P' || *(str+1) != '4') 
      return "Invalid format of input image";
    int ch;
    while ((ch=fgetc(f)) == '#') while(fgetc(f) != '\n'); ungetc(ch,f);
    int w,h;
    fscanf(f,"%i %i\n",&w, &h);
    if (w != sm->image_width || h != sm->image_height)
      return "Size of the ROI-map is invalid";
    
    buf = (unsigned char *) malloc(((w+7)>>3)+10);
    int a=0;
    for (j=0; j<h; j++, a+=w) {
      fread(buf,1,(w+7)>>3,f);
      for (i=0; i<w; i++) 
	if (*(buf + (i>>3)) & (0x80>>(i&7))) 
	  *(sm->ROI+((i+a)>>3)) |= 0x80 >> ((i+a)&7);
    }
    free(buf);
    fclose(f);
  } else {
    // Detect microcalcs
    if ((errmsg = DetectMicrocalcs(sm)) != NULL) return errmsg;
  }

  return NULL;
}

const char *DetectMicrocalcs(SetupManager *sm)
{
  // f1s = radius of first gaussian filter used to get rid of the background
  int f1s = sm->ucalcdet_backgroundfilterradius;
  // pfs = radius of positive filter
  int pfs = sm->ucalcdet_positivefilterradius;
  // nfs = radius of negative filter
  int nfs = sm->ucalcdet_negativefilterradius;
  // k defines the final treshold in terms of global standard deviation
  float k = sm->ucalcdet_thresholdmultiplier;
  // w defines the coefficient of positive filter
  float w = sm->ucalcdet_positivefilterweight;
  // surrtre defines the treshold used for clipping the surroundings of the
  // greast away. If 0 is given, the clipping is disabled
  int surrtre = sm->ucalcdet_surroundingsclipping;

  unsigned short int *buf;
  unsigned short *img = sm->image_data;
  int iw = sm->image_width;
  int ih = sm->image_height;
  int i,j,x,y;
#define PNF_EXTRABITS 2
  int normalize_nf = nfs - PNF_EXTRABITS;
  int normalize_pf = pfs - PNF_EXTRABITS;
  
  // Allocate space for filtering buffer
  buf = (unsigned short int *) malloc(iw*ih*sizeof(unsigned short int));

  // Copy image to buffer
  for (i=0, j=iw*ih; i<j; i++) *(buf+i) = *(img+i);

  // Do gaussian low-pass filtering using binomial approximation
  while(f1s--) {
    for(j=0,y=0; j<ih; j++, y+=iw) {
      for(i=0; i<iw-1; i++) *(buf+y+i) += *(buf+y+i+1);
      *(buf+y+i) = *(buf+y+i) << 1;
    }
    for(i=0; i<iw; i++) {
      for(j=0,y=0; j<ih-1; j++,y+=iw) *(buf+y+i) += *(buf+y+i+iw);
      *(buf+i+y) = *(buf+i+y) << 1;
    }
    for(j=0,y=0; j<ih; j++, y+=iw) {
      for(i=iw-1; i>0; i--) *(buf+y+i) += *(buf+y+i-1);
      *(buf+y) = *(buf+y) << 1;
    }
    for(i=0; i<iw; i++) {
      for(j=0,y=iw*(ih-1); j<ih-1; j++,y-=iw) *(buf+y+i) += *(buf+y+i-iw);
      *(buf+i+y) = *(buf+i+y) << 1;
    }

    //Normalize filtered image
    for(i=0,j=ih*iw; i<j; i++) *(buf+i) >>= 4;
  }

  int surrtre_pixels = iw*ih; // Initialize the number of all pixels.

  if(sm->ucalcdet_enablecleaning) {
    // Clean the image (remove all the texts, borders, etc)
    // First find the breast
    int east,west,north,south;
    east = west = north = south = 0;
    int center = (ih/2)*iw + iw/2;
    for(i=0, x=iw/2-iw/40; i<x; i+=4) {
      if (*(buf+center+i) >= surrtre) east++; else { if (east>=8) east-=8; else east=0; }
      if (*(buf+center-i) >= surrtre) west++; else { if (west>=8) west-=8; else west=0;}
    }
    for(i=0, x=ih/2-ih/40; i<x; i+=4) {
      if (*(buf+center+i*iw) >= surrtre) south++; else { if (south>=8) south-=8; else south=0; }
      if (*(buf+center-i*iw) >= surrtre) north++; else { if (north>=8) north-=8; else north=0; }
    }
    x = ih>iw?iw/20:ih/20;
    if (east < x && west < x && north < x &&south < x)
      return "No breast found";


    // Determine the orientation of the breast
    if (north > south && north > east && north > west) {
      north = 1; south = west = east = 0;
    } else if (north < south && south > east && south > west) {
      south = 1; north = west = east = 0;
    } else if (east > west && east > north && east > south) {
      east = 1; north = west = south = 0;
    } else if (east < west && west > north && west > south) {
      west = 1; north = east = south = 0;
    } else return "Can't figure out the orientation of the breast\n"
	     "Maybe you use too low scl value ?";
    // Find the end of the breast
    i = center;
    int stop = 0;
    while (*(buf+i) > surrtre && i>(iw<<4) && i<iw*(ih-4) && !stop) {
      x = y;
      y = i;
      if (north) {
	if (*(buf+i+iw) > surrtre) i+= iw;
	else if (*(buf+i+16) > surrtre && x != i+16) i+= 16;
	else if (*(buf+i-16) > surrtre && x != i-16) i-=16;
	else stop = 1;
      }
      if (south) {
	if (*(buf+i-iw) > surrtre) i-= iw;
	else if (*(buf+i+16) > surrtre && x != i+16) i+= 16;
	else if (*(buf+i-16) > surrtre && x != i-16) i-=16;
	else stop = 1;
      }
      if (east) {
	if (*(buf+i-1) > surrtre) i--;
	else if (*(buf+i+(iw<<4)) > surrtre && x != i+(iw<<4)) i+= (iw<<4);
	else if (*(buf+i-(iw<<4)) > surrtre && x != i-(iw<<4)) i-= iw<<4;
	else stop = 1;
      }
      if (west) {
	if (*(buf+i+1) > surrtre) i++;
	else if (*(buf+i+(iw<<4)) > surrtre && x != i+(iw<<4)) i+= (iw<<4);
	else if (*(buf+i-(iw<<4)) > surrtre && x != i-(iw<<4)) i-= iw<<4;
	else stop = 1;
      }
    }
    int x1,x2,y1,y2;
    x1 = y1 = 0; x2 = iw-1; y2 = ih-1;
    if (north) y1 = i/iw + 32;
    if (south) y2 = i/iw - 32;
    if (east) x2  = i%iw - 32;
    if (west) x1  = i%iw + 32;

    if (i<0 || i> iw*ih) 
      return "Something went wrong, while scanning for the end of the breast";
    
    // Clean the side of the image opposite to the breast
    
    for (j=y1; j<=y2; j++) { 
      y = j*iw;
      for (i=x1; i<=x2; i++) *(buf+y+i) = 0;
    }
    
    // Clean the other sides
    if (east) {
      i = iw*(ih/2)+((iw*15)/16);
      while( *(buf+i) > surrtre && i> iw) i-=iw;
      if (i>iw) {
	x1 = y1 = 0; x2 = iw-1; y2 = i/iw-8;
      } else {
	i = iw*(ih/16)+iw-1;
	while( *(buf+i) > surrtre && i> iw*(ih>>4)) i--;
	x1 = y1 = 0; x2 = i%iw-8; y2 = i/iw-4;
      }
      for (j=y1; j<=y2; j++) { 
	y = j*iw;
	for (i=x1; i<=x2; i++) *(buf+y+i) = 0;
      }
      
      i = iw*(ih/2)+((iw*15)/16);
      while( *(buf+i) > surrtre && i < iw*(ih-2)) i+=iw;
      if (i<iw*(ih-2)) {
	x1 = 0; x2 = iw-1; y1 = i/iw+8; y2 = ih-1;
      } else {
	i = iw*((ih*15)/16)+iw-1;
	while( *(buf+i) > surrtre && i> iw*((ih*15)>>4)) i--;
	x1 = 0; x2 = i%iw-8; y1 = i/iw+4; y2 = ih-1;
      }
      for (j=y1; j<=y2; j++) { 
	y = j*iw;
	for (i=x1; i<=x2; i++) *(buf+y+i) = 0;
      }
    }
    
    if (west) {
      i = iw*(ih/2)+(iw/16);
      while( *(buf+i) > surrtre && i> iw) i-=iw;
      if (i>iw) {
	x1 = y1 = 0; x2 = iw-1; y2 = i/iw-8;
      } else {
	i = iw*(ih/16);
	while( *(buf+i) > surrtre && i< iw*((ih>>4)+1)-1) i++;
	x1 = i%iw+8; y1 = 0; x2 =iw-1; y2 = i/iw-4;
      }
      for (j=y1; j<=y2; j++) { 
	y = j*iw;
	for (i=x1; i<=x2; i++) *(buf+y+i) = 0;
      }
      
      i = iw*(ih/2)+(iw/16);
      while( *(buf+i) > surrtre && i < iw*(ih-2)) i+=iw;
      if (i<iw*(ih-2)) {
	x1 = 0; x2 = iw-1; y1 = i/iw+8; y2 = ih-1;
      } else {
	i = iw*((ih*15)/16);
	while( *(buf+i) > surrtre && i< iw*(((ih*15)>>4)+1)-1) i++;
	x1 = i%iw+8; x2 = iw-1; y1 = i/iw+4; y2 = ih-1;
      }
      for (j=y1; j<=y2; j++) { 
	y = j*iw;
	for (i=x1; i<=x2; i++) *(buf+y+i) = 0;
      }
    }
    
    /// !!!!!!! SOUTH AND NORTH ARE NOT DONE YET !!!!
    if (south || north) 
      return "NOT IMPLEMENTED YET (breast in north or in south) !!!"
	" --> You can use a quick fix, rotate the picture 90 degrees ;-)"
	" ( And remember to complain about it )";
    
  } // The end of the cleaning part

  // Calculate the difference of original and filtered
  surrtre_pixels = 0;
  for(i=0, x=ih*iw; i<x; i++) {
    j = *(img+i) - *(buf+i);
    y = *(buf+i) >= surrtre;
    if (y) surrtre_pixels++;
    *(buf+i) = j > 0 && y ? j : 0;
  }

  // Allocate space for second buffer and copy the first buffer into it
  unsigned short int *buf2 = (unsigned short int *) malloc(iw*ih*sizeof(unsigned short int));
  for (i=0, j=iw*ih; i<j; i++) *(buf2+i) = *(buf+i);

  // Filter the first buffer with the positive gaussian filter
  while(pfs--) {
    for(j=0,y=0; j<ih; j++, y+=iw) {
      for(i=0; i<iw-1; i++) *(buf+y+i) += *(buf+y+i+1);
      *(buf+y+i) = *(buf+y+i) << 1;
    }
    for(i=0; i<iw; i++) {
      for(j=0,y=0; j<ih-1; j++,y+=iw) *(buf+y+i) += *(buf+y+i+iw);
      *(buf+i+y) = *(buf+i+y) << 1;
    }
    for(j=0,y=0; j<ih; j++, y+=iw) {
      for(i=iw-1; i>0; i--) *(buf+y+i) += *(buf+y+i-1);
      *(buf+y) = *(buf+y) << 1;
    }
    for(i=0; i<iw; i++) {
      for(j=0,y=iw*(ih-1); j<ih-1; j++,y-=iw) *(buf+y+i) += *(buf+y+i-iw);
      *(buf+i+y) = *(buf+i+y) << 1;
    }
    //Normalize filtered image
    if (pfs < normalize_pf)
      for(i=0,j=ih*iw; i<j; i++) *(buf+i) >>= 4;
  }

  // Filter the second buffer with the positive gaussian filter
  while(nfs--) {
    for(j=0,y=0; j<ih; j++, y+=iw) {
      for(i=0; i<iw-1; i++) *(buf2+y+i) += *(buf2+y+i+1);
      *(buf2+y+i) = *(buf2+y+i) << 1;
    }
    for(i=0; i<iw; i++) {
      for(j=0,y=0; j<ih-1; j++,y+=iw) *(buf2+y+i) += *(buf2+y+i+iw);
      *(buf2+i+y) = *(buf2+i+y) << 1;
    }
    for(j=0,y=0; j<ih; j++, y+=iw) {
      for(i=iw-1; i>0; i--) *(buf2+y+i) += *(buf2+y+i-1);
      *(buf2+y) = *(buf2+y) << 1;
    }
    for(i=0; i<iw; i++) {
      for(j=0,y=iw*(ih-1); j<ih-1; j++,y-=iw) *(buf2+y+i) += *(buf2+y+i-iw);
      *(buf2+i+y) = *(buf2+i+y) << 1;
    }
    //Normalize filtered image
    if (nfs < normalize_nf)
      for(i=0,j=ih*iw; i<j; i++) *(buf2+i) >>= 4;
  }

  // Calculate the difference of the filters and the mean and 
  double mean = 0;
  for(i=0, x=ih*iw; i<x; i++) {
    j = ((int) (w * (float)*(buf+i))) - *(buf2+i);
    j = j > 0 ? j : 0;
    *(buf+i) = j;
    mean += (double) j;
  }
  mean /= (double) surrtre_pixels;


  // Buffer 2 is not needed anymore
  free(buf2);


  // Calculate the standard deviation, and the suitable treshold value
  j=0;
  double sdev = 0;
  double tmp;
  for(i=0, x=ih*iw; i<x; i+=64) {
    if (*(buf+i)) {
      tmp = ((double) *(buf+i)) - mean;
      sdev += tmp * tmp;
    }
  }
  sdev = sqrt(sdev/(double)((surrtre_pixels/64)));
  int treshold = (int) (k * sdev * (1<<PNF_EXTRABITS));

  // OR tresholded buf to ROI
  for(i=0, x=ih*iw; i<x; i++) {
    if (*(buf+i) >= treshold)
      *(sm->ROI + (i>>3)) |= 0x80 >> (i&7);
  }
  
  free(buf);

  // Do the morphological area enlargening for ROI. This should only
  // be done, if the ROI has been empty before microcalcdetection.
  // (if not, add additional temporary bit buffer before OR:ring with
  // actual ROI)

  int oo,s,o;
  unsigned char ttt, t1, t2;
  int mes = sm->ucalcdet_morphoenlargesize;

  // Vertical enlargening
  for(i=1, x=((ih*iw)>>3); i<x; i++) {
    oo = 0;
    ttt = *(sm->ROI+i);
    for (j=1; j<= mes; j++) {
      oo += iw;
      s = oo & 7;
      o = oo >> 3;
      if (i+o < x)
	ttt |= ((*(sm->ROI+i+o) << s) | (s?(*(sm->ROI+i+o+1) >> (8-s)):0));
    }
    *(sm->ROI+i) = ttt;
  }

  for(x=((ih*iw)>>3),i=x; i>1; i--) {
    oo = 0;
    ttt = *(sm->ROI+i);
    for (j=1; j<= mes; j++) {
      oo += iw;
      s = oo & 7;
      o = oo >> 3;
      if (i-o > 0) 
  	ttt |= ((*(sm->ROI+i-o) >> s) | (s?(*(sm->ROI+i-o-1) << (8-s)):0));
    }
    *(sm->ROI+i) = ttt;
  } 

  // Horisontal enlargening
  for(i=0, x=((ih*iw)>>3); i<x; i++) {
    ttt = 0;
    t1 = *(sm->ROI+i);
    t2 = *(sm->ROI+i+1);
    for (j=1; j<= mes; j++)
      ttt |= ((t1 << j) | (t2 >> (8-j)));
    *(sm->ROI+i) |= ttt;
  }

  for(x=((ih*iw)>>3), i=x; i>0; i--) {
    ttt = 0;
    t1 = *(sm->ROI+i);
    t2 = *(sm->ROI+i-1);
    for (j=1; j<= mes; j++)
      ttt |= ((t1 >> j) | (t2 << (8-j)));
    *(sm->ROI+i) |= ttt;
  }

  return NULL;
}




