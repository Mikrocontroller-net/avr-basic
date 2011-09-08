////////////////////////////////////////////////////////////////////////////
//
//  uParse_cfg.h
//
//  Konfiguration des uParsers
//   - Grundeinstellungen (z.B. benutzte Teile)
//   - Funktionen (Aufrufe und Codecs)
//   - Befehlstexte
//
//  Abschnitte
//   - UPARSE_CONFIG          - Grundeinstellungen
//   - UPARSE_FUNCTION        - Aufzurufende Funktionen
//   - UPARSE_USER_DATA_TYPE  - Weitere Datentypen in Parameter-Element mit aufnehmen
//   - UPARSE_USER_CODEC      - Benutzer-Definierte Codecs (z.b. Bool, Datum/Zeit)
//   - UPARSE_ITEM            - Befehle die der Parser erkennen kann
//
//  Author  : Rene Böllhoff
//  Created : around Apr 10
//
//  (c) 2005-2011 by Rene Böllhoff - reneboellhoff (at) gmx (dot) de - 
//
//////////////////////////////////////////////////////////////////////////////
//
//  This source file may be used and distributed without
//  restriction provided that this copyright statement is not
//  removed from the file and that any derivative work contains
//  the original copyright notice and the associated disclaimer.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//                    
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public
//  License along with this source; if not, download it from
//  http://www.gnu.org/licenses/gpl.html  
//
////////////////////////////////////////////////////////////////////////////  
  #ifdef UPARSE_CONFIG

    #define PARSE_DATA_ELEMENTS         4     // max. Anzahl der Datenelemente
    #define PARSE_DATA_STR_LEN          16    // max. Länge der Namen/Strings

    // Benutze automatisch generierte Hilfe
    //#define PARSE_USES_HELP       
    // Zeichenausgabefunktion für Hilfe
    //#define PARSE_PRINT_HELP_CHAR(x)    printf ("%c", (x));
    
    // Benutze User-Definierte Codecs
    #define PARSE_USES_CODECS
    // Ignoriere Groß/Kleinschreibung
    #define PARSE_IGNORE_CASE

  #endif

  // Hier sind alle aufzurufenden Funktionen aufgelistet.
  // Diese werden bei func unter UPARSE_ITEM eingetragen.
  #ifdef UPARSE_FUNCTION
  //UPARSE_FUNCTION (name,                  function        ) 
    UPARSE_FUNCTION (ARG_PARAM,             vCmdArg         ) 
  #endif

  // Hier werden weitere Datentypen an die Parameter-Element-Union "gehängt"
  #ifdef UPARSE_USER_DATA_TYPE
  // single rReal;  // z.b.
  #endif

  // Hier werden eigene Codecs eingetragen
  #ifdef UPARSE_USER_CODEC
  //UPARSE_USER_CODEC (name,                  function        , displayname)
    UPARSE_USER_CODEC (CODEC_FILENAME,        cCodecFileName  , "filename")
    
  #endif

  // In den UPARSE_ITEM-Texten gibt es folgende steuerzeichen :
  // \x01 = zahl (z.b. -1, 0x34, -0b1100110, $1234, ...)
  // \x02 = name erstes zeichen alpha + '_', danach mit zahlen (z.b. _XY945, TEXT01, ...) 
  // \x03 = string (kompletten 8 bit) (also "hallo welt", "äöp")
  // alles ab \x04 -> eigene Codecs

  #ifdef UPARSE_ITEM
  //UPARSE_ITEM(name,       text,                                 func    ) 

    UPARSE_ITEM(INPUTFILE,  "-i\x04",                             ARG_PARAM)
    UPARSE_ITEM(OUTCFILE,   "-o\x04",                             ARG_PARAM)
    UPARSE_ITEM(OUTHFILE,   "-o\x04",                             ARG_PARAM)
    UPARSE_ITEM(NAME,       "-n\x02",                             ARG_PARAM)
    UPARSE_ITEM(VERBOSE,    "-v",                                 ARG_PARAM)
    UPARSE_ITEM(SUPERFAST,  "-sf",                                ARG_PARAM)

    
  #endif
