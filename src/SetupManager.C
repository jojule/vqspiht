#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;
#include "SetupManager.h"

SetupManager::SetupManager()
{
  // Set the defaults
  image_bpp = 8;
  direction = COMPRESS;
  c_alpha_percentage = 100;
  c_bpp = 1;
  image_width = image_height = 0;
  ct_width = ct_height = 0;
  in = out = (FILE *) NULL;
  in_filename = out_filename = NULL;
  image_data = NULL; 
  coefftable = NULL; 
  ROI = NULL;   
  ROI_export_filename = NULL;
  ROI_import_filename = NULL;
  dont_use_ROI = 0;
  
  // Initialize the microcalc-detection parameters
  ucalcdet_backgroundfilterradius    = 7;
  ucalcdet_positivefilterradius      = 2;
  ucalcdet_negativefilterradius      = 15;
  ucalcdet_thresholdmultiplier     = 2.5;
  ucalcdet_positivefilterweight    = 0.8;
  ucalcdet_morphoenlargesize         = 3;
  ucalcdet_surroundingsclipping      = 0;
  ucalcdet_enablecleaning            = 0;
}

char *SetupManager::parseCommandLine(int argc, char **argv)
{
  if (argc < 3) return "Too few parameters";
  if (*(argv[1]+1)) return "First parameter is invalid";
  else switch (*argv[1]) {
  case 'c' : direction = COMPRESS; break;
  case 'd' : direction = DECOMPRESS; break;
  default: return "First parameter is invalid";
  }
  
  int reading_options=1;
  for (int i = 2; i < argc; i++) {
    if (reading_options && *argv[i] == '-') {
      switch (*(argv[i]+1)) {
      case 'a' : // Set alpha
	if (dont_use_ROI) return "-d can't be used with -a";
	i++; sscanf(argv[i], "%i", &c_alpha_percentage);
	if (c_alpha_percentage < 0 || c_alpha_percentage > 100) 
	  return "Invalid -a parameter value";
	if (direction == DECOMPRESS) return "-a is valid only when compressing";
	break;
      case 'b' : // Set bpp
	if (direction == DECOMPRESS) return "-b is valid only when compressing";
	i++; sscanf(argv[i], "%f", &c_bpp);
	if (c_bpp < 0.0 || c_bpp > 32.0) return "Invalid -b parameter value";
	break;
      case 'd' : // Don't use ROI
	if (direction== DECOMPRESS) return "-d is valid only when compressing";
	if (ROI_export_filename) return "-d can't be used with -e";
	if (ROI_import_filename) return "-d can't be used with -i";
	if (c_alpha_percentage != 100) return "-d can't be used with -a";
	dont_use_ROI = 1;
	break;
      case 'e' : // Export ROI to fn
	if (dont_use_ROI) return "-d can't be used with -e";
	i++; ROI_export_filename = strdup(argv[i]);
	break;
      case 'i' : // Import ROI from fn
	if (dont_use_ROI) return "-d can't be used with -i";
	i++; ROI_import_filename = strdup(argv[i]);
	break;
      case 'c' : // Enable cleaning
	if (dont_use_ROI) return "-c can't be used with -c";
	if (direction == DECOMPRESS) return "-c is valid only when compressing";
	ucalcdet_enablecleaning = 1;
	break;
      case 'm' : // Microcalc detection parameters
	if (dont_use_ROI) return "-d can't be used with -m";
	i++; sscanf(argv[i], "%i,%i,%i, %f,%f,%i,%i", 
		    &ucalcdet_backgroundfilterradius,
		    &ucalcdet_positivefilterradius,
		    &ucalcdet_negativefilterradius,
		    &ucalcdet_thresholdmultiplier,
		    &ucalcdet_positivefilterweight,
		    &ucalcdet_morphoenlargesize,
		    &ucalcdet_surroundingsclipping);
#if (0)
	/// Debug
	cerr << "ucalcdet_backgroundfilterradius = " << ucalcdet_backgroundfilterradius << "\n";
	cerr << "ucalcdet_positivefilterradius = " << ucalcdet_positivefilterradius << "\n";
	cerr << "ucalcdet_negativefilterradius = " << ucalcdet_negativefilterradius << "\n";
	cerr << "ucalcdet_thresholdmultiplier = " << ucalcdet_thresholdmultiplier << "\n";
	cerr << "ucalcdet_positivefilterweight = " << ucalcdet_positivefilterweight << "\n";
	cerr << "ucalcdet_morphoenlargesize = " << ucalcdet_morphoenlargesize << "\n";
	cerr << "ucalcdet_surroundingsclipping = " << ucalcdet_surroundingsclipping << "\n";
	///
#endif
	if (ucalcdet_backgroundfilterradius < 0 ||
	    ucalcdet_positivefilterradius < 0 ||
	    ucalcdet_negativefilterradius < 0)
	  return "Invalid filter radius."
	    " All the radius values must non-negative";
	if (ucalcdet_backgroundfilterradius > 100 ||
	    ucalcdet_positivefilterradius > 100 ||
	    ucalcdet_negativefilterradius > 100)
	  return "Unreasonable large filter radius";
	if (ucalcdet_morphoenlargesize < 0 ||
	    ucalcdet_morphoenlargesize > 31) 
	  return "Invalid mes parameter. mes must be between 0 and 31";
	if (ucalcdet_positivefilterweight <= 0 ||
	    ucalcdet_positivefilterweight >= 1)
	  return "pfw should be positive, but smaller than 1";
	if (ucalcdet_thresholdmultiplier < 0)
	  return "tm should be positive";
	if (ucalcdet_thresholdmultiplier > 50)
	  return "tm value over 50 seems unreasonable big";
	if (ucalcdet_surroundingsclipping < 0)
	  return "clipping value must be non-negative";

	if (direction == DECOMPRESS) return "-m is valid only when compressing";
	break;
      default: reading_options = 0;
      }
    } else reading_options = 0;
    
    if(!reading_options) {
      if (i<argc-2) return "Invalid number of parameters";
      if (!in_filename)	in_filename = strdup(argv[i]);
      else out_filename = strdup(argv[i]);
    }
  }

  if (!in_filename) return "Input filename is not specified";
  else if (strcmp("-",in_filename)) {
    if (!(in =fopen(in_filename,"rb")))
      return "Can't open input-file";
  } else {
    in = stdin;
  }

  // If output filename is not specified, create it
  if (!out_filename && !strcmp(in_filename,"-")) out_filename = strdup("-");
  if (!out_filename) {
    char *str = strdup(in_filename);
    for (int i = strlen(in_filename)-1; i>1; i--)
      if (str[i] == '.') { 
	str[i] = '\0'; 
	i = -1; 
      }
    out_filename = (char *) malloc(strlen(in_filename)+15);
    if (direction == COMPRESS) sprintf(out_filename,"%s.vqSPIHT",str);
    free(str);
  }

  if (strcmp(out_filename,"-")) {
    if (!(out = fopen(out_filename,"wb"))) 
      return "Can't open output-file";
  } else {
    out = stdout;
  }

  return NULL;
}

