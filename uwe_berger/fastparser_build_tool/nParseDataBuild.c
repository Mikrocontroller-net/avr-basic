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

//
// Typendefinition
//


// Typ für Baum-Element
//
typedef struct
{
  void *pvCharIsMet;  // nächster T_CHAR_LIST_ENTRY wenn Zeichen zutrifft (also zum neuen Ast)
  void *pvNext;       // nächster T_CHAR_LIST_ENTRY im Ast (also an der akt. Position)
  U32  ulIndex;       // Index des knotens (Hilfsindex)
  U32  ulIndexMet;    // Index des knotens (Zielindex, wird für Tabelle benötigt) 
  char cChar;         // Zeichen
  U16  uiToken;       // Token-Wert (wenn Terminierungszeichen)
  B8   bIsFirst;      // erstes Zeichen im String
} T_CHAR_LIST_ENTRY;


// Rückgabe-Typ für Ergebnis der rekursiven Suche im Baum
// 
typedef struct
{
  T_CHAR_LIST_ENTRY *pstGetEntry;
  U32               ulResultIndex;
} T_LIST_GET_IDX;


// Typ für Tabelleneintrag (16-bit Index, bei 8bit einfach nur Low-byte)
//
typedef struct
{
  U16   uiMet;
  char  cChar;
} T_FAST_PARSE_CHAR;





T_FAST_PARSE_CHAR *pstBuildList = NULL; // generierte Tabelle
T_CHAR_LIST_ENTRY *pstList      = NULL; // Baumstruktur (rekursiv)
U32               ulEntryCount  = 0;    // Anzahl einträge in Tabelle


//
// Prototypen
//


void vListPrintEntry        (T_CHAR_LIST_ENTRY *pstEntry);
void vListClearEntry        (T_CHAR_LIST_ENTRY *pstEntry);
void vListPrintTokenEntries (T_CHAR_LIST_ENTRY *pstEntry);
void vListSetIndex_Id       (T_CHAR_LIST_ENTRY *pstEntry, U32 *pulIdx);
void vListSetIndex_NextId   (T_CHAR_LIST_ENTRY *pstEntry, U32 *pulIdx);
void ulListGetIndex_Id      (T_CHAR_LIST_ENTRY *pstEntry, T_LIST_GET_IDX *pstIdx);
U32  ulListGetIndexId       (T_CHAR_LIST_ENTRY *pstEntry);



//
//  Das Prinzip ist folgendermaßen :
//
//
//  Es wird aus den Tokens ein Baum aus Zeichen 
//  aufgebaut. Dieser ist so organisiert ein Knoten 
//  einerseits auf Folgebuchstaben (Zeichenposition
//  der eingabe) andererseits auf weitere Knoten an
//  der aktuellen Position verweist.
//
//  Bsp :   
//
//  ADD      [A] ------> (D) ------> (D) -----> ADD  
//  ADC       |           |           |
//  AND       |           |          [C] -----> ADC
//  BSR       |           |
//  BSL       |          [N] ------> [D] -----> AND
//  MOVE      |
//           (B) ------> [S] ------> (R) -----> BSR
//            |                       |
//            |                      [L] -----> BSL
//            |             
//           [M] --> [O] --> [V] --> [E] -----> MOVE
//        
//         
//           ( ) weitere Zeichen (Knoten) folgen
//
//           [ ] Letzter Knoten (wenn Zeichen nicht
//               passt -> Fehler  
//           
//
//
//  Der Knackpunkt ist die Liste so zu erzeugen bzw 
//  umzuformen das lineare Listen 
//  von Buchstaben inkl Verweise bei gefundenen
//  Zeichen auf weitere Listen angelegt werden. 
//  Damit wird gewährleistet das man beim "eintreffen" 
//  eines neuen zeichens (zeichenweises suchen) man 
//  eine zumeist kleine lineare liste abzuarbeiten 
//  hat da sich die Suchmöglichkeiten mit 
//  fortschreitender Zeichenposition stark 
//  einschränken. 
//
//  Vorgehensweise
//
//  Der Baum wird so angelegt das an der aktuellen Position
//  des zu vergleichenden Schlüsselwortes immer nur diejenigen
//  Möglichkeiten übrigbleiben die dem bisherigen "Pfad folgen" 
//  konnten. 
//  Das Schlüsselwort "läuft" quasi v.l.n.r. einem Ast entlang, 
//  wobei die Elemente zur Rechten die Folge-Elemente, und die 
//  Elemente nach Unten die Alternativen darstellen. Gibt es 
//  "kein Unten" so kann die Suche abgebrochen werden.
//  In jedem Element wird beim Anlegen ein Hilfszeiger gesetzt, der 
//  fortlaufend nummeriert wird. Weiterhin wird die Information ermittelt 
//  ob es das erste Zeichen im Schlüsselwort ist. Damit wird aus dem 
//  ersten Zeichen eine Einsprung-Tabelle generiert. Mit dieser wird der 
//  Suchvorgang beschleunigt, da sich nach dem ersten Zeichen die 
//  Suchmöglichkeiten extrem einschränken.
//  Ist der Baum angelegt so werden die Hilfsindizes umsortiert
//  sodass diese den linearen Suchlisten entsprechen.
//  Danach werden die eigentlichen Zielindizes neu ermittelt.
//  Dann wird der Baum durchlaufen und die Zwischentabelle generiert.
//  Aus dieser Zwischentabelle wird die 8 bzw 16-bit Tabelle generiert.  
//  Zuletzt wird die Einsprungtabelle und deren Start-Ende-Zeichen
//  eingefügt und das Ganze dann exportiert
//




