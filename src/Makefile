# Debugging and Optimization parameters
GCCPARAMS = -g # -O4 # -g #-g # O4 #-pg #-g 
ELECTRIC_FENCE = #-lefence

# Coder selection
#CODER = BitCoder.o
CODER = qm.o QMCoder.o 

OBJECTS = SetupManager.o Compress.o PGMFile.o vqSPIHTFile.o Transform.o \
	  CreateROI.o image_bw.o general.o $(CODER)

default: vqSPIHT 

clean:
	rm -f $(OBJECTS) vqSPIHT sqerr

# The main program
vqSPIHT: vqSPIHT.C $(OBJECTS)
	g++ -o vqSPIHT $(GCCPARAMS) vqSPIHT.C $(OBJECTS) -lm $(ELECTRIC_FENCE)

# Object that controls parameter information and globals
SetupManager.o: SetupManager.h SetupManager.C
	g++ -c $(GCCPARAMS) SetupManager.C

# Compression, Decompression and vqSPIHT sorting routines
Compress.o: Compress.h Compress.C SetupManager.h vqSPIHTFile.h
	g++ -c $(GCCPARAMS) Compress.C

# PGM Image handling
PGMFile.o: SetupManager.h PGMFile.C 
	g++ -c $(GCCPARAMS) PGMFile.C

# vqSPIHT Image header handling
vqSPIHTFile.o: SetupManager.h vqSPIHTFile.C vqSPIHTFile.h
	g++ -c $(GCCPARAMS) vqSPIHTFile.C

# Wavelet transform
Transform.o: Transform.h Transform.C SetupManager.h
	g++ -c $(GCCPARAMS) Transform.C

# ROI Map creating (ucalc-detection, etc.)
CreateROI.o: CreateROI.h CreateROI.C SetupManager.h
	g++ -c $(GCCPARAMS) CreateROI.C

# Bit-coders
BitCoder.o: BitCoder.C SetupManager.h
	g++ -c $(GCCPARAMS) BitCoder.C

QMCoder.o: QMCoder.C SetupManager.h qm.h
	g++ -c $(GCCPARAMS) QMCoder.C

# Wavelet transform algorithm from original SPIHT
image_bw.o: image_bw.h image_bw.C general.h
	g++ -c $(GCCPARAMS) image_bw.C
general.o: general.h general.C
	g++ -c $(GCCPARAMS) general.C

# QM Coder from AT&T modified by Fr�nti
qm.o: qm.h qm.c
	gcc -c $(GCCPARAMS)  qm.c
