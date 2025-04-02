/*******************************************************************************
* File: dmp.c						     v0.21   03/27/2025
*
* Purpose: File hex/ASCII dump utility.
*
*   The dmp utility reads the specified file(s), byte-by-byte, and outputs each
*   byte in hexadecimal, with various output formatting options.
*
*   If an isolated '-' or '--' is specified in the command line arguments, the
*   very next argument is taken to be a filename.  This option allows access to
*   filenames beginning with the '-' character.
*
*   Multiple filenames may be specified, and options affect the dump output of
*   all the files that follow.  If dumping from a pipe, all options affect the
*   pipe dump output.  Input from a pipe overrides and precludes input from a
*   file (or files).
*
* Usage:  dmp  [ [ options ]  [-|--]  [ file.ext ] ] ...
*
*    or:  echo "example pipe contents"  |  dmp  [ options ]
*
* Options:
*       +# = start dump at byte #  (default: start at first byte in file: '+0')
*       -# = limit dump to # bytes (default: dump all bytes in file: '-0')
*       -a = omit (-) or show (+) ASCII dump
*      -b# = set byte group to # bytes, -b = 1 (default), +b = 2
*       -c = continuous byte dump as fixed-length lines (-) or single string (+)
*     -e.# = set output file extension to # (default: "dmp")
*       -f = output to file: file.dmp (-) or file.ext.dmp (+)
*     -f.# = output to file: file.#   (-) or file.ext.#   (+)
*     -f:# = output to file #.dmp in current (-) or input file's (+) directory
*     -f=# = output to file #     in current (-) or input file's (+) directory
*  -f:#.## = output to file #.##  in current (-) or input file's (+) directory
*  -f=#.## = output to file #.##  in current (-) or input file's (+) directory
*            (the -f: and -f= options combine all outputs into the named file)
*       -i = omit (-) or show (+) information headers
*       -l = use lowercase (-) or uppercase (+) ASCII digits (default)
*       -n = omit (-) or show (+) line/address numbers
*      -n# = format line/address as #: s:short (default), l:long, v:variable
*      -p# = dump # bytes per line (default is 16)
*      -w# = set word group to # bytes, -w = 4, +w = 8
*       -x = omit (-) or show (+) hex digits dump
*       -X = emulate 'hexdump -C -v' output format
*      -xo = hex-only dump: as bytes (-) or continuous (+)
*   -about = show about message
*   -debug = enable debug outputs
*    -help = show help message
*     -ver = show version message
*
* Notes:
*   1. Compile instructions:  gcc -o bin/dmp dmp.c -L. -ldatam
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
*   3. The 'Ver' and 'Date' values are extracted from a copy of the 'What'
*      string.  (Referencing the 'What' variable also prevents the compiler
*      from optimizing this 'What' string out of the binary file.)
*
* History:
*   ver   ___date___  _______________________description________________________
*   0.1   05/01/2019  original version (again)
*   0.2   09/20/2019  switched -g option to -w, changed -w to -w1, +w to -w4
*   0.3   09/20/2019  added -e., and -f: options
*   0.4   10/29/2019  added -b option
*   0.5   05/13/2020  fixed -f losing filename extension for multiple files
*   0.6   08/04/2021  transcribed to Radix host (with some corrections)
*   0.7   08/05/2021  support '--' options and 'hexdump -C -v' output format
*   0.8   08/06/2021  added 'what' support, and -x/-xo options
*   0.9   08/09/2021  updated no-input help with auto-justification
*   0.10  08/23/2021  using 'blurb' and 'ss' functions from datam library
*   0.11  08/24/2021  using 'winfo' to extract version and date
*   0.12  09/01/2021  using 'what' to extract version and date
*   0.13  06/19/2024  change '+#' per-line to '-p#'; add '+#' for start at byte
*   0.14  06/25/2024  fixed terminal format blank line outputs
*   0.15  03/07/2025  support input from pipe operations
*   0.16  03/08/2025  fixed initial pipe support; refactored main loop
*   0.17  03/10/2025  finalized pipe support; fixed '-f:'/'-f=' for multi-file
*   0.18  03/11/2025  fixed empty/blank filename argument handling
*   0.19  03/25/2025  changed '-b' (1) and '-w' (4) options; debug for '--'/'++'
*   0.20  03/25/2025  unified b/w options: -b (1), +b (2), -w (4), +w (8)
*   0.21  03/27/2025  changed default pipe file name, set default extn just once
*
*******************************************************************************/

static char  *What = "@(#)dmp.c v0.21 03/27/2025 DataM";
static char  *Title = "File Hex/ASCII Dump Utility";

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>

#include "datam.h"

/* helper functions */

int  dump_file( FILE* fpi, FILE* fpo );

int  proc_args( int* aix, int argc, char** argv );
int  open_files();

int  help_msg( int mx );
int  ver_msg( int mx );
int  about_msg( int mx );

/* global variables */

static int   Debug, ToFile, Ascii, LoCase, WordLen, PerLine, AddrNum;
static int   Count, Header, Footer, LocDir, AddExt, HalfGap, EndAddr;
static int   AscWide, TermFmt, HexDump, Start, Pipe, AllOut, NewOut, Files;