//
//  Baumelement anlegen
//

T_CHAR_LIST_ENTRY *stNewCharListEntry (char cChar)
{
  T_CHAR_LIST_ENTRY *pstNew = malloc (sizeof (T_CHAR_LIST_ENTRY));

  if (pstNew == NULL) { return NULL; }

  memset (pstNew, 0, sizeof (T_CHAR_LIST_ENTRY));

  pstNew->cChar       = cChar;
  pstNew->uiToken     = 0;
  pstNew->pvCharIsMet = NULL;
  pstNew->pvNext      = NULL;
  pstNew->ulIndex     = ulEntryCount;
  pstNew->bIsFirst    = FALSE;
  ulEntryCount++;

  return pstNew;
}


//
//  Fügt einen String in den Baum ein und setzt 
//  das Token am Ende der Zeichenkette
//

void vListAppendToken (char *cString, U16 uiToken)
{
  T_CHAR_LIST_ENTRY *pstWalk = pstList, *pstNew;
  unsigned char cCnt = 0;

  // Leerstrings ignorieren
  if (strlen (cString) == 0) { return; }

  // Erstes Zeichen im Baum "sonderbehandeln"
  if (pstWalk == NULL) { pstWalk = pstList = stNewCharListEntry (*cString); pstWalk->bIsFirst = TRUE; }

  do
  {
    // Stimmt an der aktuellen Baumposition
    // das aktuelle Zeichen im String überein ?
    if (pstWalk->cChar == *cString)
    {
      // Ja ? Dann prüfe ob nächstes Zeichen im String das Ende ist ...
      cString++;
      cCnt++;
      // Ist der einzufügende String zuende ?
      if (*cString == 0)
      {
        // wenn Baum auch zuende -> dann letztes Element 
        // erzeugen, Tokenwert setzen und anhängen
        if (pstWalk->pvCharIsMet == NULL)
        {
          pstNew = stNewCharListEntry (*cString);
          pstNew->bIsFirst      = (cCnt == 0);
          pstWalk->ulIndexMet   = pstNew->ulIndex;
          pstWalk->pvCharIsMet  = pstNew;
          pstWalk               = pstNew;
          pstWalk->uiToken      = uiToken;
        }
        else
        {
          // Wenn Baum weitergeht aber String zuende ist -> Spezialfall
          // Hintergrund : z.b. 'input' und 'in' als Schlüsselwort anlegen
          // input beinhaltet in und es gibt keine Möglichkeit direkt zu erkennen ob
          // es input, in oder ein Fehler ist. Dazu benötigt man ein spezielles 
          // Zeichen (0x01) welches am Ende des Astes platziert werden muß damit vorher alle
          // Möglichkeiten geprüft werden können. Sollte dann ein anderes Zeichen
          // Zuerst dem Zeichen folgen (also auf Zeichen "hinter" dem String)
          pstWalk = pstWalk->pvCharIsMet;
          pstNew = stNewCharListEntry (0x01);
          pstNew->bIsFirst      = (cCnt == 0);
          // ggf Liste bis zum letzten Element des Astes durchlaufen
          while (pstWalk->pvNext != NULL) { pstWalk = pstWalk->pvNext; }
          // und da anhängen (damit alle anderen Elemente noch "drankommen" können)
          pstWalk->ulIndexMet = pstNew->ulIndex;
          pstWalk->pvNext     = pstNew;
          pstWalk             = pstNew;
          pstWalk->uiToken      = uiToken;

          // String abschließen
          pstNew = stNewCharListEntry (0x00);
          pstWalk->ulIndexMet   = pstNew->ulIndex;
          pstWalk->pvCharIsMet  = pstNew;
          pstWalk               = pstNew;
          pstWalk->uiToken      = uiToken;

        }
        return;
      }
      // nicht das Ende ? dann lege ggf ein neues Zeichen innerhalb 
      //                  des Astes an.
      else
      {
        // kein weiteres Zeichen im Baum ? dann lege weiteren Ast an
        if (pstWalk->pvCharIsMet == NULL)
        {
          pstNew = stNewCharListEntry (*cString);
          pstNew->bIsFirst      = (cCnt == 0);
          pstWalk->ulIndexMet   = pstNew->ulIndex;
          pstWalk->pvCharIsMet  = pstNew;
          pstWalk               = pstNew;
        }
        else
        {
          // wenn noch Zeichen im Baum, dann mit dem String "mitlaufen"
          pstWalk = pstWalk->pvCharIsMet;
        }
      }
    }
    // keine übereinstimmung ? dann lege ggf ein neues zeichen in der
    // (vertikalen) liste ein.
    else
    {
      //
      if (pstWalk->pvNext == NULL)
      {
        pstNew = stNewCharListEntry (*cString);
        pstNew->bIsFirst      = (cCnt == 0);
        pstWalk->ulIndexMet = pstNew->ulIndex;
        pstWalk->pvNext     = pstNew;
        pstWalk             = pstNew;
      }
      else
      {
        pstWalk = pstWalk->pvNext;
      }
    }
  } while (*cString != 0);
}




