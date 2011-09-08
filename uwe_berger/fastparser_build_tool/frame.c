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


// Prototypen

void vInitInputBuffer (void);
void vHandleKeyboard  (char cC);
void vProcessKey      (void);
void conprintf        (char *fmt, ...);
void vTimerProcess    (void);
void vExitLoop        (void);
char cLoop            (void);



// Timer Aufruf (Makro)

#define TIMER_MS_CALL   

// Funktionszeiger zum Verarbeiten einer Eingabe

T_PROCESS_INPUT_LINE pfProcessLine = NULL;

// zusätzliche Funktion in der Hauptschleife

#define MAIN_LOOP_FUNC  

// zusätzliche Initialisierung vor der Hauptschleife

#define MAIN_LOOP_INIT  





// uC-Timer Simulation

static long ulStateTime;      // Aktueller ms-Zähler zur Ausführungszeit
static long ulOldTime = 0;    // Letzter ms-Zähler-Wert


// Steuerung der Hauptschleife

char  bEndLoop        = 0;    // Wenn 1 wird Hauptschleife verlassen




// Eingabe-Shell

char aucLineInput [80];       // Eingabe-Zeile 
char cLinePtr;                // Eingabe-Zeiger


// Eingabe zurücksetzen
void vInitInputBuffer (void)
{
  memset (aucLineInput, 0, sizeof (aucLineInput));
  cLinePtr = 0;
}

// Einzelnes Zeichen verarbeiten
void vHandleKeyboard (char cC)
{
  switch (cC)
  {
    //////////////////////////////////////////////////////
    // Backspace/Delete -> Zeichen aus Eingabezeile 
    //                     löschen
    case 0x08 :
      if (cLinePtr > 0)
      {
        printf ("\x08 \x08");
        aucLineInput [cLinePtr--] = 0;
        aucLineInput [cLinePtr  ] = 0;
      }
      break;
    //////////////////////////////////////////////////////
    // Enter -> Eingabe abschließen und Verarbeitung 
    //          der Zeile aufrufen
    case 0x0d :
      printf ("\n\r");
      aucLineInput [cLinePtr  ] = 0;

      if (pfProcessLine)
      {
        // Verarbeitung erfolgreich ? -> dann "Ok" Ausgeben
        if (pfProcessLine (aucLineInput) == PLF_OK)
        {
          printf ("Ok\n\r");
        }
        // Verarbeitung fehlerhaft ? -> dann "Error" Ausgeben
        else
        {
          printf ("Error\n\r");
        }
      }
      // Eingabebuffer zurücksetzen
      vInitInputBuffer ();
      break;
    //////////////////////////////////////////////////////
    // ESC -> Flag setzen um Hauptschleife zu verlassen
    case 0x1b :
      bEndLoop = TRUE;
      break;
    //////////////////////////////////////////////////////
    // Tab -> in einzlnesSpace konvertieren
    case 0x09 :
      cC = 0x20;
    //////////////////////////////////////////////////////
    // Alle anderen Zeichen -> an Eingabezeile hängen
    default :
      printf ("%c", cC);
      aucLineInput [cLinePtr] = cC;
      if (cLinePtr < 80)
      {
        cLinePtr++;
      }
      else
      {
        printf ("\x08");
      }
      break;
  }
}

// Tastatur abfragen und Sonderzeichen ignorieren
void vProcessKey (void)
{
  char ucKey;
  
  if (kbhit ())
  {
    ucKey = getch ();

    // Sonderzeichen (F-Tasten, Cursor) ignorieren
    if ((ucKey == 0) || (ucKey == 0xe8))
    {
      getch ();
    }
    // ESC -> Hauptschleife verlassen
    else if (ucKey == 27)
    {
      vExitLoop ();
      return;
    }
    // Jedes andere Zeichen weiter verarbeiten
    else
    {
      vHandleKeyboard (ucKey);
    }
  }
}

// Prototyp für eine printf-Funktion mit variablen Parametern
void conprintf (char *fmt, ...)
{
  char buffer [1024];

  va_list vargs;
  va_start(vargs, fmt);
  vsprintf (buffer, fmt, vargs);
  va_end(vargs); 
  printf ("%s", buffer);
}

// Timer Simulation
// Es wird zwischen zwei vTimerProcess aufrufen geschaut wieviel Zeit (ms)
// vergangen ist und für jede ms ein Aufruf des TIMER_MS_CALL Makros
// gemacht. 
void vTimerProcess (void)
{
  long ulTime, ulDiff;

  if (ulOldTime == 0)
  {
    ulOldTime = GetTickCount();
  }
  ulTime = GetTickCount();

  if (ulOldTime != ulTime)
  {
    ulDiff = ulTime - ulOldTime;
    ulOldTime = ulTime;
    ulStateTime += ulDiff;
    while (ulDiff > 0) 
    { 
      TIMER_MS_CALL;
      ulDiff--;
    }
  }
}


// Funktionen für die Hauptschleife 

// Aufruf um Hauptschleife zu beenden (nur Flag setzen)
void vExitLoop (void)
{
  bEndLoop = TRUE;
}

// Hauptschleifen Funktion als einzelner Aufruf
char cLoop (void)
{
  vProcessKey   ();   // Tastatur verarbeiten
  vTimerProcess ();   // Timer verarbeiten
  MAIN_LOOP_FUNC;     // zusätzliche Verarbeitungen
  return bEndLoop;    // Flag zurückgeben
}

// Hauptschleife selbst
void vMainLoop (void)
{
  U8 bEnd = FALSE;

  // Titel setzen
  SetConsoleTitleA ("Console");

  // Eingabebuffer löschen
  vInitInputBuffer ();

  // zusätzliche Initialisierung
  MAIN_LOOP_INIT;

  // solange Flag FALSE ist Schleife ausführen
  while (bEnd == FALSE)
  {
    // Sleep (10) ist wichtig damit das OS diesen Task
    // einfacher unterbrechen kann und die CPU-Last
    // nicht auf 100% geht.
    Sleep (10);
    // Hauptschleifenfunktion (einzeln) aufrufen
    bEnd = cLoop (); 
  }
}