char *SetupManager::getCommandLineHelp()
{
  return 
    "Usage: \n"
    "  vqSPIHT c|d [options] inputfile [outputfile]\n\n"
    "  c|d   c for compress, d for decompress\n"
    "  -a n  Set alpha-value to be n percent of the bpp (compress only)\n"
    "  -b f  Set bits-per-pixel rate to be f (a standard floating point \n"
    "        number) (compress only)\n"
    "  -e fn Export ROI to PBM file named fn.(and do not compress the image)\n"
    "  -i fn Import ROI from PBM file named fn.\n"
    "  -d    Dont use ROI (can't be used with -e or -a) (Compress only)\n"
    "  -c    Enable the removal of the background in the microcalcification\n"
    "        detection process \n"
    "  -m p  Define microcalc detection parameters (Compress only)\n"
    "         p is defined as follows (without any spaces !): \n"
    "         f1r,pfr,nfr,tm,pfw,mes,scl  where the parameters are (default)(type)\n"
    "         f1r = radius of first gaussian filter used (7)(int)\n"
    "         pfr = positive gaussian filter radius (2)(int)\n"
    "         nfr = negative gaussian filter radius (15)(int)\n"
    "         tm  = multiplier of std.deviation used for treshold (2.5)(float)\n"
    "         pfw = positive gaussian filter weight (0.8)(float)\n"
    "         mes = 'morphological' enlargening size (3)(int)\n"
    "               (use 0 for no enlargening)\n"
    "         scl = surroundings clipping treshold value (0)\n"
    "               Use 0, if you want to save the surroundings\n"
    "         All the filter-radius parameters must be in range [0-31]\n"
    "         For more information about these parameters, refer the article:\n"
    "         Segmentation of Microcalcifications in Mammograms\n"
    "         Joachim Dengler, Sabine Behrens, and Johann Friedrich Desaga\n"
    "\n Note, that - can be used to read from stdin or write to stdout\n";
  
}

int SetupManager::directionCompress()
{
  return (direction == COMPRESS);
}