//
//  "Template" für rekursive Operationen. 
//  Wenn man durch den Baum laufen will muß man folgendermaßen
//  vorgehen :
//
//
//  void vRecursiveWalker (T_CHAR_LIST_ENTRY *pstEntry, ...)
//  {
//    // prüfen ob eintrag null ist
//    if (pstEntry == NULL) { return; } 
//
//    // hier den such/modifizierungscode reinpacken
//
//    // letztes blatt ? (keine weiteren verweise)
//    if ((pstEntry->pvCharIsMet == NULL)   &&
//        (pstEntry->pvNext      == NULL))  { return; }
//    // ansonsten liste erst vertikal durchlaufen (also alle möglichen Zeichen im Ast)
//    if ((pstEntry->pvNext      != NULL))  { vRecursiveWalker (pstEntry->pvNext, ...);      }  
//    // und dann horizontal (quasi das Schlüsselwort "ablaufen")
//    if ((pstEntry->pvCharIsMet != NULL))  { vRecursiveWalker (pstEntry->pvCharIsMet, ...); }
//  }
//


//
//  Löschen ist die einfachste aller Rekursionen
//

void vListClearEntry (T_CHAR_LIST_ENTRY *pstEntry)
{
  // Eintrag Null ? -> Ende
  if (pstEntry == NULL) { return; }
  // Wenn letzter Eintrag -> aktuelles Element löschen
  if ((pstEntry->pvCharIsMet == NULL)   &&
      (pstEntry->pvNext      == NULL))  { free (pstEntry); return; }
  // ansonsten durchlaufen
  if ((pstEntry->pvNext      != NULL))  { vListClearEntry (pstEntry->pvNext);      }
  if ((pstEntry->pvCharIsMet != NULL))  { vListClearEntry (pstEntry->pvCharIsMet); }
}

