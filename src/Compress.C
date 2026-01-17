#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;
#include "Compress.h"
#include "CreateROI.h"
#include "Transform.h"
#include "vqSPIHTFile.h"

// ========================================================
//
//  Compress Image defined by sm
//
// ========================================================
const char *Compress(SetupManager *sm) { const char *errmsg;

  // Load PGM input image
  if ((errmsg = sm->loadPGMImage()) != NULL) return errmsg;

  // Create ROI
  if (!sm->dont_use_ROI) 
    if ((errmsg = CreateROI(sm)) != NULL) return errmsg;
  
  // Do the wavelet transform and
  // release memory allocated for input image data
  if((errmsg = transformToWavelet(sm)) != NULL) return errmsg;
  free(sm->image_data);

  // Output vqSPIHT header and ROI
  if ((errmsg = sm->savevqSPIHTImage()) != NULL) return errmsg;

  // Start SPIHT sorting algorithm
  if ((errmsg = vqSPIHTsort(sm)) != NULL) return errmsg;

  // Flush coder buffers
  sm->flushCoder();

  // Save the triggers to output file header
  sm->saveTriggers();

  return NULL;
}


// ========================================================
//
//  DeCompress Image defined by sm
//
// ========================================================
const char *DeCompress(SetupManager *sm)
{
  const char *errmsg;

  // Load vqSPIHT input image Header and the ROI
  if ((errmsg = sm->loadvqSPIHTImage()) != NULL) return errmsg;

  // Allocate empty coefficient table
  sm->coefftable = (int *) malloc(sm->ct_width*sm->ct_height*4);

  // Start SPIHT sorting algorithm
  if ((errmsg = vqSPIHTsort(sm)) != NULL) return errmsg;

  // Transform reconstructed coefficient table to image data
  if((errmsg = transformFromWavelet(sm)) != NULL) return errmsg;

  // Save output PGM image
  if ((errmsg = sm->savePGMImage()) != NULL) return errmsg;

  // Flush coder buffers
  sm->flushCoder();

  return NULL;
}

// ========================================================
//
//  Does a coeffient x (index in matrix) influence ROI ?
//
// ========================================================
inline int influence(int x, SetupManager *sm)
{
  if (sm->dont_use_ROI) return 0;

  static unsigned char *RLUT = NULL;
  static int width;

  if (!RLUT) {
    // Initialize statics;
    width = sm->ct_width;
    int height = sm->ct_height;
    int i = x % width;
    int j = x / width;
    int iw = sm->image_width;
    int ih = sm->image_height;

    // Initialize LookUpTable of ROI
    int x,y,size;
    size = 1 + (height * width) >> 3;
    RLUT = (unsigned char *) malloc(sizeof(char) * size);
    unsigned char *p = RLUT;
    for(x=0; x<size; x++) *p++ = 0;
    unsigned char  *src;
    int tab1,tab2,tab3;
    int level;

    src = (unsigned char *)sm->ROI; 
    int sc = 0;
    for(level = 1; level <= sm->levels; level++) {
      tab1 = (width >> level);
      tab2 = ((height >> level) * width);
      tab3 = ((height >> level) * width) + (width >> level);
      int is = 0; 
      int id = 0;

      for(y = 0; y < ih; y+=2, is+=2*iw, id+=width) 
	for(x = 0; x < (iw >> (level-1)); x+=2) {
	  if ((*(src + ((is+sc+x)>>3)) & (0x80 >> ((is+sc+x)&7))) ||  
	      (x<(iw >> (level -1))-1 ? (*(src + ((is+sc+x+1)>>3)) & 
					 (0x80>>((is+sc+1+x)&7))) : 0) ||  
	      (y<ih-1 ?(*(src + ((is+sc+1+x+iw)>>3)) & 
			(0x80>>((is+sc+x+1+iw)&7))) ||  
	       (*(src + ((is+sc+x+iw)>>3)) & 
		(0x80>>((is+x+sc+iw)&7))) : 0)) {
	    *(RLUT + ((tab1+id+(x>>1))>>3)) |= 0x80 >> ((id+tab1+(x>>1)) & 7);
	    *(RLUT + ((tab2+id+(x>>1))>>3)) |= 0x80 >> ((id+tab2+(x>>1)) & 7);
	    *(RLUT + ((tab3+id+(x>>1))>>3)) |= 0x80 >> ((id+tab3+(x>>1)) & 7);
	  }
	}
      src = RLUT;
      sc = width >> level;
      iw = width;
      ih = height >> level;
    }
    for (y=0; y<(height>>sm->levels); y++) 
      for (x=0; x<(width>>sm->levels); x++) 
	*(RLUT+((x+width*y)>>3)) |= 0x80 >> ((x+width*y)&7);

    // DEBUG -------
    //     FILE *f = fopen("LUT.pbm","w");
    //     fprintf(f,"P4\n#\n%i %i\n",width,sm->ct_height);
    //     fwrite(RLUT,(sm->ct_width*sm->ct_height)>>3,1,f);
    //     fclose(f);
    // DEBUG -------
    
  }
  
  // return the value from RLUT. 
  return (*(RLUT + (x >> 3)) & (0x80 >> (x & 7))) ? 1 : 0;
}

