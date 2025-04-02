/*******************************************************************************
* File: blurb.c						      v0.2   08/23/2021
*
* Purpose: Write the no-arguments help blurb, based on the supplied inputs.
*
*   The blurb function writes a standard-format help blurb to stdout.
*
*   Depending on the presence or absence of the 'helps' argument, this function
*   indents its output (when 'helps' is present); or it aligns the output to the
*   start of a line (when 'helps is absent, as for the 'about_msg' function).
*
* Example usage:
*   char  *pgm = "testing";
*   char  *ver = "v1.2a";
*   char  *title = "Department of Redundancy Department Utility";
*   char  *helps = "(use '%s -help' for help)";
*
*   blurb( pgm, ver, title, helps );
*
* Example output (#1):  blurb( pgm, ver, title, helps );
*
*   % testing
*
*      testing   Department of Redundancy Department   v1.2a
*                (use 'testing -help' for help)
*
* Example output (#2):  blurb( pgm, ver, title, "" );
*
*   % testing -about
*
*   testing   Department of Redundancy Department   v1.2a
*
* Notes:
*   1. Compile instructions:  gcc -c blurb.c
*                             ar -cru libdatam.a blurb.o
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
*                blurb.c v0.2 08/23/2021 DataM
*                grmfn.c v0.1 08/23/2021 DataM
*
*   3. Referencing the 'What' variable prevents the compiler from optimizing
*      the 'What' string out of the binary file.  This reference provides no
*      functional logic to the program.
*
* History:
*   ver   ___date___  _______________________description________________________
*   0.1   08/16/2019  original version
*   0.2   08/23/2019  added 'What' reference and notes; revised for about_msg
*
*******************************************************************************/

static char  *What = "@(#)blurb.c v0.2 08/23/2021 DataM";

#include <stdio.h>
#include <string.h>


/* the blurb function */

void  blurb( char* pgm, char* ver, char* title, char* helps )
{
   if ( !What )  return;    /* Note 3 */

   /* write leading blank line and blurb line one */

   printf( "\n" );

   if ( helps  &&  helps[0] )  printf( "   " );    /* offset for no-args help */

   if (   pgm  &&    pgm[0] )  printf( "%s", pgm );
   if ( title  &&  title[0] )  printf( "   %s", title );
   if (   ver  &&    ver[0] )  printf( "   %s", ver );

   printf( "\n" );    /* terminate blurb line-one */

   /* write [optional] blurb line-two */

   if ( helps  &&  helps[0] )   /* a valid 'helps' string is present */
   {
      int   ln = 3;    /* 3-space msg indent */
      char  sp[32];

      memset( sp, 0x20, sizeof(sp) );

      if ( pgm  &&  pgm[0] )   /* sanity check */
      {
         ln += strlen( pgm );   /* add the length of the Pgm name */

         if ( ln >= sizeof(sp) )  ln = 0;    /* don't even try justification */
      }

      sp[ln] = '\0';    /* justify spacing for about pgm name */

      printf( "   %s", sp );

      printf( helps, pgm );
      printf( "\n" );
   }

   /* write trailing blank line */

   printf( "\n" );

   return;
}