//
//  Funktion zum Löschen des Baumes und der vorbereiteten Liste
//

void vListClearEntries (void)
{
  // Rekursion zum Löschen aufrufen
  vListClearEntry (pstList);
  // Variablen initialisieren
  pstList = NULL; ulEntryCount = 0;
  // Wenn Ausgabetabelle existiert -> löschen
  if (pstBuildList) { free (pstBuildList); pstBuildList = NULL; }
}

//
//  Anzahl "effektiver" Elemente im Baum zählen
//  (nicht das erste Zeichen, da Einsprungtabelle)
//

void vListCountRealEntries (T_CHAR_LIST_ENTRY *pstEntry, int *piCounter)
{
  if (pstEntry == NULL) { return; }

  // Wenn nicht erstes Zeichen -> Zähler erhöhen
  if (!pstEntry->bIsFirst) { (*piCounter) ++; }

  // Standard Rekursion
  if ((pstEntry->pvNext      != NULL))  { vListCountRealEntries (pstEntry->pvNext, piCounter);      }
  if ((pstEntry->pvCharIsMet != NULL))  { vListCountRealEntries (pstEntry->pvCharIsMet, piCounter); }
}

//
// Funktionsaufruf um Anzahl Baumelemente zu zählen
//

int iListCountRealEntries (T_CHAR_LIST_ENTRY *pstEntry)
{
  int iCount = 0;
  vListCountRealEntries (pstEntry, &iCount);
  return iCount;
}


//
//  Rekursion zum Auflisten der gesamten Struktur
//

void vListPrintEntry (T_CHAR_LIST_ENTRY *pstEntry)
{
  if (pstEntry == NULL) { return; }
  printf ("C:%08lx I:%04x '%c' -> T:%02x F:%d I:%04x M:%08lx N:%08lx\n\r",
          (long unsigned int)pstEntry,
          pstEntry->ulIndex & 0xffff,
          pstEntry->cChar,
          pstEntry->uiToken,
          pstEntry->bIsFirst ? 1 : 0,
          pstEntry->ulIndexMet & 0xffff,
          (long unsigned int)pstEntry->pvCharIsMet,
          (long unsigned int)pstEntry->pvNext);

  if ((pstEntry->pvNext      != NULL))  { vListPrintEntry (pstEntry->pvNext);      }
  if ((pstEntry->pvCharIsMet != NULL))  { vListPrintEntry (pstEntry->pvCharIsMet); }
}


//
//  Funktion zum Auflisten der gesamten Struktur
//

void vListPrintEntries (void)
{
  printf ("%d entries\n\r", ulEntryCount);
  vListPrintEntry (pstList);
}

//
// Ermittelt das niedrigste und höchste erste Zeichen (um Einsprungtabelle zu kürzen)
//

void vListGetMinMaxChar (T_CHAR_LIST_ENTRY *pstEntry, unsigned char *pucMinChar, unsigned char *pucMaxChar)
{
  unsigned char cMinChar = 0xff, cMaxChar = 0;

  // Wenn Baum leer -> nix machen
  if (pstEntry == NULL) { return; }

  // Die "ersten" Zeichen zu ermitteln ist einfach ...
  // einfach dem ersten Ast folgen
  do
  {
    // Solange Laufen möglich ist ...
    if (pstEntry)
    {
      // Zeichen auf Min/Max-Prüfen
      if (pstEntry->cChar < cMinChar) { cMinChar = pstEntry->cChar; }
      if (pstEntry->cChar > cMaxChar) { cMaxChar = pstEntry->cChar; }
    }
    // nächstes Element im Ast
    pstEntry = pstEntry->pvNext;
    // solange Elemente ...
  } while (pstEntry);

  // Ergebnisse zurückgeben
  *pucMinChar = cMinChar;
  *pucMaxChar = cMaxChar;
}




//
//  Binäre Liste vorbereiten
//


//
// Durchläuft den Baum und erzeugt die Tabelle
//

