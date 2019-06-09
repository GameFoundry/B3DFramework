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
     TOKEN_FORWARD = 277,
     TOKEN_FEATURESET = 278,
     TOKEN_PASS = 279,
     TOKEN_TAGS = 280,
     TOKEN_VARIATIONS = 281,
     TOKEN_CODE = 282,
     TOKEN_BLEND = 283,
     TOKEN_RASTER = 284,
     TOKEN_DEPTH = 285,
     TOKEN_STENCIL = 286,
     TOKEN_VARIATION = 287,
     TOKEN_FILLMODE = 288,
     TOKEN_CULLMODE = 289,
     TOKEN_DEPTHBIAS = 290,
     TOKEN_SDEPTHBIAS = 291,
     TOKEN_DEPTHCLIP = 292,
     TOKEN_SCISSOR = 293,
     TOKEN_MULTISAMPLE = 294,
     TOKEN_AALINE = 295,
     TOKEN_DEPTHREAD = 296,
     TOKEN_DEPTHWRITE = 297,
     TOKEN_COMPAREFUNC = 298,
     TOKEN_STENCILREF = 299,
     TOKEN_ENABLED = 300,
     TOKEN_READMASK = 301,
     TOKEN_WRITEMASK = 302,
     TOKEN_STENCILOPFRONT = 303,
     TOKEN_STENCILOPBACK = 304,
     TOKEN_FAIL = 305,
     TOKEN_ZFAIL = 306,
     TOKEN_ALPHATOCOVERAGE = 307,
     TOKEN_INDEPENDANTBLEND = 308,
     TOKEN_TARGET = 309,
     TOKEN_INDEX = 310,
     TOKEN_COLOR = 311,
     TOKEN_ALPHA = 312,
     TOKEN_SOURCE = 313,
     TOKEN_DEST = 314,
     TOKEN_OP = 315,
     TOKEN_NAME = 316
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
