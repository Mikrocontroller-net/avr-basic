/*
 * Copyright (c) 2011, Rene Böllhoff
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

  #include "lib.h"

// Dateinamen für Ein/Ausgabe und Name der Datenstruktur
char  aucInFile        [80];    // Eingabedatei Schlüsselwörter
char  aucOutCFile      [80];    // Ausgabedatei C
char  aucOutHFile      [80];    // Ausgabedatei H
U8    aucName          [80];    // Name der Schlüsselwort-Tabelle

// Flags
B8    bVerbose       = FALSE;   // Debug-Ausgaben
U8    cArgFlag      = 0;       // Argumente-Flags 

// Ein paar nützliche Makros
#define cRightChar(x,y)       x [strlen (x) - (y)]
#define vAppendChar(x,y)      x [strlen (x) + 1] = 0; x [strlen (x)] = (y);
#define PRINTF                if (bVerbose) printf


// Aufruf-Funktion vom Parser
PARSER_CALL (vCmdArg)
{
  GET_PAR_STR (pucPar1, 0);

  switch (ucCmd)
  {
    case INPUTFILE    : strcpy ((char *)aucInFile   , pucPar1); cArgFlag |= 1; break;
    case OUTCFILE     : strcpy ((char *)aucOutCFile , pucPar1); cArgFlag |= 2; break;
    case OUTHFILE     : strcpy ((char *)aucOutHFile , pucPar1); cArgFlag |= 8; break;
    case NAME         : strcpy ((char *)aucName     , pucPar1); cArgFlag |= 4; break;
    case SUPERFAST    : cArgFlag |= 16;                                break;
    case VERBOSE      : bVerbose = TRUE;                                break; 
  }
}

// Codec für Dateinamen
//  - Dieser Codec liefert den Text bis zum \0
PARSER_CODEC (cCodecFileName)
{
  char *pucString = *pucText;
  U16 uiLen = strlen (pucString);
  if ((uiLen > 0) && (uiLen < 80)) 
  { 
    strcpy ((char *)pstCmdData->aucText, pucString); 
    pucString += uiLen;
    *pucText = pucString;
    return 0; 
  }
  return -1; 
}


// FastParser-Daten inkl Infos als C-Tabelle plus defines und PROGMEM/RAM Attributen in Datei abspeichern
void vDumpToDataStruct (FILE *pstOutfile, U8 *aucName, T_NPARSER_WALK stParserStruct)
{
  U8  ucT, *pucBuffer = stParserStruct.pucParseTable;
  U16 uiC;

  // Unterscheidung Fast/Superfast-Parser 
  if (cArgFlag & 16)
  {
    // Unterscheidung 8/16-Bit
    if (stParserStruct.uc16)  { fprintf (pstOutfile, "\r\n/* 16bit-Addressierung */\r\n#define SFP_USES_16BIT\r\n\r\n");  } 
                        else  { fprintf (pstOutfile, "\r\n/* 8bit-Addressierung */\r\n#define SFP_USES_8BIT\r\n\r\n");    } 
  } 
  else 
  {
    // Unterscheidung 8/16-Bit
    if (stParserStruct.uc16)  { fprintf (pstOutfile, "\r\n/* 16bit-Addressierung */\r\n#define FP_USES_16BIT\r\n\r\n");   } 
                        else  { fprintf (pstOutfile, "\r\n/* 8bit-Addressierung */\r\n#define FP_USES_8BIT\r\n\r\n");     } 
  }
  
  // Byte-Array-Deklaration in Datei schreiben inkl.
  // Unterscheidung USE_PROGMEM für AVR-Zusatz PROGMEM
  fprintf (pstOutfile, "\r\n#if TOKENIZER_FASTPARSER\r\n");   
  fprintf (pstOutfile, "\r\n#if USE_PROGMEM\r\n");   
  fprintf (pstOutfile, "\r\nunsigned char auc%s [] PROGMEM = { \r\n", aucName); 
  fprintf (pstOutfile, "\r\n#else\r\n");     
  fprintf (pstOutfile, "\r\nunsigned char auc%s [] = { \r\n", aucName); 
  fprintf (pstOutfile, "\r\n#endif\r\n");     

  // Byte als Hex-Ausgeben und als Hexdump wegschreiben
  for (uiC = 0; uiC < stParserStruct.uiSize; uiC ++)
  {
    ucT = *pucBuffer++;
    // Anfang einer Zeile ? -> dann '  ' ausgeben
    if ((uiC & 15) == 0 ) { fprintf (pstOutfile, "  "); }
    // Byte immer ausgeben
    fprintf (pstOutfile, "0x%02x, ", ucT); 
    // Anfang einer Zeile ? -> dann CRLF ausgeben
    if ((uiC & 15) == 15) { fprintf (pstOutfile, "\r\n"); }
  }
  // ggf letztes CRLF ausgeben
  if ((uiC & 15) != 0) 
  { 
    fprintf (pstOutfile, "\r\n");
  }
  // Deklaration abschließen
  fprintf (pstOutfile, "  };\r\n");

  // das "#ifdef TOKENIZER_FASTPARSER" abschließen
  fprintf (pstOutfile, "\r\n#endif\r\n");     

}