void vListDropEntry (T_CHAR_LIST_ENTRY *pstEntry, T_FAST_PARSE_CHAR *pstListPtr)
{
  T_FAST_PARSE_CHAR stEntry;

  // NULL ? -> nichts machen
  if (pstEntry == NULL)   { return; }

  // solange es nicht das erste Zeichen ist -> nicht beachten
  if (!pstEntry->bIsFirst)
  {
    // bei "Zeichen gefunden" Tabelleneintrag setzen
    if (pstEntry->ulIndexMet != (U32) -1)
    {
      // Zeichen und Folge-Index speichern
      stEntry.cChar = pstEntry->cChar;
      stEntry.uiMet = pstEntry->ulIndexMet;
      // wenn letztes Element im Ast dann End-Flag setzen
      if (!(pstEntry->pvNext))
      {
        stEntry.cChar |= 0x80;
      }
    }
    // ansonsten ist es das Token selbst ->
    // Token in Tabelleneintrag setzen
    else
    {
      stEntry.cChar = 0;
      stEntry.uiMet = pstEntry->uiToken;
    }
    // Tabelleneintrag wegspeichern
    memcpy (pstBuildList, &stEntry, sizeof (T_FAST_PARSE_CHAR));
    pstBuildList++;
  }

  // Rekursion wie gehabt
  if ((pstEntry->pvNext      != NULL))  { vListDropEntry (pstEntry->pvNext     , pstListPtr); }
  if ((pstEntry->pvCharIsMet != NULL))  { vListDropEntry (pstEntry->pvCharIsMet, pstListPtr); }
}


//
// Rekursion zur Suche im Baum nach einem Eintrag
//

void ulListGetIndex_Id (T_CHAR_LIST_ENTRY *pstEntry, T_LIST_GET_IDX *pstIdx)
{
  // NULL ? -> nichts machen
  if (pstEntry == NULL) { return; }

  // wenn gesuchter Eintrag
  if (pstIdx->pstGetEntry == pstEntry)
  {
    // dann liefere den Index zurück
    pstIdx->ulResultIndex = pstEntry->ulIndex;
    return;
  }

  // Rekursion wie gehabt
  if ((pstEntry->pvNext      != NULL))  { ulListGetIndex_Id (pstEntry->pvNext,       pstIdx); }
  if ((pstEntry->pvCharIsMet != NULL))  { ulListGetIndex_Id (pstEntry->pvCharIsMet,  pstIdx); }
}


U32 ulListGetIndexId (T_CHAR_LIST_ENTRY *pstEntry)
{
  // Suchstruktur initialisieren
  T_LIST_GET_IDX stGetIdx;
  stGetIdx.ulResultIndex  = -1;
  stGetIdx.pstGetEntry    = pstEntry;
  // und suchen
  ulListGetIndex_Id    (pstList, &stGetIdx);
  return stGetIdx.ulResultIndex;
}

//
// Sortiert die Indizes neu
//

void vListSetIndex_Id (T_CHAR_LIST_ENTRY *pstEntry, U32 *pulIdx)
{
  if (pstEntry == NULL) { return; }

  // Wenn erstes Zeichen -> Sortierung aussetzen
  if (pstEntry->bIsFirst)
  {
    pstEntry->ulIndex = -1;
  }
  // Wenn nicht erstes Zeichen -> Index setzen und erhöhen
  else
  {
    pstEntry->ulIndex = *pulIdx; (*pulIdx)++;
  }
  // Rekursion wie gehabt
  if ((pstEntry->pvNext      != NULL))  { vListSetIndex_Id (pstEntry->pvNext, pulIdx);      }
  if ((pstEntry->pvCharIsMet != NULL))  { vListSetIndex_Id (pstEntry->pvCharIsMet, pulIdx); }
}

//
// Ziel-Indizes erzeugen
//

void vListSetIndex_NextId (T_CHAR_LIST_ENTRY *pstEntry, U32 *pulIdx)
{
  if (pstEntry == NULL) { return; }
  
  // nach Rechtem Element im Ast suchen und setzen
  pstEntry->ulIndexMet = ulListGetIndexId (pstEntry->pvCharIsMet);
  
  // Rekursion wie gehabt
  if ((pstEntry->pvNext      != NULL))  { vListSetIndex_NextId (pstEntry->pvNext, pulIdx);      }
  if ((pstEntry->pvCharIsMet != NULL))  { vListSetIndex_NextId (pstEntry->pvCharIsMet, pulIdx); }
}

