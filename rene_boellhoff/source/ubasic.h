/*
 * Copyright (c) 2006, Adam Dunkels
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
 *
 * ---------------------------------------------------------------------------
 * Source heavily modified by Rene Böllhoff (reneboellhoff@gmx.de); Jun 2010
 *
 *  - Removed DEBUG_PRINTF-Option
 *  - Added Char-Device-Functionality for flexible input
 *  - Added Compiler-Features & Changes for parallel use (Text/Bin)
 *  - Added Extensions for Access to C-Functions/Arrays/Variables
 *  - Added Preprocessor Generated Code with common Config (basic_cfg.h)
 *
 * ---------------------------------------------------------------------------
 */
#ifndef __UBASIC_H__
	#define __UBASIC_H__

		#ifndef BASIC_CONFIG_FILE
			#define BASIC_CONFIG_FILE "basic_cfg.h"
		#endif


		#define BASIC_CONFIG
			#include BASIC_CONFIG_FILE
		#undef BASIC_CONFIG

		#define COMMON_CONFIG
			#include BASIC_CONFIG_FILE
		#undef COMMON_CONFIG


		void ubasic_init					(I8 rmode);
		void ubasic_reset					(void);
		void ubasic_run						(void);
		I8	 ubasic_finished			(void);
		I16  ubasic_get_variable	(I8 varnum);
		void ubasic_set_variable	(I8 varnum, I16 value);

	#endif /* __UBASIC_H__ */