// ========================================================
//
//  Is a coefficient coefficient x significant on level
//  n, where 1 is the lowest level.
//
// ========================================================
inline int s_n(int x, int n, SetupManager *sm)
{
  if ((*(sm->coefftable + x)&0x7fffffff )>= (1<<(n-1))) return 1;
  return 0;
}


// ========================================================
//
//  Is some child of the coefficient ix significant on level
//  n, where 1 is the lowest level. ix is the index in coeff
//  matrix, x is the index of the same coeff in SPLM matrix.
//  The function assumes that x is not on the lowest level of
//  the pyramid. If !SPLM, assume that type of the tree is
//  one with direct children.
//
// ========================================================
inline int Sp_n(int x, int ix, int n, SetupManager *sm, unsigned int *SPLM)
{
  int nx = x<<1;
  int nix = ix<<1;

  int i = nix%sm->ct_width;
  int j = nix/sm->ct_width;

  // If the node has direct children
  if (!(SPLM && (*(SPLM+x)&0x80000000))) 
    if (s_n(nix,n,sm) || s_n(nix+1,n,sm) || s_n(nix+sm->ct_width,n,sm) || 
	s_n(nix+sm->ct_width+1,n,sm)) return 1;
  // If the node has indirect children
  if (i < (sm->ct_width>>1) && j < (sm->ct_height>>1)) 
    return Sp_n(nx,nix,n,sm,(unsigned int *) 0) || 
      Sp_n(nx+1,nix+1,n,sm,(unsigned int *) 0) || 
      Sp_n(nx+(sm->ct_width>>1),nix+sm->ct_width,n,sm,(unsigned int *) 0) ||
      Sp_n(nx+(sm->ct_width>>1)+1,nix+sm->ct_width+1,n,sm,(unsigned int *) 0);
  return 0;
}

// ========================================================
//
//  Determines the stopping point of the algorithm
//  Assumes that alpha_trigger = filesize_trigger are
//  initialized to 0x7fffffff,
//  when compressing, and sets them to correct values. 
//  x is the index in the coefficient matrix
//
// ========================================================
int *doWeWantToProcess(int x, SetupManager *sm)
{
  return NULL;
}