//
// Speicher allokieren und Tabelle erzeugen
//

T_FAST_PARSE_CHAR *pstListBuildList (void)
{
  T_FAST_PARSE_CHAR *pstOutputList = NULL;
  int iCount = iListCountRealEntries (pstList);

  pstOutputList = pstBuildList = malloc (sizeof (T_FAST_PARSE_CHAR) * iCount);

  if (pstList) { vListDropEntry (pstList, pstOutputList); }

  pstBuildList = pstOutputList;

  return pstOutputList;
}



#define TABSTARTOFFS  2

T_NPARSER_WALK stListBuild (void)
{
  T_NPARSER_WALK stWalk;
  T_FAST_PARSE_CHAR *pstOutList;
  U16 uiC, uiS;
  U8  *pucD, *pucB, uc16;
  U32 ulIdx = 0;
  unsigned char cMinChar, cMaxChar, cFirstChars;
  int iEntries;
  T_CHAR_LIST_ENTRY *pstWalk = pstList;

  stWalk.pucParseTable  = NULL;
  stWalk.uc16           = 0;
  stWalk.uiSize         = 0;


  iEntries = iListCountRealEntries (pstList);

  vListGetMinMaxChar (pstList, &cMinChar, &cMaxChar);
  cMaxChar++;

  cFirstChars = cMaxChar - cMinChar;
  

  vListSetIndex_Id      (pstList, &ulIdx);
  vListSetIndex_NextId  (pstList, &ulIdx);

  pstOutList = pstListBuildList ();

  if (pstOutList == NULL) { return stWalk; }
  
  uc16 = iEntries > 255 ? 1               : 0               ;

  uiS  =  TABSTARTOFFS + 
         (uc16           ? cFirstChars * 2 : cFirstChars * 1) + 
         (uc16           ? iEntries * 3    : iEntries * 2   );

  pucB = pucD = malloc (uiS);
  memset (pucB, 0, uiS);
  memset (pucB, 255, TABSTARTOFFS + (uc16 ? cFirstChars * 2 : cFirstChars * 1));


  pucD = &(pucD [TABSTARTOFFS + (uc16 ? cFirstChars * 2 : cFirstChars * 1 )]);

  for (uiC = 0; uiC < iEntries; uiC ++)
  {
                *pucD =  (pstOutList [uiC].cChar);              pucD++;
                *pucD = ((pstOutList [uiC].uiMet) >> 0) & 0xff; pucD++;
    if (uc16) { *pucD = ((pstOutList [uiC].uiMet) >> 8) & 0xff; pucD++; }
  }

  pucD = &(pucB [TABSTARTOFFS]);

  do
  {

    if (pstWalk)
    {
      pucD = &(pucB [TABSTARTOFFS + (((unsigned short) (pstWalk->cChar - cMinChar)) * (uc16 ? 2 : 1))]);
                  *pucD++ = ((pstWalk->ulIndexMet >> 0) & 0xff);
      if (uc16) { *pucD++ = ((pstWalk->ulIndexMet >> 8) & 0xff); }
    }
    pstWalk = pstWalk->pvNext;
  } while (pstWalk);


  pucD = pucB;

  *pucD++ = cMinChar;
  *pucD++ = cMaxChar;
//  *pucD++ = uc16;

  stWalk.pucParseTable  = pucB;
  stWalk.uiSize         = uiS;
  stWalk.uc16           = uc16;
  stWalk.ucFCs          = cFirstChars;
  return stWalk;
}


//
//
//


void vAppendLine (T_LINE **ppstLines, U16 *puiIdx, T_LINE stLine)
{
  T_LINE *pstLines = *ppstLines;
  (*puiIdx)++;
  pstLines = realloc (pstLines, (*puiIdx) * sizeof (T_LINE));
  memset (&pstLines [*puiIdx], 0, sizeof (T_LINE));
  memcpy (&pstLines [*puiIdx], &stLine, sizeof (T_LINE));
  *ppstLines = pstLines;
}


int charisalpha (char x)
{
  return ((x >= 'A') && (x <= 'Z')) || ((x >= 'a') && (x <= 'z'));
}