static char  *Pgm, *Name, *DefExts;
static char  DefExtn[256], OutName[1024], OutExtn[256], OutFile[1024];

static char  *DefExtd = ".dmp", *DefPipe = "pipe";

static FILE  *Fpi, *Fpo;


/* the main program for dmp */

int  main( int argc, char** argv )
{
   struct stat  sts;    /* used to detect pipe operations */

   int  aix, cnt, count, err;

   /* set-up global defaults */

   Debug   = 0;    /* turn off debug outputs */
   ToFile  = 0;    /* output to stdout (0), file.dmp (1), or file.ext (2) */
   AllOut  = 0;    /* combine all outputs to one file */
   NewOut  = 0;    /*   (new output file option specified) */
   Files   = 0;    /*   (number of input files processed)  */
   AddExt  = 1;    /* add filename extension to output file */
   LocDir  = 1;    /* output to local (current) directory */
   LoCase  = 0;    /* dump hex in uppercase (0) or lowercase (1) */
   Ascii   = 1;    /* dump ASCII representation at end of each line */
   AscWide = 1;    /* always full width ASCII field */
   HexDump = 1;    /* output hex-digits dump */
   WordLen = 1;    /* group into 1, 2, 4, 8, 16 bytes, or none (0) */
   PerLine = 16;   /* output N bytes per line, or continuous (0) */
   AddrNum = 2;    /* output line address numbers (n1 n2 n3) or don't (0) */
   HalfGap = 0;    /* don't add extra space half way through dump line */
   EndAddr = 0;    /* don't report final address line (which is byte count) */
   TermFmt = 1;    /* show terminal-only (blank) lines */
   Header  = 1;    /* show header info in dump output */
   Footer  = 1;    /* show footer info in dump output */
   Count   = 0;    /* number of bytes to dump, or dump all (0) */
   Start   = 0;    /* start dump at first byte in file (0) */
   Pipe    = 0;    /* default input is from files, not from pipe */

   /* set the program name and initialize the 'what' string info */

   Pgm = argv[0];

   what( What );

   /* set-up the file variables */

   DefExts = &DefExtd[1];    /* just the "###" part of the dot.ext ".###" */

   strcpy( DefExtn, DefExtd );    /* copy in the default OutFile extension */

   memset( OutFile, 0x00, sizeof(OutFile) );

   memset( OutName, 0x00, sizeof(OutName) );
   memset( OutExtn, 0x00, sizeof(OutExtn) );

   Name = NULL;

   Fpi = NULL;
   Fpo = NULL;

   /* check for pipe vs. non-pipe first */

   if ( fstat( STDIN_FILENO, &sts ) != -1 )
      Pipe = ( ( sts.st_mode & S_IFIFO ) != 0 );

   /* check for no-arguments cases: pipe vs. non-pipe */
   /* (pipe w/o args is okay; non-pipe w/no args shows a clue) */

   if ( Pipe )    /* using a pipe as input */
   {
      Name = DefPipe;    /* give us a "filename" to support To-File ops */
   }
   else if ( argc == 1 )   /* no arguments specified -- show brief message */
   {
      blurb( Pgm, what("ver"), Title, "(use '%s -help' for help)" );
      return ( 411 );
   }

   /* main processing loop; done when all arguments are processed */

   aix = 1;    /* start with the first command line argument (if any) */
   err = 0;

   do    /* process command line arguments and dump each file */
   {
      err = proc_args( &aix, argc, argv );

      if ( Name  &&  !err )   /* open the file */
      {
         err = open_files();

         if ( !err  &&  TermFmt )  printf( "\n" );
      }

      if ( Name  &&  !err )   /* dump the file */
      {
         if ( Header )
         {
            if ( AllOut > 1 )  fprintf( Fpo, "\n" );   /* before appended hdr */

            if ( Pipe )
               fprintf( Fpo, "    Dump of Pipe: (stdin)\n" );
            else
               fprintf( Fpo, "    Dump of File: %s\n", Name );
         }

         cnt = dump_file( Fpi, Fpo );

         /* end-of-file reporting */

         count = ( cnt >= 0 ? cnt : -cnt );

         if ( Footer )
         {
            if ( cnt >= 0 )
            {
               fprintf( Fpo, "    End-of-File   (%i byte%s)",
                             count, ss( count ) );

               if ( Count )
                  fprintf( Fpo, "  (EoF before %i-byte limit)\n", Count );
               else
                  fprintf( Fpo, "\n" );
            }
            else   /* dump ended at byte-count */
            {
               fprintf( Fpo, "    End-of-Dump   (%i byte%s)\n",
                             count, ss( count ) );
            }
         }

         /* report output filename (to stdout) */

         if ( ToFile )
         {
            printf( "    Dumped output (%i byte%s) to file: %s%s\n",
                    count, ss( count ), OutName,
                    ( AllOut < 2 ? "" : " (appended)" ) );
         }

         /* close files and clear names */

         if ( Fpo  &&  Fpo != stdout  &&  !AllOut )    /* close output file */
         {
            if ( Debug )  printf( "(closing output file)\n" );

            memset( OutName, 0x00, sizeof(OutName) );

            fclose( Fpo );
            Fpo = NULL;
         }

         if ( !Pipe )    /* close input file (not a pipe) */
         {
            if ( Fpi )  fclose( Fpi );
            Fpi = NULL;

            Name = NULL;
         }

         Files++;    /* another input file was processed */

         if ( Debug )  printf( "(Files.%i aix.%i err.%i)\n%s",
                               Files, aix, err,
                               ( aix < argc ? "(...)\n" : "" ) );
      }

   } while ( aix < argc  &&  !err );

   /* end-of-loop terminal operations */

   /* close output file (when combining all outputs into one file) */

   if ( Fpo  &&  Fpo != stdout )    /* close output file */
   {
      if ( Debug )  printf( "(closing output file)\n" );

      memset( OutName, 0x00, sizeof(OutName) );

      fclose( Fpo );
      Fpo = NULL;
   }

   if ( Files  &&  TermFmt )  printf( "\n" );

   return ( err );
}


