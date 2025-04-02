/*******************************************************************************
* File: grammer.c					      v0.1   08/16/2021
*
* Purpose: Grammar functions.
*
*   These functions simplify the grammar for singular/plural items.
*
*   grammar functions (because, English):  __________equivalent C code__________
*
*     ss( testVal )                        ( testVal == 1 ? "" : "s" )
*     es( testVal )                        ( testVal == 1 ? "" : "es" )
*     ar( testVal )                        ( testVal == 1 ? "is" : "are" )
*     wr( testVal )                        ( testVal == 1 ? "was" : "were" )

* Usage:  fprintf( Fpo, "End-of-File   (%i byte%s)", count, ss( count ) );
*
* Notes:
*   1. Compile instructions:  gcc -c grammer.c
*                             ar -cru libdatam.a grammer.o
*                             ranlib libdatam.a
*
*   2. Support for the 'what' command is provided via the arcane string that's
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
*                grammer.c v0.1 08/23/2021 DataM
*
*   3. Referencing the 'What' variable prevents the compiler from optimizing
*      the 'What' string out of the binary file.  This reference provides no
*      functional logic to the program.
*
* History:
*   ver   ___date___  _______________________description________________________
*   0.1   08/23/2019  original version
*
*******************************************************************************/

static char  *What = "@(#)grammer.c v0.1 08/23/2021 DataM";


/* grammer - what reference (no actual functional purpose) */

int  grammer( void )
{
  return ( What ? 1 : 0 );
}


/* ss - question mark colon 's' singular/plural grammar function */

char*  ss( int testVal )
{
  return ( testVal == 1 ? "" : "s" );
}


/* es - question mark colon 'es' singular/plural grammar function */

char*  es( int testVal )
{
  return ( testVal == 1 ? "" : "es" );
}


/* ar - question mark colon 'is/are' grammar function */

char*  ar( int testVal )
{
  return ( testVal == 1 ? "is" : "are" );
}


/* wr - question mark colon 'was/were' grammar function */

char*  wr( int testVal )
{
  return ( testVal == 1 ? "was" : "were" );
}
