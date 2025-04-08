/*******************************************************************************
* File: what.c						      v0.2   02/28/2025
*
* Purpose: Extract Version/Date information from the 'what' string.
*
* Example usage:
*   static char  *What = "@(#)dmp.c v0.11 08/24/2021 DataM";
*
*   what( What );    // initialize the 'what' function
*
*   printf( "Version: \"%s\"   Date: \"%s\"\n", what("ver"), what("date") );
*
* Example output:
*
*   Version: "v0.11"   Date: "08/24/2021"
*
* Notes:
*   1. Compile instructions:  gcc -o $HOME/obj/what.o -c what.c
*                             ar -cru $HOME/lib/libdatam.a $HOME/obj/what.o
*                             ranlib $HOME/lib/libdatam.a
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
*                what.c v0.1 09/01/2021 DataM
*
*   3. Referencing the 'What' variable prevents the compiler from optimizing
*      the 'What' string out of the binary file.  This reference provides no
*      functional logic to the program.  (See line 85.)
*
*   4. The 'what' lead-in string, WhatLead, that's used to detect the "@(#)"
*      pattern can't be defined as "@(#)" directly in the code, or the 'what'
*      utility will detect this string within the WhatLead variable, and end up
*      producing an unexpected blank line in the output for 'what.o' and for
*      'libdatam.a', and for any executable using these objects.  To avoid this,
*      the WhatLead variable is defined in the code with a incomplete pattern
*      (with a NULL character instead of an '@' as the first character), and
*      then the '@' is assigned to the first character at the top of the 'what'
*      function.
*
* History:
*   ver   ___date___  _______________________description________________________
*   0.1   09/01/2019  original version
*   0.2   02/28/2025  eliminate second embedded "@(#)" lead-in string
*
*******************************************************************************/

static char  *What = "@(#)what.c v0.2 02/28/2025 DataM";

#include <stdio.h>
#include <string.h>


static int   WhatLen = 0;
static char  WhatTok[256], WhatMsg[256], WhatLead[5];

static char  WhatLead[5] = { '\0', '(', '#', ')', '\0' };    /* incomplete */
                           /* ^^............................... on purpose */

static char *WhatEmpty = "", *WhatDelim = " \t";

static char *InitOkay = "(initialized)";
static char *NotInit = "(not initialized)";
static char *NoInfo = "(no version information)";

static char *WhatStr[4] = { NULL, NULL, NULL, NULL };
enum         WhatStrs     { Name, Ver, Date, Extra };


void  whatClear( char* msg )
{
  WhatLen = 0;
  memset( WhatTok, 0x00, sizeof(WhatTok) );
  memset( WhatStr, 0x00, sizeof(WhatStr) );
  memset( WhatMsg, 0x00, sizeof(WhatMsg) );
  strncpy( WhatMsg, msg, sizeof(WhatMsg) - 1 );
  return;
}


char*  whatsThe( enum WhatStrs item )
{
  if ( item < Name  ||  item > Extra )  return ( NULL );
  if ( !WhatStr[Name] )  return ( WhatMsg );
  if ( !WhatStr[item] )  return ( WhatEmpty );
  return ( WhatStr[item] );
}


char*  what( char* str )
{
  WhatLead[0] = '@';    /* complete the embedded "@(#)" lead-in string now */

  if ( !What  &&  !WhatLen )  strcpy( WhatMsg, NotInit );

  if ( !str )    /* NULL */
  {
    whatClear( NotInit );
    return ( WhatMsg );
  }
  else if ( !str[0] )    /* empty string */
  {
    return ( WhatMsg );
  }
  else if ( !strncmp( str, WhatLead, strlen( WhatLead ) ) )   /* @(#) */
  {
    whatClear( NotInit );
    strncpy( WhatTok, str, sizeof(WhatTok) - 1 );
    WhatLen = strlen( WhatTok );

    char  *tmp = strtok( WhatTok, WhatDelim );

    if ( !strcmp( tmp, WhatLead ) )    /* space after lead-in */
    {
      if ( !( tmp = strtok( NULL, WhatDelim ) ) )    /* nothing after lead-in */
      {
        whatClear( NoInfo );
        return ( NULL );
      }
    }
    else
    {
      tmp += strlen( WhatLead );
    }
    WhatStr[Name] = tmp;

    /* now loop through the rest of the 'what' string */

    for ( int i = Ver;  i <= Extra  &&
                        ( WhatStr[i] = strtok( NULL, WhatDelim ) );  i++ );

    /* un-terminate Extra (if present) */
    if ( WhatStr[Extra] )
    {
      int  len = strlen( WhatStr[Extra] );
      if ( WhatTok + WhatLen > WhatStr[Extra] + len )    /* more info in what */
      {
        WhatStr[Extra][len] = ' ';    /* give back all the rest to Extra */
      }
    }
    strcpy( WhatMsg, InitOkay );
    return ( WhatMsg );
  }
  else if ( !strcmp( str, "name" ) )    /* name */
  {
    return ( whatsThe( Name ) );
  }
  else if ( !strcmp( str, "ver" ) )    /* version */
  {
    return ( whatsThe( Ver ) );
  }
  else if ( !strcmp( str, "date" ) )    /* date */
  {
    return ( whatsThe( Date ) );
  }
  else if ( !strcmp( str, "extra" ) )    /* extra */
  {
    return ( whatsThe( Extra ) );
  }
  else if ( !strcmp( str, "what" ) )    /* what */
  {
    if ( !WhatStr[Name] )  return ( WhatMsg );    /* NotInit or NoInfo */

    memset( WhatMsg, 0x00, sizeof(WhatMsg) );

    for ( int  i = Name;  i <= Extra  &&  WhatStr[i];  i++ )
    {
      if ( i > Name )  strcat( WhatMsg, " " );
      strcat( WhatMsg, WhatStr[i] );
    }
    return ( WhatMsg );
  }
  else    /* unknown */
  {
    sprintf( WhatMsg, "(unknown option: %s)", str );
    return ( WhatMsg );
  }
}

