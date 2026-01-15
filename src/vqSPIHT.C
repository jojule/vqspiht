#include <cstdlib>
#include <iostream>
using namespace std;
#include "SetupManager.h"
#include "Compress.h"

char *architectureCheck()
{
  int i;
  unsigned short int usi;

  i = 70000;
  if (i<50000) return "Not at least 32bit integers available\n";

  usi = (unsigned short int) 70000;
  if (usi != 4464) return "unsigned short int is not 16bit variable\n";

  return NULL;
}


int main(int argc, char **argv)
{
  char *errmsg;
  SetupManager *sm = new SetupManager();

  // Check hardware architecture is suitable for this implementation
  if (architectureCheck()) {
    printf("%s\n",architectureCheck());
    exit(1);
  }

  // Parse commandline arguments
  if ((errmsg = sm->parseCommandLine(argc,argv)) != NULL) {
    cerr << "Parsing commandline arguments:\n" << errmsg << "\n";
    cerr << sm->getCommandLineHelp();
    exit(1);
  }

  // Compress / Decompress
  if (sm->directionCompress()) {
    if ((errmsg = Compress(sm)) != NULL) 
      cerr << "Compressing: \n" << errmsg << "\n"; 
  } else {
    if ((errmsg = DeCompress(sm)) != NULL) 
      cerr << "Decompressing: \n" << errmsg << "\n";
  }

    return 0;
}

