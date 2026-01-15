
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                     I M A G E   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//           > > > >    C++ version 11.06 -  02/01/96   < < < <

// Amir Said - amir@densis.fee.unicamp.br
// University of Campinas (UNICAMP)
// Campinas, SP 13081, Brazil

// William A. Pearlman - pearlman@ecse.rpi.edu
// Rensselaer Polytechnic Institute
// Troy, NY 12180, USA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Copyright (c) 1995, 1996 Amir Said & William A. Pearlman

// This program is Copyright (c) by Amir Said & William A. Pearlman.
// It may not be redistributed without the consent of the copyright
// holders. In no circumstances may the copyright notice be removed.
// The program may not be sold for profit nor may they be incorporated
// in commercial programs without the written permission of the copyright
// holders. This program is provided as is, without any express or
// implied warranty, without even the warranty of fitness for a
// particular purpose.


// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "general.h"
#include "image_bw.h"
#include <cstring>


// - - Constants - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static char * M_MSG = "< Image_SP >";

static char * R_MSG = "< Image_BW > cannot read from file";

static char * W_MSG = "< Image_BW > cannot write to file";

static char * L_MSG = "< Image_BW > larger than specified dimension";


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//  Auxiliary functions

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

#ifdef LOSSLESS

static void SP_Transform(int m, int in[], int l[], int h[])
{
  int i, k, d1, d2, mm = m - 1;

  for (i = k = 0; i < m; i++, k += 2) {
    l[i] = (in[k] + in[k+1]) >> 1;
    h[i] =  in[k] - in[k+1]; }

  h[0] -= (d2 = l[0] - l[1]) >> 2;
  for (i = 1; i < mm; i++) {
    d1 = d2;  d2 = l[i] - l[i+1];
    h[i] -= (((d1 + d2 - h[i+1]) << 1) + d2 + 3) >> 3; }
  h[i] -= d2 >> 2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SP_Recover(int m, int l[], int h[], int out[])
{
  int i, k, d1, d2, t;

  t = (h[m-1] += (d1 = l[m-2] - l[m-1]) >> 2);
  for (i = m - 2; i > 0; i--) {
    d2 = d1;  d1 = l[i-1] - l[i];
    t = (h[i] += (((d1 + d2 - t) << 1) + d2 + 3) >> 3); }
  h[0] += d1 >> 2;

  for (i = k = 0; i < m; i++, k += 2) {
    out[k] = l[i] + ((h[i] + 1) >> 1);
    out[k+1] = out[k] - h[i]; }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#else

static const float SmoothingFactor = 0.8;

static const int NumbTap = 4;

static const float T_LowPass[5] =
  { 0.852699,  0.377403, -0.110624, -0.023849, 0.037829 };

static const float T_HighPass[5] =
  { 0.788485, -0.418092, -0.040690,  0.064539, 0.0 };

static const float R_LowPass[5] =
  { 0.852699,  0.418092, -0.110624, -0.064539, 0.037829 };

static const float R_HighPass[5] =
  { 0.788485, -0.377403, -0.040690,  0.023849, 0.0 };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline float Filter_L(const float * f, float * v)
{
  return f[0] * v[0] +
    f[1] * (v[1] + v[-1]) + f[2] * (v[2] + v[-2]) +
    f[3] * (v[3] + v[-3]) + f[4] * (v[4] + v[-4]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline float Filter_H(const float * f, float * v)
{
  return f[0] * v[0] +
    f[1] * (v[1] + v[-1]) + f[2] * (v[2] + v[-2]) +
    f[3] * (v[3] + v[-3]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Reflection(float * h, float * t)
{  
  for (int i = 1; i <= NumbTap; i++) {
    h[-i] = h[i];  t[i] = t[-i]; }
}

#endif

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//  Member functions of the class  < Image_BW >

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Private functions - - - - - - - - - - - - - - - - - - - - - - - -

int Image_BW::max_levels(int n)
{
  int l1, l2;
  for (l1 = 0; !(n & 1); l1++) n >>= 1;
#ifdef LOSSLESS
  for (l2 = l1 - 3; n; l2++) n >>= 1;
#else
  for (l2 = l1 - 4; n; l2++) n >>= 1;
#endif
  return (l1 < l2 ? l1 : l2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::assign_mem(Image_Coord d, int b)
{
  if ((b < 1) || (b > 2)) Error("Invalid number of < Image_BW > bytes");
  if ((levels >= 0) && (dim.x == d.x) && (dim.y == d.y)) return;
  free_mem();
  if ((d.x < 64) || (d.y < 64))
    Error("< Image_BW > dimension is too small or negative");
  dim = d;  
  pdim.x = (d.x < 256 ? (d.x + 7) & 0x3FF8 : (d.x + 15) & 0x3FF0); 
  pdim.y = (d.y < 256 ? (d.y + 7) & 0x3FF8 : (d.y + 15) & 0x3FF0); 

  NEW_VECTOR(coeff, pdim.x, Pel_Type *, M_MSG);
  for (int i = 0; i < pdim.x; i++) {
    NEW_VECTOR(coeff[i], pdim.y, Pel_Type, M_MSG); }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::free_mem(void)
{
  if (levels >= 0) {
    for (int i = pdim.x - 1; i >= 0; i--) delete [] coeff[i];
    delete [] coeff; }
  bytes = dim.x = dim.y = 0;  levels = -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::extend(void)
{
  int i, j;
  for (j = dim.y - 1; j < pdim.y - 1; j++) {
    coeff[0][j+1] = (coeff[0][j] + coeff[1][j]) / 2;
    coeff[dim.x-1][j+1] = (coeff[dim.x-1][j] + coeff[dim.x-2][j]) / 2;
    for (i = dim.x - 2; i > 0; i--)
      coeff[i][j+1] = (coeff[i-1][j] + coeff[i][j] + coeff[i+1][j]) / 3; }
  for (i = dim.x - 1; i < pdim.x - 1; i++) {
    coeff[i+1][0] = (coeff[i][0] + coeff[i][1]) / 2;
    coeff[i+1][pdim.y-1] = (coeff[i][pdim.y-1] + coeff[i][pdim.y-2]) / 2;
    for (j = pdim.y - 2; j > 0; j--)
      coeff[i+1][j] = (coeff[i][j-1] + coeff[i][j] + coeff[i][j+1]) / 3; }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - Public functions  - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::read_pic(Image_Coord d, char * file_name, int b)
{
  assign_mem(d, b);
  mean = levels = 0;  bytes = b;

  FILE * in_file = fopen(file_name, "rb");
  if (in_file == NULL) Error(R_MSG);

  int i, j, k, p, c;
  for (i = 0; i < dim.x; i++)
    for (j = 0; j < dim.y; j++) {
      for (p = k = 0; k < bytes; k++) {
        if ((c = getc(in_file)) == EOF) Error(R_MSG);
        p = (p << 8) | c; }
      coeff[i][j] = p; }
  if (getc(in_file) != EOF) Error(L_MSG);
  fclose(in_file);

  extend();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

float Image_BW::compare(char * file_name)
{
  if (levels) Error("cannot compare < Image_BW >");

  FILE * in_file = fopen(file_name, "rb");
  if (in_file == NULL) Error(R_MSG);

  double mse = 0.0;
  int i, j, k, p, c, t;
  for (i = 0; i < dim.x; i++)
    for (j = 0; j < dim.y; j++) {
#ifdef LOSSLESS
      t = coeff[i][j];
#else
      t = int(floor(0.499 + coeff[i][j]));
#endif
      if (t < 0) t = 0;
      if ((bytes == 1) && (t > 255)) t = 255;
      for (p = k = 0; k < bytes; k++) {
        if ((c = getc(in_file)) == EOF) Error(R_MSG);
        p = (p << 8) | c; }
      mse += Sqr(p - t); }
  if (getc(in_file) != EOF) Error(L_MSG);
  fclose(in_file);

  return (mse / dim.x) / dim.y;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::write_pic(char * file_name)
{
  if (levels) Error("cannot write < Image_BW >");

  FILE * out_file = fopen(file_name, "wb");
  if (out_file == NULL) Error(W_MSG);

  int i, j, k;
  for (i = 0; i < dim.x; i++)
    for (j = 0; j < dim.y; j++) {
#ifdef LOSSLESS
      k = coeff[i][j];
#else
      k = int(floor(0.499 + coeff[i][j]));
#endif
      if (k < 0) k = 0;
      if (bytes == 2) {
        if (putc(k >> 8, out_file) == EOF) Error(W_MSG); }
      else {
        if (k > 255) k = 255; }
      if (putc(k & 0xFF, out_file) == EOF) Error(W_MSG); }

  fclose(out_file);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::reset(Image_Coord d)
{
  assign_mem(d, 1);
  bytes = 1;  mean = shift = smoothing = 0;
  levels = Min(max_levels(pdim.x), max_levels(pdim.y));

  int i, j;
  for (i = 0; i < pdim.x; i++)
    for (j = 0; j < pdim.y; j++) coeff[i][j] = 0;
}
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::reset(Image_Coord d,int byt)
{
  assign_mem(d, byt);
  bytes = byt; mean = levels = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::reset(Image_Coord d, int b, int m, int shf, int smt)
{
  reset(d);
  bytes = b;  mean = m;  shift = shf;  smoothing = smt;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::transform(int sm_numb)
{
  if (levels) Error("cannot transform < Image_BW >");
  if ((dim.x < 32) || (dim.y < 32))
    Error("< Image_BW > is too small to transform");

#ifdef DEBUGMSG
  Chronometer cpu_time;
  cpu_time.start("\n  Starting image transformation...");
#endif

  levels = Min(max_levels(pdim.x), max_levels(pdim.y));
  if (levels < 3) Error("invalid < Image_BW > dimension");

  smoothing = sm_numb;
  if ((sm_numb < 0) || (sm_numb > 7))
    Error("invalid < Image_BW > smoothing factor");

#ifdef LOSSLESS
  int i = 0, j = Max(pdim.x, pdim.y), k = j << 1;
#else
  int i = NumbTap, j = 0, k = Max(pdim.x, pdim.y) + (i << 1);
  float sm_mt;
#endif
  CREATE_VECTOR(temp_line, k, Pel_Type, M_MSG);
  Pel_Type * t, * in_line = temp_line + i, * out_line = in_line + j;

// hierarchical wavelet or S+P transformation

  int lv, nx, ny, mx = pdim.x, my = pdim.y;
  for (lv = 0; lv < levels; lv++) {

  // shifts are halved, multiplier is updated

    nx = mx;  mx >>= 1;  ny = my;  my >>= 1;

#ifndef LOSSLESS
  float sm_mt = 1 + smoothing * SmoothingFactor / (2 + lv * lv);
#endif
  // transformation of columns

    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) in_line[i] = coeff[i][j];
#ifdef LOSSLESS
      SP_Transform(mx, in_line, out_line, out_line + mx);
      for (i = 0; i < nx; i++) coeff[i][j] = out_line[i]; }
#else
      Reflection(in_line, in_line + nx - 1);
      for (i = 0, t = in_line; i < mx; i++) {
        coeff[i][j] = sm_mt * Filter_L(T_LowPass, t++);
        coeff[i+mx][j] = Filter_H(T_HighPass, t++); } }
#endif

  // transformation of rows

    for (i = 0; i < nx; i++) {
      memcpy(in_line, coeff[i], ny * sizeof(Pel_Type));
#ifdef LOSSLESS
      SP_Transform(my, in_line, coeff[i], coeff[i] + my); } }
#else
      Reflection(in_line, in_line + ny - 1);
      for (j = 0, t = in_line; j < my; j++) {
        coeff[i][j] = sm_mt * Filter_L(T_LowPass, t++);
        coeff[i][j+my] = Filter_H(T_HighPass, t++); } } }
#endif

// calculate and subtract mean

  float s = 0;
  for (i = 0; i < mx; i++)
    for (j = 0; j < my; j++) s += coeff[i][j];
  s /= float(mx) * float(my);

  for (shift = 0; s > 1e3; shift++) s *= 0.25;
  mean = int(0.5 + s);

#ifdef LOSSLESS
  int tm = mean << (shift + shift);
#else
  float tm = mean * pow(4, shift);
#endif
  for (i = 0; i < mx; i++)
    for (j = 0; j < my; j++) coeff[i][j] -= tm;

  delete [] temp_line;
#ifdef DEBUGMSG
  cpu_time.display(" Image transformed in");
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Image_BW::recover(void)
{
  if (levels <= 0) Error("cannot recover < Image_BW >");

#ifdef DEBUGMSG
  Chronometer cpu_time;
  cpu_time.start("\n  Starting inverse transformation...");
#endif

#ifdef LOSSLESS
  int i = 0, j = Max(pdim.x, pdim.y), k = j << 1;
#else
  int i = NumbTap, j = 0, k = Max(pdim.x, pdim.y) + (i << 1);
  float sm_mt;
#endif
  CREATE_VECTOR(temp_line, k, Pel_Type, M_MSG);
  Pel_Type * t, * in_line = temp_line + i, * out_line = in_line + j;

  int lv, mx, my, nx = pdim.x >> levels, ny = pdim.y >> levels;

// add mean

#ifdef LOSSLESS
  int tm = mean << (shift + shift);
#else
  float tm = mean * pow(4, shift);
#endif
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++) coeff[i][j] += tm;

// inverse hierarchical wavelet or S+P transformation

  for (lv = levels - 1; lv >= 0; lv--) {

  // shifts are doubled, multiplier is updated

    mx = nx;  nx <<= 1;  my = ny;  ny <<= 1;

#ifndef LOSSLESS
  float sm_mt = 1 / (1 + smoothing * SmoothingFactor / (2 + lv * lv));
#endif

  // inverse transformation of rows

    for (i = 0; i < nx; i++) {
#ifdef LOSSLESS
      memcpy(in_line, coeff[i], ny * sizeof(Pel_Type));
      SP_Recover(my, in_line, in_line + my, coeff[i]); }
#else
      for (j = 0, t = in_line; j < my; j++) {
        *(t++) = sm_mt * coeff[i][j];  *(t++) = coeff[i][j+my]; }
      Reflection(in_line, in_line + ny - 1);
      for (j = 0, t = in_line; j < ny;) {
        coeff[i][j++] = Filter_H(R_HighPass, t++);
        coeff[i][j++] = Filter_L(R_LowPass, t++); } }
#endif

  // inverse transformation of columns

    for (j = 0; j < ny; j++) {
#ifdef LOSSLESS
      for (i = 0; i < nx; i++) in_line[i] = coeff[i][j];
      SP_Recover(mx, in_line, in_line + mx, out_line);
      for (i = 0; i < nx; i++) coeff[i][j] = out_line[i]; } }
#else
      for (i = 0, t = in_line; i < mx; i++) {
        *(t++) = sm_mt * coeff[i][j];  *(t++) = coeff[i+mx][j]; }
      Reflection(in_line, in_line + nx - 1);
      for (i = 0, t = in_line; i < nx;) {
        coeff[i++][j] = Filter_H(R_HighPass, t++);
        coeff[i++][j] = Filter_L(R_LowPass, t++); } } }
#endif

  levels = 0;  delete [] temp_line;
#ifdef DEBUGMSG
  cpu_time.display(" Image transformed in");
#endif
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// end of file  < Image_BW.C >
