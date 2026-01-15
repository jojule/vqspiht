#ifndef SETUPMANAGER_H
#define SETUPMANAGER_H

#include <stdio.h>
#include <stdlib.h>

#define VQSPIHT_HEADER_SIZE 81

////////////////////////////////////////////////////////////////////////////
// Setup Manager class, that takes care of all the 
// configuration-files, command-line arguments, etc.
////////////////////////////////////////////////////////////////////////////

class SetupManager {

public:

  ////////////////////////
  // Public datastructures
  ////////////////////////

  // Pointter image data in row-major order
  unsigned short int *image_data; 

  // Number of bits per pixel in source image
  int image_bpp;   

  // Table of 32bit signed integer wavelet coefficients
  int *coefftable; 

  // Bitmap of ROI (Region of Interest)
  // One bit per pixel in original image
  // 0 means insignificant, 1 means significant
  unsigned char *ROI;

  // File streams
  // In compression in file is original PGM image and in out file is 
  // vqSPIHT image. 
  FILE *in;
  FILE *out;

  // Compressed file trigger parameters
  // Both alpha percentage (percentual size of general data from 
  // whole compressed image data) and c_bpp (compressed bits per
  // pixel ratio) are parsed from command-line in compression and
  // are not meaningfull in decompression.
  // In compression both alpha_trigger and filesize_trigger variables
  // are determined, and saved to vqSPIHT file header. Both the triggers
  // are in terms of bits outputted to coder, and thus can be compared
  // to nbc.
  int c_alpha_percentage;
  float c_bpp;
  int alpha_trigger;
  int filesize_trigger;

  // If user wants, that ROI Creation algorithm saves the ROI in 
  // PBM format to some file for debugging, the filename is given
  // in following variable. Otherwise variable is initialized to
  // NULL.
  char *ROI_export_filename;

  // If user wants to detect ROI with external tools and specified the
  // ROI map
  char *ROI_import_filename;
  
  // Width and Height of original image
  int image_width, image_height;

  // Width and Height of image expanded by Wavelet transform.
  // This is done to adjust original size to be more suitable
  // for Wavelet transform
  int ct_width, ct_height;

  // Wavelet parameters used by transform. They have no meaning
  // for user, but the transform need them to be saved in 
  // vqSPIHT header
  int wp_bytes;
  int wp_mean;
  int wp_shift;
  int wp_smoothing;

  // Number of levels in transformed coefficient pyramid 
  int levels;

  // Number of bits outputtted to coder in compression or 
  // number of bits got from decoder in decompression.
  int nbc;            

  // Estimated number of bytes written to disk (or in 
  // output buffer) by coder. This variable had no meaning
  // in decompression. Because of the nature of arithmetic
  // compression method used (QM-coder), the estimate can
  // be very inaccurate. This variable is used by sorting 
  // algorithm for finding the meaningfull values for 
  // both filesize_trigger and alpha_trigger.
  int outbytes;       // Total number of bytes written into outputfile

  // Direction of the algorithm (compression/decompression)
  int direction;      
#define COMPRESS 0
#define DECOMPRESS 1
  
  // If user do not want to use ROI for compression, this variable
  // is set to 1 (otherwise 0). If this variable is set, algorithm
  // should imitate the SPIHT very closely.
  int dont_use_ROI;


  // Parameters, that are used to controll u-calc detection 
  // process, and their default values
  int ucalcdet_backgroundfilterradius;
  int ucalcdet_positivefilterradius;
  int ucalcdet_negativefilterradius;
  float ucalcdet_thresholdmultiplier;
  float ucalcdet_positivefilterweight;
  int ucalcdet_morphoenlargesize;
  int ucalcdet_surroundingsclipping;
  int ucalcdet_enablecleaning;

  ////////////////////////////////////////////////////////
  // Public functions -- the interface of the SetupManager
  ////////////////////////////////////////////////////////

  // Constructor that initializes the variables
  SetupManager();  

  // Parses command-line arguments. returns NULL, if
  // ok, otherwise returns pointter to error message.
  char *parseCommandLine(int argc, char **argv);

  // Returns a pointter to array of chars containing command line help
  char *getCommandLineHelp();

  // Return 1 if user selects to use the program for compression (otherwise
  // 0)
  int directionCompress();

  // Interface for PGM handling functions. The implementation 
  // of these functions can be found in PGMFile.C
  // Load function reads the header of the PGM file from in stream,
  // that must be opened for reading and initializes image_height,
  // image_width and image_bpp variables; allocated memory for
  // image data and sets image_data pointter to point to allocated
  // memory block. Memory is allways allocated 16bits/pixed.
  // At this time, only 8 and 12 bit RAW PGM images are supported.
  // After reading the image to memory, in file is closed.
  // Save function assumes that out file stream is opened for writing.
  // Image header and data is written from the image_bpp, image_width,
  // image_height and image_data in raw PGM format, and the out file
  // stream is closed after image is written.
  // On success both functions return NULL. If error has happened
  // in file operation, a pointter to string explaining the 
  // error condition is returned.
  char *loadPGMImage(); // Load input-image (header and data)
  char *savePGMImage(); // Save input-image (header and data)


  // Interface for vqSPIHT fileformat handling functions, that 
  // are implemented in vqSPIHTFile.C
  // Load function assumes that the in stream is opened for
  // reading, and reads image_*, ct_*, wp_*, levels, *_trigger
  // and dont_use_ROI variabled from header; Initialized the
  // coder (with 7 states) and uses it to read RLE compressed
  // ROI-map from in stream to memory block that is also allocated.
  // Finally ....
  // .......
  // On success both functions return NULL. If error has happened
  // in file operation, a pointter to string explaining the 
  // error condition is returned.
  char *loadvqSPIHTImage(); // Load input-image (header and ROI)
  char *savevqSPIHTImage(); // Save input-image (header and ROI)
  void saveTriggers(); // Save the trigger-values to outputfile header


  void initCoder(int states = 1); // Initialize coder.
  int inBit(int state = 0);  // get one bit from input stream
  void outBit(int, int state = 0); // Put one bit to output stream
  int inByte();  // get one byte from input stream
  void outByte(int); // Put one byte to output stream
  int inInt();  // get one int from input stream
  void outInt(int); // Put one int to output stream
  void flushCoder(); // This has to be called, when outputting, before quitting

private:
  char *export_ROI();


  // Files
  char *in_filename;
  char *out_filename;

  // BitCoder
  unsigned char codebuffer; // Buffer 
  int size_of_codebuffer;   // mask to buffer

};

#endif






