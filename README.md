# vqSPIHT - Variable Quality Image Compression Based on SPIHT

This repository contains the implementation and paper for vqSPIHT (Variable Quality SPIHT), a wavelet-based image compression algorithm that allows different parts of an image to be encoded at different quality levels.

## Overview

vqSPIHT extends the SPIHT (Set Partitioning In Hierarchical Trees) algorithm to support variable quality image compression. This is particularly useful for applications like digital mammography, where certain regions of interest (ROIs) must be preserved at high quality while allowing more aggressive compression elsewhere.

### Key Features

- Region of interest (ROI) based compression with configurable quality levels
- Embedded wavelet encoding allowing progressive transmission
- Automatic microcalcification detection for mammography applications
- Matrix-based implementation with reduced memory requirements compared to list-based approaches

## Code Author

J. Lehtinen (Turku Centre for Computer Science / University of Turku)

## Publication

> A. Jarvi, J. Lehtinen, O. Nevalainen. "Variable quality image compression system based on SPIHT"

The paper describes the algorithm, its implementation, and experimental results on both comic images and digital mammograms. A pre-compiled PDF is available at [paper/vqSPIHT.pdf](paper/vqSPIHT.pdf).

## Building

### Prerequisites

- C++ compiler (g++ or clang++)
- make

### Compiling the Source

```bash
./build.sh
```

Or manually:

```bash
cd src
make
```

This creates the `vqSPIHT` executable in the `src/` directory.

## Usage

```
vqSPIHT c|d [options] inputfile [outputfile]

  c|d   c for compress, d for decompress
  -a n  Set alpha-value to be n percent of the bpp (compress only)
  -b f  Set bits-per-pixel rate to be f (compress only)
  -e fn Export ROI to PBM file named fn
  -i fn Import ROI from PBM file named fn
  -d    Don't use ROI (can't be used with -e or -a)
  -c    Enable background removal in microcalcification detection
  -m p  Define microcalcification detection parameters
```

### Examples

Compress an image at 0.5 bits per pixel with 80% alpha:
```bash
./src/vqSPIHT c -b 0.5 -a 80 image.pgm compressed.vqSPIHT
```

Decompress:
```bash
./src/vqSPIHT d compressed.vqSPIHT output.pgm
```

## Building the Paper

The `paper/` directory contains the LaTeX source for the research paper.

### Prerequisites

- LaTeX distribution (TeX Live, MacTeX, etc.)
- ImageMagick (for converting figure formats)

### Compiling the Paper

```bash
cd paper
./build-paper.sh
```

This will generate `vqSPIHT.pdf`.

## Third-Party Code Attribution

This project includes code from third parties:

### Wavelet Transform (from original SPIHT)
The wavelet transform implementation in `image_bw.C`, `image_bw.h`, `general.C`, and `general.h` is from the original SPIHT authors:

> Copyright (c) 1995, 1996 Amir Said & William A. Pearlman
> University of Campinas (UNICAMP) / Rensselaer Polytechnic Institute

These files may not be redistributed without consent of the copyright holders.

Note: The vqSPIHT sorting algorithm itself (in `Compress.C`) is an independent matrix-based reimplementation, not derived from the original SPIHT source code.

### QM Arithmetic Coder
The file `qm.c` is a QM-coder implementation:

> Modified from AT&T source code by Pasi Fränti and Eugene Ageenko

The QM coder is an arithmetic coding implementation used in the JBIG and JPEG standards.

## Project Structure

```
vqspiht/
├── src/                    # Source code
│   ├── vqSPIHT.C          # Main program
│   ├── Compress.C         # Compression/decompression routines
│   ├── SetupManager.C     # Command-line parsing and configuration
│   ├── CreateROI.C        # ROI creation and microcalc detection
│   ├── Transform.C        # Wavelet transform interface
│   ├── image_bw.C         # Wavelet transform implementation (SPIHT)
│   ├── general.C          # Utility functions (SPIHT)
│   ├── qm.c               # QM arithmetic coder
│   └── Makefile
├── paper/                  # Research paper
│   ├── vqSPIHT.tex        # LaTeX source
│   ├── elsart.cls         # Elsevier article class
│   ├── *.eps, *.gif       # Figures
│   └── build-paper.sh     # Paper build script
├── build.sh               # Source build script
└── README.md
```

## License

The vqSPIHT-specific code is released under the MIT License. See [LICENSE](LICENSE) for details.

Note: The third-party components (wavelet transform and QM coder) have separate licensing terms. If you wish to use this implementation, you must obtain appropriate rights from their respective copyright holders. See the LICENSE file for details.

## References

- A. Said and W.A. Pearlman, "A new fast and efficient image codec based on set partitioning in hierarchical trees", IEEE Trans. Circuits and Systems for Video Technology, Vol. 6, June 1996, pp. 243-250.
- J.M. Shapiro, "Embedded image coding using zerotrees of wavelet coefficients", IEEE Trans. Signal Processing, Vol. 31, No. 12, December 1993.