int  open_files()
{
   int   err = 0;
   char  *dot;

   if ( !Name  ||  !Name[0]  ||  isspace( Name[0] ) )  return ( 1 );

   /* open the input source */

   if ( Pipe )    /* using pipe for input */
   {
      if ( Fpi  &&  Fpi != stdin )  fclose( Fpi );    /* just in case */

      Fpi = stdin;

      if ( Debug )  printf( "(using pipe for input)\n" );
   }
   else if ( ( Fpi = fopen( Name, "r" ) ) == 0 )    /* file open failed */
   {
      err = errno;

      if ( Files )  printf( "\n" );
      printf( "  error %i opening input file: \"%s\"\n", err, Name );
      printf( "  (%s)\n", strerror( err ) );
   }

   /* open the output destination */

   if ( !err )    /* input file opened successfully (or using pipe) */
   {
      memset( OutName, 0x00, sizeof(OutName) );

      if ( ToFile )   /* output is to a file (not stdout) */
      {
         /* create the output filename */

         if ( OutFile[0] )   /* the output filename was specified... */
         {
            if ( strrchr( OutFile, '/' ) )   /* ...with a directory */
            {
               strncpy( OutName, OutFile, sizeof(OutName) - 1 );
            }
            else   /* no output directory specified */
            {
               if ( !LocDir )   /* use input directory (if any) */
               {
                  strncpy( OutName, Name, sizeof(OutName) - 1 );

                  if ( ( dot = strrchr( OutName, '/' ) ) )
                  {
                     dot[1] = '\0';    /* strip off input filename in OutName */
                  }
                  else   /* no directory in input filename */
                  {
                     memset( OutName, 0x00, sizeof(OutName) );   /* clean-out */
                  }
               }

               /* append specified output filename */

               strncat( OutName, OutFile, sizeof(OutName) - 1 );
            }

            if ( Debug )  printf( "(output base = \"%s\")\n", OutName );
         }
         else   /* base the output filename on the input filename */
         {
            /* check for a directory in the input filename */

            if ( LocDir  &&  ( dot = strrchr( Name, '/' ) ) )
            {
               /* strip the leading path from the input filename */
               /*    dot = "/filename.extn"                      */

               if ( Debug )  printf( "(output dot = \"%s\")\n", dot );

               strncpy( OutName, &dot[1], sizeof(OutName) - 1 );
            }
            else   /* output to input file's directory or no input file dir */
            {
               strncpy( OutName, Name, sizeof(OutName) - 1 );
            }

            if ( Debug )  printf( "(output base = \"%s\")\n", OutName );

            if ( ToFile == 1  &&  ( dot = strrchr( OutName, '.' ) ) )
            {
               dot[0] = '\0';    /* truncate current extension */
            }
         }

         if ( AddExt )   /* add a filename extension to the output filename */
         {
            if ( OutExtn[0] )   /* user-specified extension */
            {
               strcat( OutName, OutExtn );
            }
            else   /* use the default */
            {
               strcat( OutName, DefExtn );
            }
         }

         /* if no output extension was specified, use the default */

         if ( !OutExtn[0] )  strcat( OutExtn, DefExtn );

         /* don't let the output and input filenames match */

         if ( !strcmp( Name, OutName ) )  strcat( OutName, OutExtn );

         if ( Debug )  printf( "(output name = \"%s\")\n", OutName );

         /* close the output file if a new one was specified */

         if ( Fpo  &&  Fpo != stdout  &&  AllOut  &&  NewOut )
         {
            fclose( Fpo );
            Fpo = NULL;

            if ( Debug )  printf( "(closed old output file)\n" );
         }

         /* open the output file */

         if ( AllOut  &&  Fpo )    /* output file already opened */
         {
            if ( Debug )
               printf( "(append.... output file: \"%s\")\n", OutName );

            AllOut++;    /* count the number of appended output files */
         }
         else if ( ( Fpo = fopen( OutName, "w+" ) ) == 0 )
         {
            err = errno;

            if ( Files )  printf( "\n" );
            printf( "  error %i opening output file: \"%s\"\n", err, OutName );
            printf( "  (%s)\n", strerror( err ) );
         }
         else    /* output file opened okay */
         {
            NewOut = 0;

            if ( Debug )
               printf( "(opened new output file: \"%s\")\n", OutName );
         }
      }
      else   /* output is to stdout */
      {
         if ( Fpo  &&  Fpo != stdout )  fclose( Fpo );

         Fpo = stdout;
      }
   }

   /* report out input/output */

   if ( Debug  &&  !err )
   {
      if ( Pipe )
         printf( "(input from stdin)\n" );
      else
         printf( "(opened input file: \"%s\")\n", Name );

      if ( ToFile )
         printf( "(output to file: \"%s\")\n", OutName );
      else
         printf( "(output to stdout)\n" );
   }

   return ( err );
}