// Baut C und H-Datei aus SChlüsselwort-Datei zusammen
char cBuildFiles (void)
{
  FILE *pstInFile = NULL, *pstOutFile = NULL;
  T_NPARSER_WALK stWalker;

  // Baum löschen
  vListClearEntries ();
  
  // Eingabedatei vorhanden ? 
  pstInFile  = fopen ((char *)aucInFile,  "rb+");
  if (!(pstInFile)) { return -1; }
  fclose (pstInFile);

  // C-Datei erstellen
  pstOutFile = fopen ((char *)aucOutCFile, "wb+");
  if (!(pstOutFile)) { return -2; }

  // Eingabedatei einlesen und Baum erzeugen
  uiLoadParserFile ((char *)aucInFile);
  // Superfastparser Tabelle erzeugen
  stWalker = stListBuild ();
  // wenn Verbose -> erzeugten Baum anzeigen
  if (bVerbose) vListPrintEntries ();

  // 
  vDumpToDataStruct (pstOutFile, aucName, stWalker);

  fclose (pstOutFile);

  pstOutFile = fopen (aucOutHFile, "wb+");
  if (!(pstOutFile)) { return -3; }

  uiMakeParserEnums (aucInFile, pstOutFile);

  //fprintf (pstOutFile, "\r\n\r\n  extern FMPRE T_NPARSER_WALK st%s;\r\n\r\n", aucName);

  fclose (pstOutFile);

  return 0;

}


void usage (void)
{
  printf ("usage : nParserBuild.exe -Iinfile -Ooutfile.h -Nname [-V] [-ACL]\r\n");
}

int main (int argc, char *argv [])
{

  if (argc < 3) 
  {
    usage ();
    return -1;
  }

  while (--argc > 0) 
  { 
    if ((strlen (argv [argc]) == 1) || (cParserProcess (argv [argc]) != 0))
    {
      usage ();
      printf ("invalid parameter : %s\n\r", argv [argc]);
      return -2;
    }
  }

  if ((cArgFlag & 7) != 7) { usage (); return -1; }
  
  if ((cRightChar (aucOutCFile, 1) != 'c') ||
      (cRightChar (aucOutCFile, 2) != '.')) 
  { 
    usage ();
    printf ("C Output File must end on \x22.c\x22\r\n");
    return -2;
  }
  
  if ((cArgFlag & 8) == 8) 
  {
    if ((cRightChar (aucOutHFile, 1) != 'h') ||
        (cRightChar (aucOutHFile, 2) != '.')) 
    { 
      usage ();
      printf ("H Output File must end on \x22.h\x22\r\n");
      return -2;
    }
  }
  else
  {
    strcpy (aucOutHFile, aucOutCFile);
    cRightChar (aucOutHFile, 1) = 'h';
  }
  
  switch (cBuildFiles ())
  {
    case  0 :
      printf ("Table(s) & Headerfile generated/appended\n\r");
      return 0;
    case -1 :
      usage ();
      //printf ("infile %s does not exist\n\r", argv[1]);
      printf ("infile %s does not exist\n\r", aucInFile);
      return -3;
      break;
    case -2 :
    case -3 :
      usage ();
      //printf ("outfile %s cannot be opened for write\n\r", argv[2]);
      printf ("outfile %s cannot be opened for write\n\r", aucOutHFile);
      return -4;
      break;
  }
  return -1;
}
