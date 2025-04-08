/*******************************************************************************
* File: winfo.c						      v0.1   08/24/2021
*
* Purpose: Extract Version and Date information from the 'what' string.
*
* Example usage:
*   static char  *What = "@(#)dmp.c v0.11 08/24/2021 DataM";
*   static char  *Ver, *Date;
*
*   winfo( What, &Ver, &Date );
*
*   printf( "Version: \"%s\"   Date: \"%s\"\n", Ver, Date );
*
* Example output:
*
*   Version: "v0.11"   Date: "08/24/2021"
*
* Notes:
*   1. Compile instructions:  gcc -o $HOME/obj/winfo.o -c winfo.c
*                             ar -cru HOME/lib/libdatam.a HOME/obj/winfo.o
*                             ranlib HOME/lib/libdatam.a
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
*                dmp.c v0.11 08/24/2021 DataM
*                blurb.c v0.2 08/23/2021 DataM
*                grmfn.c v0.1 08/23/2021 DataM
*                winfo.c v0.1 08/24/2021 DataM
*
*   3. Referencing the 'What' variable prevents the compiler from optimizing
*      the 'What' string out of the binary file.  This reference provides no
*      functional logic to the program.
*
* History:
*   ver   ___date___  _______________________description________________________
*   0.1   08/24/2019  original version
*
*******************************************************************************/

static char  *What = "@(#)winfo.c v0.1 08/24/2021 DataM";

#include <string.h>


/* the winfo function */

void  winfo( char* what, char** ver, char** date )
{
   static char  whatTok[256];

   if ( !What  ||  !what  ||  !ver  ||  !date )  return;    /* Note 3 */

   /* copy the string since strtok is destructive and to get a static version */

   strncpy( whatTok, what, sizeof(whatTok) );   

           strtok( whatTok, " " );    /* we're discarding the first token */
   *ver  = strtok(    NULL, " " );
   *date = strtok(    NULL, " " );

   return;
}
