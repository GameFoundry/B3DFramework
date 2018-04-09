/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton interface for Bison GLR parsers in C
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_BSPARSERFX_H_INCLUDED
# define YY_YY_BSPARSERFX_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */


#include "BsMMAlloc.h"
#include "BsASTFX.h"
#include "BsIncludeHandler.h"

#define YY_NO_UNISTD_H 1
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
	typedef void* yyscan_t;
#endif

typedef struct YYLTYPE {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	char *filename;
} YYLTYPE;
#define YYLTYPE_IS_DECLARED 1

#define YYLLOC_DEFAULT(Current, Rhs, N)																\
	do																								\
		if (N)																						\
		{																							\
			(Current).first_line = YYRHSLOC (Rhs, 1).first_line;									\
			(Current).first_column = YYRHSLOC (Rhs, 1).first_column;								\
			(Current).last_line = YYRHSLOC (Rhs, N).last_line;										\
			(Current).last_column = YYRHSLOC (Rhs, N).last_column;									\
			(Current).filename = YYRHSLOC (Rhs, 1).filename;										\
		}																							\
		else																						\
		{																							\
			(Current).first_line = (Current).last_line = YYRHSLOC (Rhs, 0).last_line;				\
			(Current).first_column = (Current).last_column = YYRHSLOC (Rhs, 0).last_column;			\
			(Current).filename = NULL;																\
		}																							\
	while (0)
	




/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_INTEGER = 258,
     TOKEN_FLOAT = 259,
     TOKEN_BOOLEAN = 260,
     TOKEN_STRING = 261,
     TOKEN_IDENTIFIER = 262,
     TOKEN_FILLMODEVALUE = 263,
     TOKEN_CULLANDQUEUEVALUE = 264,
     TOKEN_COMPFUNCVALUE = 265,
     TOKEN_OPVALUE = 266,
     TOKEN_BLENDOPVALUE = 267,
     TOKEN_COLORMASK = 268,
     TOKEN_OPTIONS = 269,
     TOKEN_SHADER = 270,
     TOKEN_SUBSHADER = 271,
     TOKEN_MIXIN = 272,
     TOKEN_SEPARABLE = 273,
     TOKEN_SORT = 274,
     TOKEN_PRIORITY = 275,
     TOKEN_TRANSPARENT = 276,
     TOKEN_FEATURESET = 277,
     TOKEN_PASS = 278,
     TOKEN_TAGS = 279,
     TOKEN_VARIATIONS = 280,
     TOKEN_CODE = 281,
     TOKEN_BLEND = 282,
     TOKEN_RASTER = 283,
     TOKEN_DEPTH = 284,
     TOKEN_STENCIL = 285,
     TOKEN_VARIATION = 286,
     TOKEN_FILLMODE = 287,
     TOKEN_CULLMODE = 288,
     TOKEN_DEPTHBIAS = 289,
     TOKEN_SDEPTHBIAS = 290,
     TOKEN_DEPTHCLIP = 291,
     TOKEN_SCISSOR = 292,
     TOKEN_MULTISAMPLE = 293,
     TOKEN_AALINE = 294,
     TOKEN_DEPTHREAD = 295,
     TOKEN_DEPTHWRITE = 296,
     TOKEN_COMPAREFUNC = 297,
     TOKEN_STENCILREF = 298,
     TOKEN_ENABLED = 299,
     TOKEN_READMASK = 300,
     TOKEN_WRITEMASK = 301,
     TOKEN_STENCILOPFRONT = 302,
     TOKEN_STENCILOPBACK = 303,
     TOKEN_FAIL = 304,
     TOKEN_ZFAIL = 305,
     TOKEN_ALPHATOCOVERAGE = 306,
     TOKEN_INDEPENDANTBLEND = 307,
     TOKEN_TARGET = 308,
     TOKEN_INDEX = 309,
     TOKEN_COLOR = 310,
     TOKEN_ALPHA = 311,
     TOKEN_SOURCE = 312,
     TOKEN_DEST = 313,
     TOKEN_OP = 314
   };
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


	int intValue;
	float floatValue;
	float matrixValue[16];
	int intVectorValue[4];
	const char* strValue;
	ASTFXNode* nodePtr;
	NodeOption nodeOption;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


int yyparse (ParseState* parse_state, yyscan_t scanner);

#endif /* !YY_YY_BSPARSERFX_H_INCLUDED  */