int charisnum (char x)
{
  return ((x >= '0') && (x <= '9'));
}


// fischt sich aus der text-datei die tokens (also hinter einem =) 
// und baut daraus den baum auf. 
// das enum-handling (hochzählen und setzen) wird berücksichtigt

U16 uiLoadParserFile (char *pcFile)
{
  FILE *stFile;
  T_LINE stLine;
  U16 uiToken = 0;
  U8  ucLP, ucCh, ucD;
  char cS = 1;

  vListClearEntries (); 

  stFile = fopen (pcFile, "rb+");
  if (stFile)
  {
    fseek (stFile, 0, 0);
    do
    {
      memset (stLine , 0, sizeof (stLine));
      ucLP = 0; cS = 1;
      do
      {
        ucCh = fgetc (stFile);
        switch (cS)
        {
          // alphanumerische zeichen als enum
          case 1 :
            if (ucCh == 0x0a)             { break; }
            else if (charisalpha (ucCh))  { cS =  2; }
            else                          { cS = -1; }
            break;
          // alphanumerische zeichen als enum bis , oder =
          case 2 :
            if (charisalpha (ucCh))       { cS =  2; }
            else if (charisnum (ucCh))    { cS =  2; }
            else if (ucCh == '_')         { cS =  2; }
            else if (ucCh == ',')         { cS =  3; }
            else if (ucCh == '=')         { cS = 11; }
            else if (ucCh == 0x0d)        { cS = 0;  }
            else                          { cS = -1; }
            break;

          // numerische zeichen als enum-wert bis =
          case 3 :
            if (charisnum (ucCh))         { cS = 3; uiToken *= 10; uiToken += (ucCh - '0'); }
            else if (ucCh == '=')         { cS = 11; }
            else if (ucCh == 0x0d)        { cS = 0;  }
            else                          { cS = -1; }
            break;


          case 11 :
            if      (ucCh == 0x0a)        { break; }
            else if (ucCh == 0x0d)        { if (ucLP > 0) { stLine [ucLP++] = 0; cS = 0; } }
            else if (ucCh <  0x20)        { break; }
            else if (ucCh >  0x7f)        { break; }
            else if (ucCh == '\\')        { cS = 12; }
            else                          { stLine [ucLP++] = ucCh; }
            break;
          case 12 :
            if      (ucCh == 'x')         { cS = 13; }
            else                          { stLine [ucLP++] = '\\'; stLine [ucLP++] = ucCh; cS = 1; }
            break;
          case 13 :
            ucD = 0;
            if     (((ucCh >= '0') && (ucCh <= '9')) ||
                    ((ucCh >= 'a') && (ucCh <= 'f')) || 
                    ((ucCh >= 'A') && (ucCh <= 'F'))) { 
              cS    = 14;
              ucD  |= (ucCh >= 'a' ? ucCh + 10 - 'a' : 
                       ucCh >= 'A' ? ucCh + 10 - 'A' : ucCh - '0');
              stLine [ucLP] = ucD;
            }
            else
            {
              cS = -1;
            }
            break;
          case 14 :
            if     (((ucCh >= '0') && (ucCh <= '9')) ||
                    ((ucCh >= 'a') && (ucCh <= 'f')) || 
                    ((ucCh >= 'A') && (ucCh <= 'F'))) {
              cS    = 11;
              ucD   = ucD << 4;
              ucD  |= (ucCh >= 'a' ? ucCh + 10 - 'a' : 
                       ucCh >= 'A' ? ucCh + 10 - 'A' : ucCh - '0') << 0;
              stLine [ucLP++] = ucD;
            }
            else
            {
              cS = -1;
            }
            break;
        }
      } while (!((cS <= 0) || (feof (stFile))));

      if (strlen (stLine) > 0) { vListAppendToken ((char *)stLine, uiToken); }
      uiToken++;

    } while (!((cS != 0) || (feof (stFile))));
  }
 
  if (stFile) { fclose (stFile); }

  return uiToken;
}

// fischt sich aus der text-datei die token-namen und schreibt sie als
// enum-auflistung weg
// das enum-handling (hochzählen und setzen) wird berücksichtigt