int  dump_file( FILE* fpi, FILE* fpo )
{
   int  adr = 0, ch, cnt = 0, ix = 0;

   char  asc[64], sp[4];

   if ( !fpi  ||  !fpo )  return ( 0 );

   memset( asc, 0x00, sizeof(asc) );

   while ( ( ch = fgetc( fpi ) ) != EOF  &&  ( !Count  ||  cnt < Count ) )
   {
      if ( Start  &&  adr < Start )
      {
         adr++;
         continue;
      }

      if ( !ix  &&  AddrNum )
      {
         if ( AddrNum == 1 )
         {
            fprintf( fpo, ( LoCase ? "%04x  " : "%04X  " ), adr );
         }
         else if ( AddrNum == 2 )
         {
            fprintf( fpo, ( LoCase ? "%08x  " : "%08X  " ), adr );
         }
         else if ( AddrNum == 3 )
         {
            if ( adr < 0x00010000 )
               fprintf( fpo, ( LoCase ? "    %04x  " : "    %04X  " ), adr );
            else if ( adr < 0x00100000 )
               fprintf( fpo, ( LoCase ? "   %05x  " : "   %05X  " ), adr );
            else if ( adr < 0x01000000 )
               fprintf( fpo, ( LoCase ? "  %06x  " : "  %06X  " ), adr );
            else if ( adr < 0x10000000 )
               fprintf( fpo, ( LoCase ? " %07x  " : " %07X  " ), adr );
            else /* ( adr < 0x10000000 ) */
               fprintf( fpo, ( LoCase ? "%08x  " : "%08X  " ), adr );
         }
      }

      if ( HexDump )  fprintf( fpo, ( LoCase ? "%02x" : "%02X" ), ch );

      if ( Ascii )
      {
         if ( ch == '\0' )
            asc[ix] = '_';
         else if ( ch < ' ' )
            asc[ix] = '.';
         else if ( ch > '~' )
            asc[ix] = '.';
         else
            asc[ix] = ch;
      }

      ix++;

      if ( HexDump )
      {
         if ( WordLen  &&  ( ix % WordLen ) == 0 )  fprintf( fpo, " " );
         if ( HalfGap  &&  ( ix % HalfGap ) == 0 )  fprintf( fpo, " " );
      }

      if ( PerLine  &&  ix >= PerLine )
      {
         if ( Ascii )
         {
            memset( sp, 0x00, sizeof(sp) );

            if ( HexDump )
            {
               strcpy( sp,"  " );
               if ( WordLen              )  sp[1] = '\0';
               if ( WordLen  &&  HalfGap )  sp[0] = '\0';
            }

            fprintf( fpo, "%s|%s|", sp, asc );
         }

         fprintf( fpo, "\n" );

         ix = 0;
         memset( asc, 0x00, sizeof(asc) );
      }

      adr++;
      cnt++;
   }

   /* end-of-file processing */

   if ( Ascii  &&  ix )   /* finish off the hex dump line w/ASCII */
   {
      /* blank-fill the rest of the hex data portion */

      while ( PerLine  &&  ix < PerLine )
      {
         if ( HexDump )  fprintf( fpo, "  " );    /* spaces instead of digits */

         if ( AscWide )  asc[ix] = ' ';    /* to justify the right column */

         ix++;

         if ( HexDump )
         {
            if ( WordLen  &&  ( ix % WordLen ) == 0 )  fprintf( fpo, " " );
            if ( HalfGap  &&  ( ix % HalfGap ) == 0 )  fprintf( fpo, " " );
         }
      }

      /* blank-fill the rest of the ASCII data portion */

      memset( sp, 0x00, sizeof(sp) );

      if ( HexDump )
      {
         strcpy( sp,"  " );
         if ( WordLen              )  sp[1] = '\0';
         if ( WordLen  &&  HalfGap )  sp[0] = '\0';
      }

      /* write the final part of the dump line */

      fprintf( fpo, "%s|%s|\n", sp, asc );

      memset( asc, 0x00, sizeof(asc) );
   }
   else if ( ix )   /* finish off the hex dump line w/o ASCII */
   {
      fprintf( fpo, "\n" );
   }

   /* report the ending (next) address, like 'hexdump -C -v' */

   if ( EndAddr )  fprintf( fpo, "%08x\n", cnt );

   /* report differently for End-of-File and count-limited dumps */

   return ( ( ch == EOF ? cnt : -cnt ) );
}


int  about_msg( int mx )
{
   /* show us the program name, title, and version... */

   blurb( Pgm, what("ver"), Title, "" );    /* blurb line-one only */

   if ( mx < 0 )  return( mx );    /* all done */

   /* ...and then, whatever else we might want to know */

   if ( mx )    /* +about */
   {
      printf( "(no additional information for +about)\n" );
   }
   else    /* -about */
   {
      printf( "Developed to gain quick and consistent insight into binary"
              " files.\n" );
   }

   printf( "\n" );

   return ( 411 );
}


