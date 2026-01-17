
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//         G E N E R A L   P U R P O S E   F U N C T I O N S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//           > > > >    C++ version  1.08 -  02/01/96   < < < <

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
#include <ctime>
#include <cstring>

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1e6
#endif


// - - Static variable - - - - - - - - - - - - - - - - - - - - - - - - -

static Char_Line line;



// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//  Error-handling functions

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void Error(const char * s)
{
  fprintf(stderr, "\n\n\a -> Error: ");
  fputs(s, stderr);
  fputs("\n Execution terminated!\n", stderr);
  exit(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Warning(const char * s)
{
  fprintf(stderr, "\n\n\a -> Warning: ");
  fputs(s, stderr);
  Pause();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Test_Pointer(void * p, const char * orr)
{
  if (p != NULL) return;
  fputs("\n\n\a -> Error: insufficient memory.", stderr);
  if (orr != NULL) fprintf(stderr, "\nOrigin = %s\n", orr);
  fputs("\n Execution terminated!\n", stderr);
  exit(1); 
}


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//  Terminal input functions

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

char * Input_Line(const char * s, char * r)
{
  if (s != NULL) printf("\n  %s =? ", s);
  if (!fgets(line,79,stdin)) Error("line could not be read from < stdin >");
  if (r != NULL) strcpy(r, line);
  if (line[0] == '!') {
    fputs("\n Program terminated by user.\n", stderr);
    exit(0); }
  return line;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Pause(void)
{
  Input_Line("Press ENTER to continue");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int Input_Int(char * s)
{
  int a;
  for (;;) {
    if (s != NULL) printf("\n  %s =? ", s);
    Input_Line();
    if (sscanf(line, "%d", &a) == 1) break;
    fputs("Invalid data: please repeat input.", stderr); }
  return a;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

float Input_Float(char * s)
{
  float a;
  for (;;) {
    if (s != NULL) printf("\n  %s =? ", s);
    Input_Line();
    if (sscanf(line, "%f", &a) == 1) break;
    fputs("Invalid data: please repeat input.", stderr); }
  return a;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

boolean Input_Answer(char * s)
{
  for (;;) {
    printf("\n  %s? (y/n) -> ", s);
    Input_Line();
    if ((line[0] == 'y') || (line[0] == 'n')) break;
    fputs("Please answer 'y' for 'yes' or 'n' for 'no'",
      stderr); }
  return (line[0] == 'y' ? 1 : 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FILE * Open_File(char * s, char * mode)
{
  FILE * f;
  for (;;) {
    printf("\n  %s =? ", s);
    Input_Line();
    if ((f = fopen(line, mode)) != NULL) break;
    fputs("Cannot open file: repeat input or enter '!' to stop",
      stderr); }
  return f;
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//  Functions of the class < Chronometer >

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void Chronometer::start(const char * s)
{
  if (s != NULL) puts(s);
  if (stat)
    Warning("chronometer already on!");
  else {
    mark = clock();  stat = 1; }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Chronometer::stop(void)
{
  if (stat) {
    elp += clock() - mark;  stat = 0; }
  else
    Warning("chronometer already off!");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

float Chronometer::read(void)
{
  return float(stat ? elp + (clock() - mark) : elp) / CLOCKS_PER_SEC;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Chronometer::display(char * s)
{
  float sc = float(stat ? elp + (clock() - mark) : elp) / CLOCKS_PER_SEC;
  int hr = int (sc / 3600.0);  sc -= 3600.0 * hr;
  int mn = int (sc / 60.0);  sc -= 60.0 * mn;
  if (s != NULL) printf(" %s ", s);
  if (hr) {
    printf("%d hour", hr);
    if (hr > 1) printf("s, "); else printf(", "); }
  if ((hr) || (mn)) {
    printf("%d minute", mn);
    if (mn > 1) printf("s, and "); else printf(", and "); }
  printf("%5.2f seconds.\n", sc);
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// end of file < General.C >