U16 uiMakeParserEnums (char *pcFile, FILE *pstOutFile)
{
  FILE *stFile;
  T_LINE stLine;
  U16 uiToken = 0;
  U8  ucLP, ucCh, ucD, ucCO = 0;
  char cS = 1;

  vListClearEntries (); 

  stFile = fopen (pcFile, "rb+");
  if (stFile)
  {
    fprintf (pstOutFile, "enum {\r\n  ");

    fseek (stFile, 0, 0);
    do
    {
      memset (stLine , 0, sizeof (stLine));
      ucLP = 0; cS = 1; ucCO = 0; 
      do
      {
        ucCh = fgetc (stFile);
        switch (cS)
        {
          // alphanumerische zeichen als enum
          case 1 :
            if (ucCh == 0x0a)             { break; }
            else if (charisalpha (ucCh))  { cS =  2; fprintf (pstOutFile, "%c", ucCh); ucCO = 1; }
            else                          { cS = -1; }
            break;
          // alphanumerische zeichen als enum bis , oder =
          case 2 :
            if (charisalpha (ucCh))       { cS =  2; fprintf (pstOutFile, "%c", ucCh); ucCO = 1; }
            else if (charisnum (ucCh))    { cS =  2; fprintf (pstOutFile, "%c", ucCh); ucCO = 1; }
            else if (ucCh == '_')         { cS =  2; fprintf (pstOutFile, "%c", ucCh); ucCO = 1; }
            else if (ucCh == ',')         { cS =  3; }
            else if (ucCh == '=')         { cS = 11; }
            else if (ucCh == 0x0d)        { cS = 0;  }
            else                          { cS = -1; }
            break;

          // numerische zeichen als enum-wert bis =
          case 3 :
            if (charisnum (ucCh))         { cS = 3;  uiToken *= 10; uiToken += (ucCh - '0'); }
            else if (ucCh == '=')         { cS = 11; fprintf (pstOutFile, " = %d", uiToken); }
            else if (ucCh == 0x0d)        { cS = 0;  }
            else                          { cS = -1; }
            break;


          case 11 :
            if      (ucCh == 0x0a)        { break; }
            else if (ucCh == 0x0d)        { if (ucLP > 0) { stLine [ucLP++] = 0; cS = 0; } }
            else if (ucCh <  0x20)        { break; }
            else if (ucCh >  0x7f)        { break; }
            else if (ucCh == '\\')        { cS = 12; }
            else                          { stLine [ucLP++] = ucCh; }
            break;
          case 12 :
            if      (ucCh == 'x')         { cS = 13; }
            else                          { stLine [ucLP++] = '\\'; stLine [ucLP++] = ucCh; cS = 1; }
            break;
          case 13 :
            ucD = 0;
            if     (((ucCh >= '0') && (ucCh <= '9')) ||
                    ((ucCh >= 'a') && (ucCh <= 'f')) ||
                    ((ucCh >= 'A') && (ucCh <= 'F'))) {
              cS = 14;
              ucD |= (ucCh >= 'a' ? ucCh + 10 - 'a' : 
                      ucCh >= 'A' ? ucCh + 10 - 'A' : ucCh - '0');
              stLine [ucLP] = ucD;
            }
            else
            {
              cS = -1;
            }
            break;
          case 14 :
            if     (((ucCh >= '0') && (ucCh <= '9')) ||
                    ((ucCh >= 'a') && (ucCh <= 'f')) ||
                    ((ucCh >= 'A') && (ucCh <= 'F'))) {
              cS = 11;
              ucD = ucD << 4;
              ucD |= (ucCh >= 'a' ? ucCh + 10 - 'a' :
                      ucCh >= 'A' ? ucCh + 10 - 'A' : ucCh - '0') << 0;
              stLine [ucLP++] = ucD;
            }
            else
            {
              cS = -1;
            }
            break;
        }
      } while (!((cS <= 0) || (feof (stFile))));

      if (ucCO) { fprintf (pstOutFile, ",\r\n  "); }
      uiToken++;

    } while (!((cS != 0) || (feof (stFile))));

    fprintf (pstOutFile, "};\r\n\r\n");
  }

  if (stFile) { fclose (stFile); }

  return 0;
}