int  ver_msg( int mx )
{
   printf( "\n" );

   if ( mx )    /* +ver */
   {
      printf( "%s   %s\n", Pgm, what("what") );
   }
   else    /* -ver */
   {
      printf( "%s   %s\n", Pgm, what("ver") );
   }

   printf( "\n" );

   return ( 411 );
}


int  proc_args( int* aix, int argc, char** argv )
{
   int   err = 0, i, ln, mx;
   char  *optn, opt;

   if ( !aix )  return ( -1 );

   while ( *aix < argc  &&  !err )
   {
      if ( argv[*aix][0] == '-'  ||  argv[*aix][0] == '+' )
      {
         mx = argv[*aix][0] == '+';

         /* create a pointer to the start of the current option string */

         i = ( argv[*aix][0] == argv[*aix][1] ? 2 : 1 );

         optn = &argv[*aix][i];    /* starts after the '-'/'--' part */

         opt = optn[0];    /* grab the option character */

         /* decode the command line option string */

         if ( !strcmp( optn, "debug" ) )
         {
            Debug = mx + 1;

            printf( "(debug: %i)\n", Debug );
         }
         else if ( !strcmp( optn, "about" ) )
         {
            err = about_msg( mx );
         }
         else if ( !strcmp( optn, "help" ) )
         {
            err = help_msg( mx );
         }
         else if ( !strcmp( optn, "ver" )  ||
                   !strcmp( optn, "version" ) )
         {
            err = ver_msg( mx );
         }
         else if ( !strcmp( optn, "xo" ) )   /* hex-only */
         {
            AddrNum = 0;
            Ascii = 0;

            PerLine = 0;
            HalfGap = 0;
            WordLen = !mx;    /* -xo for bytes, +xo for continuous */
         }
         else if ( isdigit( opt ) )   /* -###  and  +### */
         {
            if ( mx )   /* +# = set start-byte of dump */
            {
               if ( sscanf( optn, "%i", &Start ) != 1 )
               {
                  Start = 0;

                  printf( "  bad start-byte option \"%s\"\n", argv[*aix] );
                  err = 1;
               }
            }
            else   /* -N = set dump byte limit */
            {
               if ( sscanf( optn, "%i", &Count ) != 1 )
               {
                  Count = 0;

                  printf( "  bad byte-limit option \"%s\"\n", argv[*aix] );
                  err = 1;
               }
            }

            if ( Debug )  printf( "(Start: %i   Count: %i)\n", Start, Count );
         }
         else if ( opt == 'a' )   /* -a */
         {
            Ascii = mx;
         }
         else if ( opt == 'c' )   /* -c */
         {
            if ( mx )  PerLine = 0;

            HalfGap = 0;
            WordLen = 0;

            Ascii = 0;
            AddrNum = 0;
         }
         else if ( opt == 'e' )   /* -e */
         {
            if ( !optn[1] )   /* -e = default */
            {
               strcpy( DefExtn, DefExtd );
            }
            else if ( optn[1] == '.' )
            {
               if ( !optn[2] )   /* -e. = default */
               {
                  strcpy( DefExtn, DefExtd );
               }
               else   /* -e.extn = replace default with ".extn" */
               {
                  strncpy( DefExtn, &optn[1], sizeof(DefExtn) - 1 );
               }
            }
            else   /* -e? bad */
            {
               printf( "  bad extension option \"%s\"\n", argv[*aix] );
               err = 1;
            }

            if ( Debug )  printf( "(DefExtn: \"%s\")\n", DefExtn );
         }
         else if ( opt == 'f' )   /* -f -f.### -f:### -f=### */
         {
            ToFile = mx + 1;   /* 1 (-f) or 2 (+f) */
            AddExt = 1;        /* default to adding an output file extension */
            AllOut = 0;        /* default to single file */

            memset( OutFile, 0x00, sizeof(OutFile) );
            memset( OutExtn, 0x00, sizeof(OutExtn) );

            if ( !optn[1] )   /* -f = use filename w/default extn */
            {
               strcpy( OutExtn, DefExtn );
            }
            else if ( optn[1] == '.' )   /* -f.extn */
            {
               if ( !optn[2] )   /* -f. = use filename w/default extn */
               {
                  strcpy( OutExtn, DefExtn );
               }
               else   /* -f.extn = use ".extn" for this output file */
               {
                  strncpy( OutExtn, &optn[1], sizeof(OutExtn) - 1 );
               }
            }
            else if ( optn[1] == ':'  ||   /* -f:file w/auto-extn */
                      optn[1] == '=' )     /* -f=file w/o auto-extn */
            {
               LocDir = !mx;    /* -f: use local dir, +f: use input dir */
               AllOut = 1;      /* combine all outputs into one file */
               NewOut = 1;      /* new output file option */

               if ( !optn[2] )   /* -f: / -f= use filename w/default extn */
               {
                  strcpy( OutExtn, DefExtn );
               }
               else   /* -f:file or -f=file use "file" for this output file */
               {
                  strncpy( OutFile, &optn[2], sizeof(OutFile) - 1 );

                  /* don't do auto-extn for '-f=' or if an extn was specified */

                  if ( optn[1] == '='  ||  strrchr( OutFile, '.' ) )
                  {
                     AddExt = 0;    /* use exactly the output filename */
                  }
                  else    /* doing auto-extn is what we want */
                  {
                     strcpy( OutExtn, DefExtn );    /* use default extension */
                  }
               }
            }
            else   /* -f? bad */
            {
               printf( "  bad file option \"%s\"\n", argv[*aix] );
               err = 1;
            }

            if ( Debug )
            {
               printf( "(ToFile: %i)\n", ToFile );
               printf( "(AllOut: %i)\n", AllOut );
               printf( "(LocDir: %i)\n", LocDir );
               printf( "(AddExt: %i)\n", AddExt );
               printf( "(OutFile: %s)\n", ( OutFile[0] ? OutFile : "----" ) );
               printf( "(OutExtn: %s)\n", ( OutExtn[0] ? OutExtn : "----" ) );
            }
         }
         else if ( opt == 'i' )   /* -i */
         {
            TermFmt = mx;    /* show terminal-only (blank) lines */
            Header  = mx;    /* show header info in dump output */
            Footer  = mx;    /* show footer info in dump output */
         }
         else if ( opt == 'l' )   /* -l */
         {
            LoCase = !mx;
         }
         else if ( opt == 'n' )   /* -n -n# */
         {
            AddrNum = mx;

            if ( !optn[1]             ||  optn[1] == '0' )
               AddrNum = 0;
            else if ( optn[1] == 's'  ||  optn[1] == '1' )
               AddrNum = 1;
            else if ( optn[1] == 'l'  ||  optn[1] == '2' )
               AddrNum = 2;
            else if ( optn[1] == 'v'  ||  optn[1] == '3' )
               AddrNum = 3;
            else
            {
               printf( "  unknown line address \"%s\"\n", argv[*aix] );
               err = 1;
            }
         }
         else if ( opt == 'p' )   /* -p/-p# */
         {
            /* scan the rest of the string to make sure it's all numeric */

            for ( i = 1;  optn[i]  &&  !err;  i++ )
            {
               if ( !isdigit( optn[i] ) )   /* verify numeric only */
               {
                  err = 1;    /* non-numeric encountered */

                  printf( "  bad bytes-per-line option \"%s\"\n",
                          argv[*aix] );
                  PerLine = 16;
               }
            }

            if ( !err )   /* all-numeric option -- okay for -p/-p# */
            {
               if ( !optn[1] )   /* just -p (no per-line value) */
               {
                  PerLine = 0;
               }
               else if ( sscanf( &optn[1], "%i", &PerLine ) != 1 )
               {
                  PerLine = 16;

                  printf( "  bad bytes-per-line option \"%s\"\n",
                          argv[*aix] );
                  err = 1;
               }

               if ( PerLine == 0 )   /* no-limit lines */
               {
                  AddrNum = 0;
                  Ascii = 0;
               }
               else if ( PerLine > 24 )   /* too-long lines */
               {
                  Ascii = 0;
               }
            }

            if ( Debug )
            {
               printf( "(PerLine: %i)\n", PerLine );
               printf( "(AddrNum: %i)\n", AddrNum );
               printf( "(  Ascii: %i)\n", Ascii );
            }
         }
         else if ( opt == 'b'  ||  opt == 'w' )   /* -b -b## -w -w## */
         {
            /* scan the rest of the string to make sure it's all numeric */

            for ( i = 1;  optn[i]  &&  !err;  i++ )
            {
               if ( !isdigit( optn[i] ) )   /* verify numeric only */
               {
                  printf( "  bad %s group option \"%s\"\n",
                          ( opt == 'b' ? "byte" : "word" ), argv[*aix] );
                  err = 1;
               }
            }

            if ( !err )   /* all-numeric option */
            {
               if ( !optn[1] )   /* -b = 1, +b = 2, -w = 4, +w = 8 */
               {
                  if ( opt == 'b' )              /* -b  +b   default opt */
                     WordLen = ( mx ? 2 : 1 );   /*   1   2  bytes/group */
                  else                           /* -w  +w   default opt */
                     WordLen = ( mx ? 8 : 4 );   /*   4   8  bytes/group */
               }
               else if ( sscanf( &optn[1], "%i", &WordLen ) != 1 )
               {
                  WordLen = ( opt == 'b' ? 1 : 4 );    /* revert to default */

                  printf( "  bad %s group option \"%s\"\n",
                          ( opt == 'b' ? "byte" : "word" ), argv[*aix] );
                  err = 1;
               }
            }

            if ( Debug )
            {
               printf( "(WordLen: %i)\n", WordLen );
            }
         }
         else if ( opt == 'x' )   /* -x */
         {
            HexDump = mx;
         }
         else if ( opt == 'X' )   /* -X (emulate 'hexdump -C -v') */
         {
            TermFmt = 0;    /* don't show terminal-only (blank) lines */
            Header  = 0;    /* don't show header info in dump output */
            Footer  = 0;    /* don't show footer info in dump output */

            LoCase = 1;

            AddrNum = 2;
            Ascii = 1;

            PerLine = 16;
            HalfGap = 8;
            WordLen = 1;

            AscWide = 0;
            EndAddr = 1;
         }
         else if ( !opt )   /* '-' or '--' alone, with no option string */
         {
            if ( Debug )
            {
               printf( "(argv[%i]: \'%s\'  argc: %i  mx: %i) ... (%s)\n",
                       *aix, argv[*aix], argc, mx,
                       ( *aix + 1 < argc ? ( Pipe ? "!" : "ok" ) : "?" ) );
            }

            *aix = *aix + 1;    /* next option is a filename */

            if ( *aix < argc  )    /* there is a next option to process */
            {
               if ( Pipe )    /* pipe operation precludes use of input files */
               {
                  printf( "  invalid option (\"%s\"): input file is not valid"
                          " in pipe operations\n",
                          argv[*aix] );
                  err = 1;
               }
               else   /* this following argument is taken to be a filename */
               {
                  Name = argv[*aix];

                  if ( Debug )  printf( "(Name: \"%s\")\n", Name );
               }
            }
            else if ( Debug )   /* no following filename argument */
            {
               printf( "(no filename follows \'%s\' option)\n",
                       argv[ *aix - 1 ] );
            }
         }
         else   /* not a recognized option */
         {
            printf( "  unrecognized option \"%s\"\n", argv[*aix] );
            err = 1;
         }
      }
      else    /* non-option command line argument */
      {
         if ( Pipe )    /* pipe operation precludes use of input files */
         {
            printf( "  invalid option (\"%s\"): input file is not valid"
                    " in pipe operations\n",
                    argv[*aix] );
            err = 1;
         }
         else
         {
            Name = argv[*aix];

            if ( Debug )  printf( "(Name: \"%s\")\n", Name );
         }
      }

      *aix = *aix + 1;    /* point to the next option (if any) */

      /* for pipes: don't check for a filename, just process ALL options */
      /* for files: break out of the loop as soon as a filename is found */

      if ( !Pipe  &&  Name )    /* filename argument found */
      {
         /* verify that this "filename" is not empty/blank */

         if ( !Name[0]  ||  isspace( Name[0] ) )
         {
            ln = strlen( Name );

            if ( Debug )
            {
               printf( "(Name at arg %i is %s, len: %i)%s\n",
                       *aix - 1, ( ln ? "blank" : "empty" ), ln,
                       ( ln ? " ...hex dump follows..." : "" ) );
 
               for ( i = 0;  i < ln;  i++ )    /* hex dump the Name */
               {
                  if ( i % 16 == 0 )    /* new line */
                  {
                     if ( i )  printf( " )\n" );    /* end previous line */
                     printf( "( %02X:", i );        /* start of new line */
                  }
                  printf( " %02X", Name[i] );
               }
               if ( i % 16 != 0 )  printf( " )\n" );    /* last line end */
            }

            /* report the error (and carry on) */

            if ( Files )  printf( "\n" );
            printf( "  skipped %s filename (argument %i)\n",
                    ( ln ? "blank" : "empty" ), *aix - 1 );

            Name = NULL;
         }
         else
         {
            break;    /* good to go with an actual name */
         }
      }
   }

   return ( err );
}


