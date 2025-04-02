/*******************************************************************************
* File: datam.h
*
* Purpose: Header file for datam library functions.
*
* Notes:
*   1. Compile library functions:
*
*        gcc -c blurb.c grammer.c what.c winfo.c
*        ar -cru libdatam.a blurb.o grammer.o what.o winfo.o
*        ranlib libdatam.a
*
*   2. Compile using the library:  gcc -o bin/dmp dmp.c -L. -ldatam
*
*   3. Support for the 'what' command is provided via the arcane string that's
*      assigned to the 'What' variable.  The "@(#)" part is what 'what' detects
*      to produce its output (which consists of the rest of this string).  The
*      venerable convention was to provide the source filename and its version
*      and date, separated by spaces, in this string.  Example 'what' usage:
*
*        % which dmp
*        /Users/mallen/bin/dmp
*
*        % what /Users/mallen/bin/dmp
*        /Users/mallen/bin/dmp
*                dmp.c v0.10 08/23/2021 DataM
*                blurb.c v0.2 08/23/2021 DataM
*                grammer.c v0.1 08/23/2021 DataM
*
*   4. Referencing the 'What' variable prevents the compiler from optimizing
*      the 'What' string out of the binary file.  This reference provides no
*      functional logic to the program.
*
* History:
*         ___date___  _______________________description________________________
*         08/23/2021  original version
*         09/01/2021  added 'what'
*
*******************************************************************************/

#ifndef DATAM_H
#define DATAM_H

extern  void  blurb( char* pgm, char* ver, char* title, char* helps );

extern  void  winfo( char* what, char** ver, char** date );
extern  char*  what( char* str );

extern  char*  ss( int testVar );
extern  char*  es( int testVar );
extern  char*  ar( int testVar );
extern  char*  wr( int testVar );

#endif