// ========================================================
//
//  Main compression algorithm: The vqSPIHT
//
// ========================================================
const char *vqSPIHTsort(SetupManager *sm)
{
  unsigned int *SPLM;   // Significant Pyramid List Matrix
  unsigned int *PSM;    // Point Significance Matrix
  int decode = sm->direction == DECOMPRESS ? 1 : 0;
  unsigned int *endoftab, *coeff, *coefftab;
  coefftab = (unsigned int *) sm->coefftable;
  int height = sm->ct_height;
  int width = sm->ct_width;
  endoftab = coefftab + width * height;
  int n,i,j,x,y,prev;
  unsigned int *p;
  int eiPSM = width * height;
  int eiSPLM = (width*height)>>2;
  int outbyte_trigger;
  int second_change=0; // Hack that makes it possible to
  // check the last entry in the SPLM two times in a sorting step
  
  if (!decode) {
    sm->filesize_trigger = 0x7fffffff;
    sm->alpha_trigger = 0x7fffffff;
    outbyte_trigger = (int) (sm->c_bpp * (float)(sm->c_alpha_percentage * 
						 sm->image_width
						 * sm->image_height) /
			     800.0 ) - sizeof(vqSPIHTheader);
  } else outbyte_trigger = 0x7fffffff;
 // Convert coefftable from 2-complement
  if(decode) 
    for(coeff = coefftab; coeff < endoftab; coeff++) *coeff = 0;
  else
    for(coeff = coefftab; coeff < endoftab; coeff++) 
      *coeff = (*coeff & 1<<31) ? 
	((1<<31) | ((unsigned int) (-((int) *coeff)))) : *coeff;

  // [ 1 ] Initialization
  
  // Get n
  if (decode) n = sm->inByte();
  else {
    int i;
    for(i = 0, coeff = coefftab; coeff < endoftab; coeff++) {
      if (((*coeff) & 2147483647 ) > i) {
	i = ((*coeff) & 2147483647 );
      }
    }
    for(n=-1; i>0; i = i>>1, n++);
    n++;
    sm->outByte(n);
  }

  // Initialize matrices
  // highest bit (31) of the SLPM entry if 0 iff entry is of type A
  // (pyramid includes direct descendants of the top). bits 0-30 tell the
 // index next coefficient.
  // If index = 0, then list ends. (this impliest that coordinate
  // (0,0) cant be top of any pyramid !).
  SPLM = (unsigned int *) malloc((eiSPLM+1)*4);
  for(i=0,p=SPLM; i<=eiSPLM; i++) *p++ = 0;
  // (si,sj) points to the head of list in SPLM. Initially list is 
  // empty (it points to (0,0)) and (ei,ej) point to the last item in
  // list.
  int start,end; start = end = 0;
  // Construct list in SPLM of all the scaling coefficients that have
  // descendants and make them to be of type A

  for(i=0; i<(width>>(sm->levels)); i++) {
    for(j=0; j<(height>>(sm->levels)); j++) {
      if (i >= (width>>(sm->levels+1)) || j >= (height >> (sm->levels+1))) {
	int x = (j*(width>>1)+i);
	*(SPLM + x) = start;
	start = x;
	if(!end) end = start;
      }
    }
  }


  // PSM entry contains only two bits:
  // 00  unknown
  // 01  insignificant
  // 10  significant_prev
  // 11  significant_last
  // We initialize all the coefficients to be of unknown state
  PSM = (unsigned int *) malloc(4+(eiPSM>>2));
  for(i=0,p=PSM; i<=((eiPSM)>>4); i++) *p++ = 0;
  // Mark all scaling coefficients to be of insignificant state
  for(i=0; i<(width>>(sm->levels)); i++)
    for(j=0; j<(height>>(sm->levels)); j++)
	*(PSM + ((i+j*width)>>4)) |= (0x40000000 >> (((i+j*width)&15)<<1));


#define guess_of_16_tail_bits 0x6000
#define COMPILE_WITH_STEP3

  // [ 2 ] Sorting step

  while (n>0 && sm->filesize_trigger > sm->nbc ) {

    // For every element x in PSM having label insignificant do
    for (x=0; x<eiPSM; x++) {
      if (sm->nbc < sm->filesize_trigger && 
	  (sm->nbc < sm->alpha_trigger || influence(x,sm))) {
	if (sm->outbytes > outbyte_trigger) {
	  if (sm->alpha_trigger == 0x7fffffff) {
	    sm->alpha_trigger = sm->nbc+1;
	    outbyte_trigger = (int) 
	      (sm->c_bpp * (float)(sm->image_width* sm->image_height)/
	       8.0 ) - sizeof(vqSPIHTheader);
	  }
	  else sm->filesize_trigger = sm->nbc + 1;
	}

	switch ((*(PSM +(x>>4))>>((15-(x&15))<<1))&3) {
	case 0:
	  break;
	case 1:  // Insignificant
	  // input/output S_n(c_x)
	  if(decode) y = sm->inBit(1); 
	  else sm->outBit(y=s_n(x,n,sm),1);
	  // If S_n(c_x) =  1 then set the PSM label x to 
	  // significant_last and input/output sign of c_x.
	  if (y) {
	    *(PSM + (x>>4)) |= 0x80000000 >>((x&15)<<1);
	    if (decode) {
	      *(coefftab + x) |= (sm->inBit(2) ? 0x80000000 : 0) | 
		(1<<(n-1)) | (guess_of_16_tail_bits>>(17-n));
	    }
	    else sm->outBit(*(coefftab+x)&0x80000000 ? 1 : 0,2);
	  }
	  break;
	default:
#ifndef COMPILE_WITH_STEP3
	  if (decode) { 
	    *(coefftab+x) &= ~((1<<n)-1);
	    *(coefftab+x) |= ((sm->inBit(6))?(1<<(n-1)):0) | 
	      (guess_of_16_tail_bits >>(17-n));
	  } else {
	    sm->outBit((*(coefftab+x) & (1<<(n-1)))?1:0,6);
	  }
#else 
	  break;
#endif
	}
      }
    }

    // For each element x in the list in SPLM do
    for(second_change=0, x=start, prev = 0; x;
	prev = second_change ? prev : x, x = second_change ? x : 
	  ((!x && !prev && start) ? start :(*(SPLM + x) & 0x7fffffff)), 
	  prev = (start == x) ? 0 : prev, 
	  second_change = 0) {

      int ix = (x<<1) - x%(width>>1); // ix = x converted to index in whole table
      

      if (sm->nbc < sm->filesize_trigger && (sm->nbc < sm->alpha_trigger || influence(ix,sm))) {
	if (sm->outbytes > outbyte_trigger) {
	  if (sm->alpha_trigger == 0x7fffffff) { sm->alpha_trigger = sm->nbc+1;
	    outbyte_trigger = (int) (sm->c_bpp * (float)(sm->image_width*sm->image_height)/8.0 ) - sizeof(vqSPIHTheader);
	  }
	  else sm->filesize_trigger = sm->nbc + 1;
	}
	if (!((*(SPLM + x)) & 0x80000000)) {
	  // Sub pyramid x is of type type A
	  // input/output Sp_n(x)
	  if(decode) y = sm->inBit(3); 
	  else sm->outBit(y=Sp_n(x,ix,n,sm,SPLM),3);
	  // if Sp_n(x) = 1 then
	  if (y) {
	    // For each i belonging in immediate offspring of x do
	    for(j=0, i=(ix<<1); j<4; j++, i=j==1?i+1:(j==2?i+width:i-1)) {
	      if (sm->nbc < sm->filesize_trigger && (sm->nbc < sm->alpha_trigger || influence(i,sm))) {
		if (sm->outbytes > outbyte_trigger) {
		  if (sm->alpha_trigger == 0x7fffffff) {
		    sm->alpha_trigger = sm->nbc+1;
		    outbyte_trigger = (int) (sm->c_bpp * 
					     (float)(sm->image_width*sm->image_height)/8.0 ) - sizeof(vqSPIHTheader);
		  } else sm->filesize_trigger = sm->nbc + 1;
		}
		// input/output S_n(c_i)
		if(decode) y = sm->inBit(4); 
		else sm->outBit(y=s_n(i,n,sm),4);
		// if S_n(c_i) = 1 then 
		if (y) {
		  // Set PSM label i to significant_last
		  // and input/output thr sign of c_i
		  *(PSM + (i>>4)) |= 0x80000000 >>((i&15)<<1);
		  if (decode) {
		    *(coefftab + i) |= (sm->inBit(2) ? 0x80000000 : 0) | 
		      (1<<(n-1)) | (guess_of_16_tail_bits>>(17-n));
		  }
		  else sm->outBit(*(coefftab+i)&0x80000000 ? 1 : 0,2);
		} else {
		  // else set the PSM label i to insignificant
		  *(PSM + (i>>4)) |= 0x40000000 >>((i&15)<<1);
		}
	      }
	    }
	    // If is not on one of the lowest levels of pyramid, then
	    if ((ix/width)<(height>>2) && (ix%width)<(width>>2)) {
	      // move x to the end of the list in SPLM
	      if (start == x) {
		start = (*(SPLM + x) & 0x7fffffff);
		if (end == x) end = 0;
	      } else {
		*(SPLM + prev) = (*(SPLM + prev) & 0x80000000) | 
		  (*(SPLM + x) & 0x7fffffff);
		if (end == x) end = prev;
	      }
	      if (end) {
		*(SPLM + end) = (*(SPLM + end) & 0x80000000) | x;
		// And set it to be of type B
		*(SPLM + x) = 0x80000000;
		if (!start) start = x;
		end = x;
		x = prev;
	      } else {
		// if there is no end, there should be no start
		if (start) cerr << "end of SPLM found when there is no end !!\n";
		start = end = x;
		*(SPLM + x) = 0x80000000;
		// We want to give a second change to the x as a type B entry.
		second_change = 1;
	      }
	    } else {
	      // remove x from SLPM
	      if (start == x) {
		start = (*(SPLM + x) & 0x7fffffff);
		if (end == x) end = 0;
	      } else {
		*(SPLM + prev) = (*(SPLM + prev) & 0x80000000) | 
		  (*(SPLM + x) & 0x7fffffff);
		if (end == x) end = prev;
		x = prev;
	      }
	    }
	  }
	} else {
	  // Sub pyramid x is of type type B
	  // input/output Sp_n(x)
	  if(decode) y = sm->inBit(5); 
	  else sm->outBit(y=Sp_n(x,ix,n,sm,SPLM),5);
	  // if Sp_n(x) = 1 then
	  if (y) {
	    // For each i belonging in immediate offspring of x do
	    int unmod_ix = ix;
	    for(j=0, i=(x<<1), ix=(ix<<1); j<4; j++, 
		  i=(j==1)?i+1:(j==2?i+(width>>1):i-1),
		  ix=(j==1)?ix+1:(j==2?ix+(width>>1):ix-1)) {
	      // add i
	      if (sm->nbc < sm->filesize_trigger && (sm->nbc < sm->alpha_trigger || 
					 influence(unmod_ix,sm))) {
		if (sm->outbytes > outbyte_trigger) {
		  if (sm->alpha_trigger == 0x7fffffff) {
		    sm->alpha_trigger = sm->nbc+1;
		    outbyte_trigger = (int) (sm->c_bpp * 
					     (float)(sm->image_width*sm->image_height)/8.0 ) - sizeof(vqSPIHTheader);
		  }
		  else sm->filesize_trigger = sm->nbc + 1;
		}
		if (i%(width>>1)>=(width>>sm->levels) ||
		    i/(width>>1)>=(height>>sm->levels)) {
		  *(SPLM + i) = 0;
		  *(SPLM + end) = (*(SPLM +end) & 0x80000000) | i;
		  end = i;
		}
	      }
	    }
	    // remove x from SLPM
	    if (start == x) {
	      start = (*(SPLM + x) & 0x7fffffff);
	      if (end == x) end = 0;
	    } else {
	      *(SPLM + prev) = (*(SPLM + prev) & 0x80000000) | 
		(*(SPLM + x) & 0x7fffffff);
	      if (end == x) end = prev;
	      x = prev;
	    }
	  }
	}
      }
    }
    

    // [ 3 ] Refinement Step
#ifdef COMPILE_WITH_STEP3
    for(x=0;x<eiPSM;x++) {
      i = ((*(PSM + (x>>4)))>>(30-((x&15)<<1)))&3;
      if (i==2) {
	if (sm->nbc < sm->filesize_trigger && (sm->nbc < sm->alpha_trigger || influence(x,sm))) {
	  if (sm->outbytes > outbyte_trigger) {
	    if (sm->alpha_trigger == 0x7fffffff) {
	      sm->alpha_trigger = sm->nbc+1;
	      outbyte_trigger = (int) (sm->c_bpp * (float)(sm->image_width*sm->image_height)/8.0 ) - sizeof(vqSPIHTheader);
	    }
	    else sm->filesize_trigger = sm->nbc + 1;
	  }
	  if (decode) { 
	    *(coefftab+x) &= ~((1<<n)-1);
	    *(coefftab+x) |= ((sm->inBit(6))?(1<<(n-1)):0) | 
	      (guess_of_16_tail_bits >>(17-n));
	  } else 
	    sm->outBit((*(coefftab+x) & (1<<(n-1)))?1:0,6);
	} else {
	  // Set i to be of unknown state to disable further refinement 
	  // step tests
	  *(PSM+(x>>4)) &= ~(0xc0000000 >> ((x&15)<<1));
	}
      } else if (i==3) 
	*(PSM+(x>>4)) &= ~(0x40000000 >> ((x&15)<<1));
    }
#endif

    // [ 4 ] Quantization-step update
    n--;
  }

 // Convert coefftable from 2-complement
  if(decode) 
    for(coeff = coefftab; coeff < endoftab; coeff++) 
      *coeff = (*coeff & 1<<31) ? -(int)(0x7FFFFFFF & *coeff) : *coeff;

  return NULL;
}


      