int  help_msg( int mx )
{
   /* just give us a blank line, and then the name, title, and version... */

   about_msg( -1 );

   /* ...and whatever else they want to know */

   if ( mx )    /* +help */
   {
      printf( "Example #1: pipe usage (output to stdout):\n" );
      printf( "\n" );
      printf( "   %% echo \"example pipe contents\" | %s\n", Pgm );
      printf( "\n" );
      printf( "       Dump of Pipe: (stdin)\n" );
      printf( "   00000000  65 78 61 6D 70 6C 65 20 70 69 70 65 20 63 6F 6E"
              "  |example pipe con|\n" );
      printf( "   00000010  74 65 6E 74 73 0A                              "
              "  |tents.          |\n" );
      printf( "       End-of-File   (22 bytes)\n" );
      printf( "\n" );
      printf( "   %% _\n" );
      printf( "\n" );
      printf( "Example #2: pipe usage (output to a file):\n" );
      printf( "\n" );
      printf( "   %% echo \"example pipe contents\" | %s -f\n", Pgm );
      printf( "\n" );
      printf( "       Dumped output (22 bytes) to file: %s.%s\n",
              DefPipe, DefExts );
      printf( "\n" );
      printf( "   %% _\n" );
      printf( "\n" );
      printf( "The output file created by Example #2 contains the same"
              " information that was\n" );
      printf( "produced by Example #1:\n" );
      printf( "\n" );
      printf( "   %% cat %s.%s\n", DefPipe, DefExts );
      printf( "       Dump of Pipe: (stdin)\n" );
      printf( "   00000000  65 78 61 6D 70 6C 65 20 70 69 70 65 20 63 6F 6E"
              "  |example pipe con|\n" );
      printf( "   00000010  74 65 6E 74 73 0A                              "
              "  |tents.          |\n" );
      printf( "       End-of-File   (22 bytes)\n" );
      printf( "   %% _\n" );
      printf( "\n" );
      printf( "Example #3: pipe usage (output to stdout as hex-only):\n" );
      printf( "\n" );
      printf( "   %% echo \"example pipe contents\" | %s -xo -i\n", Pgm );
      printf( "   65 78 61 6D 70 6C 65 20 70 69 70 65 20 63 6F 6E"
              " 74 65 6E 74 73 0A\n" );
      printf( "   %% _\n" );
      printf( "\n" );
      printf( "The output produced by Example #3 is a single continuous"
              " string that contains\n" );
      printf( "the series of blank-separated hex-digit pairs representing"
              " the bytes in the\n" );
      printf( "piped input to the %s utility.\n", Pgm );
      printf( "\n" );
      printf( "This can be verified by piping this output to %s again:\n",
              Pgm );
      printf( "\n" );
      printf( "   %% echo \"example pipe contents\" | %s -xo -i | %s\n",
              Pgm, Pgm );
      printf( "\n" );
      printf( "       Dump of Pipe: (stdin)\n" );
      printf( "   00000000  36 35 20 37 38 20 36 31 20 36 44 20 37 30 20 36"
              "  |65 78 61 6D 70 6|\n" );
      printf( "   00000010  43 20 36 35 20 32 30 20 37 30 20 36 39 20 37 30"
              "  |C 65 20 70 69 70|\n" );
      printf( "   00000020  20 36 35 20 32 30 20 36 33 20 36 46 20 36 45 20"
              "  | 65 20 63 6F 6E |\n" );
      printf( "   00000030  37 34 20 36 35 20 36 45 20 37 34 20 37 33 20 30"
              "  |74 65 6E 74 73 0|\n" );
      printf( "   00000040  41 20 0A                                       "
              "  |A .             |\n" );
      printf( "       End-of-File   (67 bytes)\n" );
      printf( "\n" );
      printf( "   %% _\n" );
      printf( "\n" );
      printf( "Note that the final two bytes in the original hex-only output"
              " are a blank\n" );
      printf( "(0x20) and a linefeed (0x0A).\n" );
   }
   else    /* -help */
   {
      printf( "Usage:  %s  [ [ options ]  [-|--]  [ file.ext ] ] ...\n", Pgm );
      printf( "   or:  echo \"example pipe contents\"  |  %s  [ options ]\n",
              Pgm );
      printf( "\n" );
      printf( "Options:\n" );
      printf( "      +# = start dump at byte # (default: start at first byte"
                          " in file: '+0')\n" );
      printf( "      -# = limit dump to # bytes (default: dump all bytes in"
                          " file: '-0')\n" );
      printf( "      -a = omit (-) or show (+) ASCII dump\n" );
      printf( "     -b# = set byte group to # bytes,"
                          " -b = 1 (default), +b = 2\n" );
      printf( "      -c = continuous byte dump as fixed-length lines (-)"
                          " or single string (+)\n" );
      printf( "    -e.# = set output file extension to # (default \"%s\")\n",
              DefExts );
      printf( "      -f = output to file: file.%s (-) or file.ext.%s (+)\n",
              DefExts, DefExts );
      printf( "    -f.# = output to file: file.#   (-) or file.ext.#   (+)\n" );
      printf( "    -f:# = output to file #.%s in current (-)"
                          " or input file\'s (+) directory\n", DefExts );
      printf( "    -f=# = output to file #     in current (-)"
                          " or input file\'s (+) directory\n" );
      printf( " -f:#.## = output to file #.##  in current (-)"
                          " or input file\'s (+) directory\n" );
      printf( " -f=#.## = output to file #.##  in current (-)"
                          " or input file\'s (+) directory\n" );
      printf( "           (the -f: and -f= options combine all outputs"
                          " into the named file)\n" );
      printf( "      -i = omit (-) or show (+) information headers\n" );
      printf( "      -l = use lowercase (-) or uppercase (+) ASCII digits"
                          " (default)\n" );
      printf( "      -n = omit (-) or show (+) line/address numbers\n" );
      printf( "     -n# = format line/address as #: s:short (default), l:long,"
                          " v:variable\n" );
      printf( "     -p# = dump # bytes per line (default 16,"
                          " '-p' is no limit)\n" );
      printf( "     -w# = set word group to # bytes, -w = 4, +w = 8\n" );
      printf( "      -x = omit (-) or show (+) hex digits dump\n" );
      printf( "      -X = emulate \'hexdump -C -v\' output format\n" );
      printf( "     -xo = hex-only dump: as bytes (-) or continuous (+)\n" );
      printf( "  -about = show about message\n" );
      printf( "  -debug = enable debug outputs\n" );
      printf( "   -help = show help message\n" );
      printf( "    -ver = show version message\n" );
      printf( "\n" );
      printf( "The %s utility reads the specified file(s), byte-by-byte,"
              " and outputs\n", Pgm );
      printf( "each byte in hexadecimal, with various output formatting"
              " options.\n" );
      printf( "\n" );
      printf( "If an isolated \'-\' or \'--\' is specified in the command"
              " line arguments, the\n" );
      printf( "very next argument is taken to be a filename.  This option"
              " allows access to\n" );
      printf( "filenames beginning with the \'-\' character.\n" );
      printf( "\n" );
      printf( "Multiple filenames may be specified, and options affect the"
              " dump output of\n" );
      printf( "all the files that follow.  If dumping from a pipe, all"
              " options affect the\n" );
      printf( "pipe dump output.  Input from a pipe overrides and precludes"
              " input from a\n" );
      printf( "file (or files).\n" );
   }

   printf( "\n" );

   return ( 411 );
}

