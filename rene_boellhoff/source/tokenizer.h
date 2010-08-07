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

	#ifndef __TOKENIZER_H__
		#define __TOKENIZER_H__


			#ifndef BASIC_CONFIG_FILE
				#define BASIC_CONFIG_FILE "basic_cfg.h"
			#endif

			// Gemeinsam verwendete Einstellungen/Konfigurationen einblenden
			#define COMMON_CONFIG
				#include BASIC_CONFIG_FILE
			#undef COMMON_CONFIG

			// Für Tokenizer verwendete Einstellungen/Konfigurationen einblende
			#define TOKENIZER_CONFIG
				#include BASIC_CONFIG_FILE
			#undef TOKENIZER_CONFIG

			// Für Compilerunterstützung verwendete Einstellungen/Konfigurationen einblende
#ifdef BASIC_USES_COMPILER
			#define COMPILER_CONFIG
				#include BASIC_CONFIG_FILE
			#undef COMPILER_CONFIG
#endif

			// 
			typedef struct
			{
				char type;
				union {
					char text [MAX_USER_KEYWORD_LEN];
					int  value;
				};
			} T_USER_FUNC_DATA;


			typedef T_USER_FUNC_DATA T_USER_FUNC_DATAS [MAX_USER_PARAMS];

			typedef void (* T_BASIC_FUNCTION)		(void);
			typedef I16  (* T_BASIC_EXPR_FUNC)	(I16 a, I16 b);
			
			typedef int  (* T_BASIC_USER_FUNC)	(T_USER_FUNC_DATAS stDatas);
			typedef int  (* T_BASIC_USER_ARRF)	(int index);

#ifdef BASIC_USES_CHARDEVICE
			enum {
				#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) name,
					#include BASIC_CONFIG_FILE
				#undef TOKENIZER_MEDIUM
				TOKENIZER_NUM_CHARDEVICES
			};

			#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) extern void st (void *pvData, CPOS len);
				#include BASIC_CONFIG_FILE
			#undef TOKENIZER_MEDIUM

			#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) extern char cc (void);
				#include BASIC_CONFIG_FILE
			#undef TOKENIZER_MEDIUM

			#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) extern void nc (void);
				#include BASIC_CONFIG_FILE
			#undef TOKENIZER_MEDIUM

			#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) extern CPOS gp (void);
				#include BASIC_CONFIG_FILE
			#undef TOKENIZER_MEDIUM

			#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) extern void sp (CPOS stRel);
				#include BASIC_CONFIG_FILE
			#undef TOKENIZER_MEDIUM

#endif


			enum {
				#define BASIC_ERROR(name,text) name,
					#include BASIC_CONFIG_FILE
				#undef BASIC_ERROR
				TOKENIZER_NUMBER_ERRORS
			};



			#define BASIC_TOKENS

				enum
				{
					TOKENIZER_ERROR,
					TOKENIZER_ENDOFINPUT,
					TOKENIZER_NUMBER,
					TOKENIZER_STRING,
#ifdef BASIC_USES_EXTENSIONS
					TOKENIZER_NAME,
#endif
					TOKENIZER_VARIABLE,
					TOKENIZER_KEYWORDS = TOKENIZER_VARIABLE,
					

					#define TOKEN_WORD(name,text,func) name,
					#define TOKEN_CHAR(name,char)
						#include BASIC_CONFIG_FILE
					#undef TOKEN_WORD
					#undef TOKEN_CHAR

					TOKENIZER_KEYCHARS, 
					TOKENIZER_KEYCHARS_1 = TOKENIZER_KEYCHARS - 1,

					#define TOKEN_WORD(name,text,func) 
					#define TOKEN_CHAR(name,char)				name,
						#include BASIC_CONFIG_FILE
					#undef TOKEN_WORD
					#undef TOKEN_CHAR


					TOKENIZER_NUM_TOKENS
				};

				#define NO_FUNCTION		dummyfunc
				#define NO_EXPR_FUNC	dummyexprfunc

				#define TOKEN_WORD(name,text,func) 							extern void func (void);
				#define TOKEN_CHAR(name,char)
					#include BASIC_CONFIG_FILE
				#undef TOKEN_WORD
				#undef TOKEN_CHAR

			#undef BASIC_TOKENS

#ifdef BASIC_USES_FUNC_AS_OPS
			#define BASIC_TOKEN_OPS

				#define TOKEN_CHAR_TERM(token,op,func)					extern I16 func (I16 a, I16 b);
				#define TOKEN_CHAR_EXPR(token,op,func)					extern I16 func (I16 a, I16 b);
				#define TOKEN_CHAR_REL(token,op,func)  					extern I16 func (I16 a, I16 b);
					#include BASIC_CONFIG_FILE
				#undef TOKEN_CHAR_TERM
				#undef TOKEN_CHAR_EXPR
				#undef TOKEN_CHAR_REL

			#undef BASIC_TOKEN_OPS
#endif

#ifdef BASIC_USES_EXTENSIONS
			#define BASIC_USER_EXTENSIONS
				enum {
					#define USER_FUNCTION(name,text,func,param)		name, 
					#define USER_VARIABLE(name,text,var)					name,
					#define USER_ARRAY(name,text,var,size)				name, 
						#include BASIC_CONFIG_FILE
					#undef USER_FUNCTION
					#undef USER_VARIABLE
					#undef USER_ARRAY
					BASIC_NUM_USER_EXTENSIONS
				};

				#define USER_FUNCTION(name,text,func,param)			extern I16 func (T_USER_FUNC_DATAS stDatas);
				#define USER_VARIABLE(name,text,var)						extern I16 var;
				#define USER_ARRAY(name,text,var,size)					extern I16 var [size];
					#include BASIC_CONFIG_FILE
				#undef USER_FUNCTION
				#undef USER_VARIABLE
				#undef USER_ARRAY

			#undef BASIC_USER_EXTENSIONS
#endif

			#define RM_TEXT				0
			#define RM_COMPILED		1

#ifdef BASIC_USES_CHARDEVICE
			void  tokenizer_chardevice    			(U8 device);
#endif


			void 	tokenizer_set_text 						(void *program, CPOS length);
			void	tokenizer_init								(char rmode);
			void  tokenizer_start_pos     			(void);
			void	tokenizer_next								(void);
			TOKEN	tokenizer_token								(void);
			void  tokenizer_set_rel_pos					(CPOS stPos);
			CPOS  tokenizer_get_rel_pos   			(void);
			
			int		tokenizer_num									(void);
			char	tokenizer_variable_num				(void);
			void	tokenizer_string							(char *dest, char len);

#ifdef BASIC_USES_EXTENSIONS
			TOKEN	tokenizer_userdata						(U8 *type, U16 *data, void **ptr);
#endif

			char	tokenizer_finished						(void);

			void  tokenizer_exec_token_func			(void);
			char *tokenizer_geterror_text 			(U8 error);


#ifdef BASIC_USES_COMPILER
			typedef void (* T_SAVE_BYTE)			(unsigned char x);

			char 	tokenizer_translate					(T_SAVE_BYTE drop);
			void  tokenizer_jump_to_pos 			(CPOS jumptodest);

#endif

		#endif /* __TOKENIZER_H__ */
