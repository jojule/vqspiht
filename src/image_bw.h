
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                     I M A G E   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//           > > > >    C++ version 11.04 -  02/01/96   < < < <

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


// - - External definitions  - - - - - - - - - - - - - - - - - - - - - -

#ifdef LOSSLESS

typedef int Pel_Type;
#define ABS abs

#else

typedef float Pel_Type;
#define ABS fabs

#endif

struct Image_Coord { int x, y; };


// - - Class definition  - - - - - - - - - - - - - - - - - - - - - - - -

class Image_BW
{

protected:
  // . private data .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .

    Image_Coord dim, pdim;

    int levels, bytes, mean, shift, smoothing;

    Pel_Type ** coeff;

  // . private functions  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .

    int max_levels(int);

    void assign_mem(Image_Coord, int);

    void free_mem(void);

  // . constructor and destructor  .  .  .  .  .  .  .  .  .  .  .  .  .

  public :

    void extend(void);

    Image_BW(void) { levels = -1; }

    ~Image_BW(void) { free_mem(); }

  // . public functions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .

    Pel_Type & operator[](const Image_Coord & c) {
      return coeff[c.x][c.y]; }

    Pel_Type & operator()(int i, int j) {
      return coeff[i][j]; }

    Pel_Type * address(const Image_Coord & c) {
      return coeff[c.x] + c.y; }

    Image_Coord dimension(void) { return dim; }

    Image_Coord pyramid_dim(void) { return pdim; }

    int transform_mean(void) { return mean; }

    int mean_shift(void) { return shift; }

    int smoothing_factor(void) { return smoothing; }

    int pyramid_levels(void) { return levels; }

    int pixel_bytes(void) { return bytes; }

    float compare(char * file_name);

    void read_pic(Image_Coord, char * file_name, int nbytes = 1);

    void write_pic(char * file_name);

    void reset(Image_Coord);

    void reset(Image_Coord,int);

    void reset(Image_Coord, int nbytes, int m, int mshift,
               int smoothing_factor = 0);

    void transform(int smoothing_factor = 0);

    void recover(void);

    void dispose(void) { free_mem(); }

};  // end definition of class  < Image_BW >

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// end of file  < Image_BW.H >
