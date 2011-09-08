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

    #ifndef __NPARSEDATABUILD_H__
      #define __NPARSEDATABUILD_H__



        typedef struct
        {
          U8  *pucParseTable;
          U16 uiSize;
          U8  uc16;
          U8  ucFCs;
        } T_NPARSER_WALK;

        typedef char T_LINE [200];

        void    vListClearEntries   (void);
        void    vListAppendToken    (char *cString, U16 uiToken);
        void    vListPrintEntries   (void);
        void    vListBuildAndDump   (void);
        T_NPARSER_WALK stListBuild  (void);
        T_NPARSER_WALK stListBuild2 (void);
        U16     uiLoadParserFile    (char *pcFile);
        U16     uiMakeParserEnums   (char *pcFile, FILE *pstOutFile);
        void    vAppendListAsCArray (char *pcFile, char *aucName, FILE *pstOutFile);

      #endif

