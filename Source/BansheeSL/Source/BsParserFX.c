/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton implementation for Bison GLR parsers in C
   
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

/* C GLR parser skeleton written by Paul Hilfinger.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "glr.c"

/* Pure parsers.  */
#define YYPURE 1






/* Copy the first part of user declarations.  */
/* Line 207 of glr.c  */
#line 1 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"

#include "BsParserFX.h"
#include "BsLexerFX.h"
#define inline

void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg);

/* Line 207 of glr.c  */
#line 64 "BsParserFX.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#include "BsParserFX.h"

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Default (constant) value used for initialization for null
   right-hand sides.  Unlike the standard yacc.c template, here we set
   the default value of $$ to a zeroed-out value.  Since the default
   value is undefined, this behavior is technically correct.  */
static YYSTYPE yyval_default;
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;

/* Copy the second part of user declarations.  */

/* Line 230 of glr.c  */
#line 98 "BsParserFX.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#ifndef YYFREE
# define YYFREE free
#endif
#ifndef YYMALLOC
# define YYMALLOC malloc
#endif
#ifndef YYREALLOC
# define YYREALLOC realloc
#endif

#define YYSIZEMAX ((size_t) -1)

#ifdef __cplusplus
   typedef bool yybool;
#else
   typedef unsigned char yybool;
#endif
#define yytrue 1
#define yyfalse 0

#ifndef YYSETJMP
# include <setjmp.h>
# define YYJMP_BUF jmp_buf
# define YYSETJMP(Env) setjmp (Env)
/* Pacify clang.  */
# define YYLONGJMP(Env, Val) (longjmp (Env, Val), YYASSERT (0))
#endif

/*-----------------.
| GCC extensions.  |
`-----------------*/

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

#ifndef YYASSERT
# define YYASSERT(Condition) ((void) ((Condition) || (abort (), 0)))
#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   686

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  130
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  100
/* YYNRULES -- Number of rules.  */
#define YYNRULES  232
/* YYNRULES -- Number of states.  */
#define YYNSTATES  722
/* YYMAXRHS -- Maximum number of symbols on right-hand side of rule.  */
#define YYMAXRHS 33
/* YYMAXLEFT -- Maximum number of symbols to the left of a handle
   accessed by $0, $-1, etc., in any rule.  */
#define YYMAXLEFT 0

/* YYTRANSLATE(X) -- Bison symbol number corresponding to X.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   376

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     128,   129,     2,     2,   126,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   127,   123,
       2,   122,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   124,     2,   125,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    20,
      25,    30,    35,    41,    44,    45,    48,    50,    52,    54,
      56,    61,    66,    68,    74,    77,    78,    80,    84,    90,
      93,    94,    97,    99,   101,   106,   111,   116,   121,   126,
     131,   136,   141,   146,   151,   156,   161,   166,   171,   176,
     182,   188,   194,   200,   205,   210,   212,   217,   226,   229,
     232,   235,   238,   241,   244,   247,   250,   253,   261,   262,
     265,   270,   275,   280,   285,   291,   294,   295,   298,   300,
     305,   310,   316,   322,   328,   334,   339,   342,   345,   346,
     349,   355,   360,   365,   370,   371,   374,   376,   381,   386,
     391,   396,   401,   406,   411,   416,   421,   427,   433,   436,
     442,   443,   446,   451,   456,   461,   467,   475,   485,   491,
     499,   509,   523,   541,   561,   587,   621,   627,   630,   631,
     634,   639,   644,   649,   654,   659,   664,   669,   674,   679,
     684,   689,   694,   699,   704,   709,   714,   719,   724,   729,
     733,   737,   740,   743,   746,   749,   752,   755,   758,   761,
     764,   767,   770,   773,   776,   779,   782,   785,   788,   791,
     794,   797,   800,   803,   806,   809,   812,   815,   818,   821,
     824,   827,   830,   833,   836,   839,   842,   845,   848,   851,
     854,   857,   858,   861,   862,   865,   866,   869,   870,   873,
     874,   877,   878,   881,   882,   885,   886,   889,   890,   893,
     894,   897,   898,   901,   902,   905,   906,   909,   910,   915,
     916,   919,   925,   928,   929,   932,   936,   939,   940,   943,
     949,   955,   961
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const short int yyrhs[] =
{
     131,     0,    -1,    -1,   132,   131,    -1,   133,    -1,   134,
      -1,   182,    -1,   223,    -1,    62,   122,     5,   123,    -1,
      63,   122,    17,   123,    -1,    64,   122,     3,   123,    -1,
      65,   122,     5,   123,    -1,   135,   124,   136,   125,   123,
      -1,    68,   122,    -1,    -1,   137,   136,    -1,   138,    -1,
     142,    -1,   146,    -1,   147,    -1,    69,   122,     6,   123,
      -1,    70,   122,     6,   123,    -1,   139,    -1,   140,   124,
     141,   125,   123,    -1,    72,   122,    -1,    -1,     6,    -1,
       6,   126,   141,    -1,   143,   124,   144,   125,   123,    -1,
      71,   122,    -1,    -1,   145,   144,    -1,   146,    -1,   147,
      -1,   102,   122,     3,   123,    -1,    81,   122,     8,   123,
      -1,    82,   122,     9,   123,    -1,    83,   122,     4,   123,
      -1,    84,   122,     4,   123,    -1,    85,   122,     5,   123,
      -1,    86,   122,     5,   123,    -1,    87,   122,     5,   123,
      -1,    88,   122,     5,   123,    -1,    89,   122,     5,   123,
      -1,    90,   122,     5,   123,    -1,    91,   122,    10,   123,
      -1,    92,   122,     5,   123,    -1,    93,   122,     3,   123,
      -1,    94,   122,     3,   123,    -1,   149,   124,   152,   125,
     123,    -1,   150,   124,   152,   125,   123,    -1,   149,   124,
     151,   125,   123,    -1,   150,   124,   151,   125,   123,    -1,
      99,   122,     5,   123,    -1,   100,   122,     5,   123,    -1,
     154,    -1,    80,   122,     3,   123,    -1,   148,   124,   102,
     122,     3,   123,   125,   123,    -1,    73,   122,    -1,    74,
     122,    -1,    75,   122,    -1,    76,   122,    -1,    77,   122,
      -1,    78,   122,    -1,    79,   122,    -1,    95,   122,    -1,
      96,   122,    -1,    11,   126,    11,   126,    11,   126,    10,
      -1,    -1,   153,   152,    -1,    97,   122,    11,   123,    -1,
      98,   122,    11,   123,    -1,    71,   122,    11,   123,    -1,
      91,   122,    10,   123,    -1,   155,   124,   156,   125,   123,
      -1,   101,   122,    -1,    -1,   157,   156,    -1,   158,    -1,
     102,   122,     3,   123,    -1,   103,   122,     5,   123,    -1,
     159,   124,   161,   125,   123,    -1,   160,   124,   161,   125,
     123,    -1,   159,   124,   162,   125,   123,    -1,   160,   124,
     162,   125,   123,    -1,   106,   122,    12,   123,    -1,   104,
     122,    -1,   105,   122,    -1,    -1,   163,   161,    -1,    11,
     126,    11,   126,    15,    -1,   107,   122,    11,   123,    -1,
     108,   122,    11,   123,    -1,   109,   122,    15,   123,    -1,
      -1,   165,   164,    -1,   166,    -1,   111,   122,    14,   123,
      -1,   112,   122,    14,   123,    -1,   113,   122,    14,   123,
      -1,   114,   122,     3,   123,    -1,   115,   122,     4,   123,
      -1,   116,   122,     4,   123,    -1,   117,   122,     4,   123,
      -1,   118,   122,   173,   123,    -1,    91,   122,    10,   123,
      -1,   167,   124,   169,   125,   123,    -1,   167,   124,   168,
     125,   123,    -1,   110,   122,    -1,    13,   126,    13,   126,
      13,    -1,    -1,   170,   169,    -1,   119,   122,    13,   123,
      -1,   120,   122,    13,   123,    -1,   121,   122,    13,   123,
      -1,   124,     4,   126,     4,   125,    -1,   124,     4,   126,
       4,   126,     4,   125,    -1,   124,     4,   126,     4,   126,
       4,   126,     4,   125,    -1,   124,     3,   126,     3,   125,
      -1,   124,     3,   126,     3,   126,     3,   125,    -1,   124,
       3,   126,     3,   126,     3,   126,     3,   125,    -1,   124,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   126,
       4,   125,    -1,   124,     4,   126,     4,   126,     4,   126,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   125,
      -1,   124,     4,   126,     4,   126,     4,   126,     4,   126,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   125,
      -1,   124,     4,   126,     4,   126,     4,   126,     4,   126,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   126,
       4,   126,     4,   126,     4,   125,    -1,   124,     4,   126,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   126,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   126,
       4,   126,     4,   126,     4,   126,     4,   126,     4,   125,
      -1,   183,   124,   184,   125,   123,    -1,    66,   122,    -1,
      -1,   185,   184,    -1,   186,   228,   208,   123,    -1,   187,
     228,   209,   123,    -1,   188,   228,   210,   123,    -1,   189,
     228,   211,   123,    -1,   190,   228,   212,   123,    -1,   191,
     228,   213,   123,    -1,   192,   228,   214,   123,    -1,   193,
     228,   215,   123,    -1,   194,   228,   211,   123,    -1,   195,
     228,   211,   123,    -1,   196,   228,   216,   123,    -1,   197,
     228,   217,   123,    -1,   198,   228,   216,   123,    -1,   199,
     228,   218,   123,    -1,   200,   228,   219,   123,    -1,   201,
     228,   217,   123,    -1,   202,   228,   219,   123,    -1,   203,
     228,   220,   123,    -1,   205,   228,   222,   123,    -1,   206,
     228,   123,    -1,   207,   221,   123,    -1,    18,     7,    -1,
      19,     7,    -1,    20,     7,    -1,    21,     7,    -1,    22,
       7,    -1,    23,     7,    -1,    24,     7,    -1,    25,     7,
      -1,    26,     7,    -1,    27,     7,    -1,    28,     7,    -1,
      29,     7,    -1,    30,     7,    -1,    31,     7,    -1,    32,
       7,    -1,    33,     7,    -1,    34,     7,    -1,    35,     7,
      -1,    36,     7,    -1,    37,     7,    -1,    38,     7,    -1,
      39,     7,    -1,    40,     7,    -1,    41,     7,    -1,    42,
       7,    -1,    43,     7,    -1,    44,     7,    -1,    45,     7,
      -1,    46,     7,    -1,    47,     7,    -1,    48,     7,    -1,
      49,     7,    -1,    50,     7,    -1,    51,     7,    -1,    52,
       7,    -1,    53,     7,    -1,    54,     7,    -1,    55,     7,
      -1,    56,     7,    -1,   204,   228,    -1,    -1,   122,     4,
      -1,    -1,   122,   171,    -1,    -1,   122,   172,    -1,    -1,
     122,   173,    -1,    -1,   122,     3,    -1,    -1,   122,   174,
      -1,    -1,   122,   175,    -1,    -1,   122,   176,    -1,    -1,
     122,   177,    -1,    -1,   122,   178,    -1,    -1,   122,   179,
      -1,    -1,   122,   180,    -1,    -1,   122,   181,    -1,    -1,
     122,   124,   164,   125,    -1,    -1,   122,     6,    -1,   224,
     124,   225,   125,   123,    -1,    67,   122,    -1,    -1,   226,
     225,    -1,   227,   228,   123,    -1,    57,     7,    -1,    -1,
     229,   228,    -1,   127,    59,   128,     6,   129,    -1,   127,
      58,   128,     6,   129,    -1,   127,    60,   128,     5,   129,
      -1,   127,    61,   128,    16,   129,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   285,   285,   286,   290,   291,   292,   293,   297,   298,
     299,   300,   306,   310,   317,   319,   323,   324,   325,   326,
     330,   331,   332,   336,   340,   347,   349,   354,   364,   368,
     375,   377,   381,   382,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   408,   414,   428,   433,
     438,   443,   448,   453,   458,   468,   476,   484,   498,   500,
     504,   505,   506,   507,   512,   516,   523,   525,   529,   533,
     534,   535,   536,   537,   538,   539,   544,   552,   559,   561,
     565,   578,   579,   580,   585,   587,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,   605,   606,   610,   618,
     630,   632,   636,   637,   638,   643,   647,   651,   655,   659,
     663,   667,   676,   687,   698,   709,   723,   727,   734,   736,
     740,   741,   742,   743,   744,   745,   746,   747,   748,   749,
     750,   751,   752,   753,   754,   755,   756,   757,   758,   759,
     760,   776,   780,   784,   788,   792,   796,   800,   804,   808,
     812,   816,   820,   824,   828,   832,   836,   840,   844,   848,
     849,   850,   851,   852,   856,   857,   858,   859,   860,   861,
     862,   863,   864,   868,   869,   870,   871,   872,   873,   874,
     878,   886,   887,   891,   892,   896,   897,   901,   902,   906,
     907,   911,   912,   916,   917,   921,   922,   926,   927,   931,
     932,   936,   937,   941,   942,   946,   947,   951,   952,   956,
     957,   962,   966,   973,   975,   979,   983,   997,   999,  1003,
    1004,  1005,  1006
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOKEN_INTEGER", "TOKEN_FLOAT",
  "TOKEN_BOOLEAN", "TOKEN_STRING", "TOKEN_IDENTIFIER",
  "TOKEN_FILLMODEVALUE", "TOKEN_CULLMODEVALUE", "TOKEN_COMPFUNCVALUE",
  "TOKEN_OPVALUE", "TOKEN_COLORMASK", "TOKEN_ADDRMODEVALUE",
  "TOKEN_FILTERVALUE", "TOKEN_BLENDOPVALUE", "TOKEN_BUFFERUSAGE",
  "TOKEN_QUEUETYPE", "TOKEN_FLOATTYPE", "TOKEN_FLOAT2TYPE",
  "TOKEN_FLOAT3TYPE", "TOKEN_FLOAT4TYPE", "TOKEN_INTTYPE",
  "TOKEN_INT2TYPE", "TOKEN_INT3TYPE", "TOKEN_INT4TYPE", "TOKEN_COLORTYPE",
  "TOKEN_MAT2x2TYPE", "TOKEN_MAT2x3TYPE", "TOKEN_MAT2x4TYPE",
  "TOKEN_MAT3x2TYPE", "TOKEN_MAT3x3TYPE", "TOKEN_MAT3x4TYPE",
  "TOKEN_MAT4x2TYPE", "TOKEN_MAT4x3TYPE", "TOKEN_MAT4x4TYPE",
  "TOKEN_SAMPLER1D", "TOKEN_SAMPLER2D", "TOKEN_SAMPLER3D",
  "TOKEN_SAMPLERCUBE", "TOKEN_SAMPLER2DMS", "TOKEN_TEXTURE1D",
  "TOKEN_TEXTURE2D", "TOKEN_TEXTURE3D", "TOKEN_TEXTURECUBE",
  "TOKEN_TEXTURE2DMS", "TOKEN_RWTEXTURE1D", "TOKEN_RWTEXTURE2D",
  "TOKEN_RWTEXTURE3D", "TOKEN_RWTEXTURE2DMS", "TOKEN_BYTEBUFFER",
  "TOKEN_STRUCTBUFFER", "TOKEN_RWTYPEDBUFFER", "TOKEN_RWBYTEBUFFER",
  "TOKEN_RWSTRUCTBUFFER", "TOKEN_RWAPPENDBUFFER", "TOKEN_RWCONSUMEBUFFER",
  "TOKEN_PARAMSBLOCK", "TOKEN_AUTO", "TOKEN_ALIAS", "TOKEN_SHARED",
  "TOKEN_USAGE", "TOKEN_SEPARABLE", "TOKEN_SORT", "TOKEN_PRIORITY",
  "TOKEN_TRANSPARENT", "TOKEN_PARAMETERS", "TOKEN_BLOCKS",
  "TOKEN_TECHNIQUE", "TOKEN_RENDERER", "TOKEN_LANGUAGE", "TOKEN_PASS",
  "TOKEN_TAGS", "TOKEN_VERTEX", "TOKEN_FRAGMENT", "TOKEN_GEOMETRY",
  "TOKEN_HULL", "TOKEN_DOMAIN", "TOKEN_COMPUTE", "TOKEN_COMMON",
  "TOKEN_STENCILREF", "TOKEN_FILLMODE", "TOKEN_CULLMODE",
  "TOKEN_DEPTHBIAS", "TOKEN_SDEPTHBIAS", "TOKEN_DEPTHCLIP",
  "TOKEN_SCISSOR", "TOKEN_MULTISAMPLE", "TOKEN_AALINE", "TOKEN_DEPTHREAD",
  "TOKEN_DEPTHWRITE", "TOKEN_COMPAREFUNC", "TOKEN_STENCIL",
  "TOKEN_STENCILREADMASK", "TOKEN_STENCILWRITEMASK",
  "TOKEN_STENCILOPFRONT", "TOKEN_STENCILOPBACK", "TOKEN_FAIL",
  "TOKEN_ZFAIL", "TOKEN_ALPHATOCOVERAGE", "TOKEN_INDEPENDANTBLEND",
  "TOKEN_TARGET", "TOKEN_INDEX", "TOKEN_BLEND", "TOKEN_COLOR",
  "TOKEN_ALPHA", "TOKEN_WRITEMASK", "TOKEN_SOURCE", "TOKEN_DEST",
  "TOKEN_OP", "TOKEN_ADDRMODE", "TOKEN_MINFILTER", "TOKEN_MAGFILTER",
  "TOKEN_MIPFILTER", "TOKEN_MAXANISO", "TOKEN_MIPBIAS", "TOKEN_MIPMIN",
  "TOKEN_MIPMAX", "TOKEN_BORDERCOLOR", "TOKEN_U", "TOKEN_V", "TOKEN_W",
  "'='", "';'", "'{'", "'}'", "','", "':'", "'('", "')'", "$accept",
  "shader", "shader_statement", "shader_option", "technique",
  "technique_header", "technique_body", "technique_statement",
  "technique_option", "tags", "tags_header", "tags_body", "pass",
  "pass_header", "pass_body", "pass_statement", "pass_option", "code",
  "code_header", "stencil_op_front_header", "stencil_op_back_header",
  "stencil_op_body_init", "stencil_op_body", "stencil_op_option", "target",
  "target_header", "target_body", "target_statement", "target_option",
  "blend_color_header", "blend_alpha_header", "blenddef_body",
  "blenddef_body_init", "blenddef_option", "sampler_state_body",
  "sampler_state_option", "addr_mode", "addr_mode_header",
  "addr_mode_body_init", "addr_mode_body", "addr_mode_option", "float2",
  "float3", "float4", "int2", "int3", "int4", "mat6", "mat8", "mat9",
  "mat12", "mat16", "parameters", "parameters_header", "parameters_body",
  "parameter", "param_header_float", "param_header_float2",
  "param_header_float3", "param_header_float4", "param_header_int",
  "param_header_int2", "param_header_int3", "param_header_int4",
  "param_header_color", "param_header_mat2x2", "param_header_mat2x3",
  "param_header_mat2x4", "param_header_mat3x2", "param_header_mat3x3",
  "param_header_mat3x4", "param_header_mat4x2", "param_header_mat4x3",
  "param_header_mat4x4", "param_header_sampler", "param_header_texture",
  "param_header_buffer", "param_header_qualified_sampler",
  "param_body_float", "param_body_float2", "param_body_float3",
  "param_body_float4", "param_body_int", "param_body_int2",
  "param_body_int3", "param_body_int4", "param_body_mat6",
  "param_body_mat8", "param_body_mat9", "param_body_mat12",
  "param_body_mat16", "param_body_sampler", "param_body_tex", "blocks",
  "blocks_header", "blocks_body", "block", "block_header",
  "qualifier_list", "qualifier", YY_NULL
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   130,   131,   131,   132,   132,   132,   132,   133,   133,
     133,   133,   134,   135,   136,   136,   137,   137,   137,   137,
     138,   138,   138,   139,   140,   141,   141,   141,   142,   143,
     144,   144,   145,   145,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   147,   148,   148,
     148,   148,   148,   148,   148,   149,   150,   151,   152,   152,
     153,   153,   153,   153,   154,   155,   156,   156,   157,   158,
     158,   158,   158,   158,   158,   158,   159,   160,   161,   161,
     162,   163,   163,   163,   164,   164,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   166,   166,   167,   168,
     169,   169,   170,   170,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   184,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     204,   204,   204,   204,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   206,   206,   206,   206,   206,   206,   206,
     207,   208,   208,   209,   209,   210,   210,   211,   211,   212,
     212,   213,   213,   214,   214,   215,   215,   216,   216,   217,
     217,   218,   218,   219,   219,   220,   220,   221,   221,   222,
     222,   223,   224,   225,   225,   226,   227,   228,   228,   229,
     229,   229,   229
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     4,     4,
       4,     4,     5,     2,     0,     2,     1,     1,     1,     1,
       4,     4,     1,     5,     2,     0,     1,     3,     5,     2,
       0,     2,     1,     1,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     5,
       5,     5,     5,     4,     4,     1,     4,     8,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     7,     0,     2,
       4,     4,     4,     4,     5,     2,     0,     2,     1,     4,
       4,     5,     5,     5,     5,     4,     2,     2,     0,     2,
       5,     4,     4,     4,     0,     2,     1,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     2,     5,
       0,     2,     4,     4,     4,     5,     7,     9,     5,     7,
       9,    13,    17,    19,    25,    33,     5,     2,     0,     2,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     3,
       3,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     0,     2,     0,     2,     0,     2,     0,     2,     0,
       2,     0,     2,     0,     2,     0,     2,     0,     2,     0,
       2,     0,     2,     0,     2,     0,     2,     0,     4,     0,
       2,     5,     2,     0,     2,     3,     2,     0,     2,     5,
       5,     5,     5
};

/* YYDPREC[RULE-NUM] -- Dynamic precedence of rule #RULE-NUM (0 if none).  */
static const unsigned char yydprec[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0
};

/* YYMERGER[RULE-NUM] -- Index of merging function for rule #RULE-NUM.  */
static const unsigned char yymerger[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0
};

/* YYDEFACT[S] -- default reduction number in state S.  Performed when
   YYTABLE doesn't specify something else to do.  Zero means the default
   is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     0,     0,     0,     0,     0,     0,     0,     2,
       4,     5,     0,     6,     0,     7,     0,     0,     0,     0,
       0,   127,   222,    13,     1,     3,    14,   128,   223,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    16,    22,     0,
      17,     0,    18,    19,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   128,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   227,   227,   227,
     227,   217,     0,     0,   223,   227,     8,     9,    10,    11,
       0,     0,    29,    24,    58,    59,    60,    61,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,    66,     0,     0,
      75,     0,     0,    15,    25,    30,     0,    68,    68,    76,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,     0,
     129,     0,   191,   227,   193,   195,   197,   199,   201,   203,
     205,   197,   197,   207,   209,   207,   211,   213,   209,   213,
     215,   190,   219,     0,     0,     0,   226,     0,   224,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    26,     0,     0,    30,    32,    33,     0,     0,     0,
       0,     0,     0,     0,     0,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    76,    78,     0,     0,   126,     0,
       0,     0,     0,     0,     0,   228,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   149,    94,   150,
     221,   225,    20,    21,    56,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    53,
      54,    34,    25,     0,     0,    31,     0,     0,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,    86,
      87,     0,     0,    77,    88,    88,     0,     0,     0,     0,
     192,   130,     0,   194,   131,     0,   196,   132,     0,   198,
     133,   200,   134,     0,   202,   135,     0,   204,   136,     0,
     206,   137,   138,   139,     0,   208,   140,     0,   210,   141,
     142,     0,   212,   143,     0,   214,   144,   145,   146,     0,
     216,   147,   220,   148,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    96,     0,    27,    23,
      28,     0,     0,     0,     0,     0,     0,    51,    49,    52,
      50,     0,     0,     0,    74,     0,     0,     0,     0,     0,
       0,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     108,     0,     0,     0,     0,     0,     0,     0,     0,   218,
      95,   110,     0,     0,    72,    73,    70,    71,    79,    80,
      85,     0,     0,     0,     0,     0,     0,    89,     0,     0,
     230,   229,   231,   232,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     110,     0,     0,     0,     0,     0,     0,    81,    83,    82,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,    97,    98,    99,   100,   101,   102,   103,
     104,     0,     0,     0,     0,     0,     0,   111,    57,     0,
       0,    91,    92,    93,   115,     0,     0,   118,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
     106,    67,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   112,   113,   114,   116,     0,   119,     0,
       0,     0,     0,     0,     0,   109,     0,     0,     0,     0,
       0,     0,     0,   117,   120,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   121,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   122,     0,     0,     0,     0,     0,     0,   123,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   124,     0,     0,     0,     0,     0,     0,     0,
       0,   125
};

/* YYPDEFGOTO[NTERM-NUM].  */
static const short int yydefgoto[] =
{
      -1,     8,     9,    10,    11,    12,    65,    66,    67,    68,
      69,   282,    70,    71,   283,   284,    72,    73,    74,    75,
      76,   293,   294,   295,    77,    78,   303,   304,   305,   306,
     307,   479,   480,   481,   454,   455,   456,   457,   558,   559,
     560,   403,   406,   409,   414,   417,   420,   425,   428,   432,
     435,   440,    13,    14,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   314,   317,
     319,   321,   323,   325,   327,   329,   333,   335,   338,   340,
     344,   255,   346,    15,    16,   143,   144,   145,   232,   233
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -371
static const short int yypact[] =
{
       9,   -96,   -44,   -24,   -18,   -15,   -13,   -12,   113,     9,
    -371,  -371,    -8,  -371,    -3,  -371,    34,   154,   171,   186,
     189,  -371,  -371,  -371,  -371,  -371,    61,    11,   138,    73,
      74,    75,    76,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    61,  -371,  -371,   114,
    -371,   115,  -371,  -371,   116,   117,   118,  -371,   119,   206,
     226,   227,   228,   230,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   153,    11,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   158,   274,   157,   138,   152,  -371,  -371,  -371,  -371,
     277,   278,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,   282,   279,   280,   284,   286,   281,   287,   288,   289,
     290,   291,   292,   293,   294,   296,  -371,  -371,   295,   298,
    -371,   301,   168,  -371,   299,    91,   199,    17,    17,   -22,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,   183,
    -371,    42,   185,   152,   187,   188,   190,   191,   192,   193,
     194,   190,   190,   195,   196,   195,   197,   198,   196,   198,
     200,  -371,   201,   202,   184,   203,  -371,   204,  -371,   205,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   229,
    -371,   231,   233,   234,    91,  -371,  -371,   232,   235,   271,
     272,   273,   275,   276,   283,     8,   285,   297,   302,   303,
     304,   305,   306,   307,   -22,  -371,   236,   309,  -371,   308,
     310,   311,   312,   317,   300,  -371,   313,   318,   319,   321,
     322,   324,   326,   325,   327,   329,   330,   332,   333,   335,
     336,   337,   338,   340,   341,   343,   344,   345,   347,   348,
     350,   351,   352,   353,   355,   349,   356,  -371,   -21,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,   299,   357,   358,  -371,   346,   339,   342,   314,
     354,   359,   360,   361,  -371,   362,   363,   364,   366,  -371,
    -371,   365,   367,  -371,    16,    16,   368,   369,   371,   370,
    -371,  -371,   374,  -371,  -371,   375,  -371,  -371,   376,  -371,
    -371,  -371,  -371,   378,  -371,  -371,   379,  -371,  -371,   380,
    -371,  -371,  -371,  -371,   381,  -371,  -371,   383,  -371,  -371,
    -371,   384,  -371,  -371,   385,  -371,  -371,  -371,  -371,   386,
    -371,  -371,  -371,  -371,   320,   323,   328,   331,   334,   372,
     373,   377,   382,   387,   388,   -21,  -371,   390,  -371,  -371,
    -371,   389,   391,   392,   393,   395,   396,  -371,  -371,  -371,
    -371,   397,   398,   399,  -371,   400,   401,   402,   403,   404,
     405,    10,   406,   407,   182,   408,   409,   410,   414,   415,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   394,
    -371,   435,   447,   450,   411,   412,   413,   425,   322,  -371,
    -371,     7,   426,   457,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,   460,   465,   471,   472,   429,   430,  -371,   431,   432,
    -371,  -371,  -371,  -371,   427,   484,   485,   488,   489,   490,
     492,   493,   494,   496,   497,   433,   434,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   448,   449,
     -34,   452,   451,   453,   455,   458,   459,  -371,  -371,  -371,
    -371,   461,   454,   462,   464,   466,   467,   468,   469,   470,
     473,   474,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,   495,   498,   514,   515,   475,   478,  -371,  -371,   500,
     487,  -371,  -371,  -371,  -371,   499,   501,  -371,   503,   504,
     529,   530,   531,   532,   554,   476,   480,   481,   482,  -371,
    -371,  -371,  -371,   483,   486,   491,   502,   505,   506,   507,
     508,   509,   557,  -371,  -371,  -371,  -371,   567,  -371,   569,
     572,   579,   580,   581,   583,  -371,   511,   512,   513,   516,
     517,   518,   519,  -371,  -371,   586,   587,   593,   602,   603,
     520,   521,   522,   523,   524,   605,   606,   607,   609,   610,
     526,   527,   528,   533,   534,  -371,   611,   613,   614,   615,
     535,   536,   537,   538,   616,   617,   618,   619,   540,   541,
     542,   543,  -371,   620,   621,   622,   545,   546,   547,  -371,
     623,   625,   548,   549,   626,   634,   550,   551,   636,   637,
     553,   555,  -371,   648,   556,   651,   558,   652,   559,   653,
     561,  -371
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -371,   649,  -371,  -371,  -371,  -371,   600,  -371,  -371,  -371,
    -371,   -16,  -371,  -371,    67,  -371,  -164,  -162,  -371,  -371,
    -371,   174,  -166,  -371,  -371,  -371,    59,  -371,  -371,  -371,
    -371,  -370,   -31,  -371,   -89,  -371,  -371,  -371,  -371,  -192,
    -371,  -371,  -371,  -139,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,   315,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,  -371,
    -371,  -173,  -371,  -371,  -371,  -371,   127,   125,  -371,   135,
    -371,  -371,  -371,  -371,  -371,   539,  -371,  -371,  -121,  -371
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short int yytable[] =
{
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     554,   285,   297,   286,   259,   482,    17,   475,   288,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   330,   331,
     444,     1,     2,     3,     4,     5,     6,     7,    18,   289,
     298,   299,   300,   301,   302,   555,   556,   557,   289,   445,
     446,   447,   448,   449,   450,   451,   452,   453,    19,   290,
     309,   310,   311,   312,    20,   291,   292,    21,   290,    22,
      23,   527,   315,    24,   291,   292,    26,   476,   477,   478,
     285,    27,   286,   476,   477,   478,   555,   556,   557,   384,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    28,    29,
      61,    62,    63,    64,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    30,    31,
      61,    62,    63,    64,    32,   142,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   190,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   191,   192,   193,   182,   194,   184,   185,
     186,   187,   188,   189,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   231,
     254,   256,   257,   260,   261,   262,   267,   263,   265,   264,
     266,   280,   268,   269,   270,   271,   272,   275,   274,   276,
     277,   287,   273,   278,   279,   281,   308,   313,   348,   316,
     318,   530,   320,   322,   324,   326,   328,   332,   334,   337,
     339,   400,   343,   345,   464,   347,   349,   350,   351,   411,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   461,
     462,   375,   371,   463,   376,   442,   458,   372,   373,   374,
     394,   377,   296,   393,   483,   465,   510,   471,   597,   553,
     466,   472,   336,   341,   484,   485,   486,   473,   488,   489,
     490,   491,   492,   493,   342,   494,   487,   495,   496,   497,
     498,     0,     0,   378,   379,   380,     0,   381,     0,     0,
       0,   382,     0,     0,   545,     0,     0,     0,   383,     0,
     385,     0,     0,     0,   549,     0,   550,   551,     0,     0,
       0,     0,   386,   401,   387,   388,   389,   390,   391,   552,
       0,   571,   392,   395,   230,     0,   396,   402,   397,   398,
     399,   404,   499,   405,   407,   500,   408,   410,   412,   546,
     501,   413,   415,   502,   416,   418,   503,   419,   421,   422,
     423,   547,   424,   426,   548,   427,   429,   430,   562,   431,
     433,   563,   434,   436,   437,   438,   564,   439,   441,   443,
     459,   460,   565,   467,   468,   469,   470,   566,   572,   573,
     474,   574,   575,   576,   504,   505,   577,   578,   579,   506,
     580,   581,   622,   623,   507,   624,   625,   626,   615,   508,
     621,   616,   512,   509,   511,   514,   515,   513,   516,   517,
     518,   519,   520,   522,   523,   524,   521,   617,   618,   525,
     526,   528,   529,   627,   628,   629,   630,   531,   532,   533,
     534,   535,   536,   537,   538,   539,   540,   541,   542,   543,
     544,   561,   567,   568,   569,   570,   582,   583,   631,   584,
     585,   586,   587,   588,   589,   590,   592,   593,   594,   591,
     645,   646,   647,   595,   596,   598,   648,   599,   601,   600,
     605,   602,   603,   649,   650,   651,   604,   652,   606,   607,
     660,   661,   608,   609,   610,   611,   612,   662,   619,   613,
     614,   620,   632,   633,   634,   635,   663,   664,   636,   670,
     671,   672,   637,   673,   674,   680,   638,   681,   682,   683,
     688,   689,   690,   691,   696,   697,   698,   702,   639,   703,
     706,   640,   641,   642,   643,   644,   653,   654,   707,   655,
     710,   711,   656,   657,   658,   659,   665,   666,   667,   668,
     669,   675,   714,   676,   677,   716,   718,   720,    25,   678,
     679,   684,   685,   686,   687,   692,   183,   693,   694,   695,
     699,     0,   700,   701,   704,   705,   708,   709,   712,     0,
       0,   713,   715,   258,   717,   719,   721
};

/* YYCONFLP[YYPACT[STATE-NUM]] -- Pointer into YYCONFL of start of
   list of conflicting reductions corresponding to action entry for
   state STATE-NUM in yytable.  0 means no conflicts.  The list in
   yyconfl is terminated by a rule number of 0.  */
static const unsigned char yyconflp[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0
};

/* YYCONFL[I] -- lists of conflicting rule numbers, each terminated by
   0, pointed into by YYCONFLP.  */
static const short int yyconfl[] =
{
       0
};

static const short int yycheck[] =
{
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
      13,   185,   188,   185,   145,   395,   122,    11,    11,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,   241,   242,
      91,    62,    63,    64,    65,    66,    67,    68,   122,    71,
     102,   103,   104,   105,   106,   119,   120,   121,    71,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   122,    91,
      58,    59,    60,    61,   122,    97,    98,   122,    91,   122,
     122,   481,   233,     0,    97,    98,   124,   107,   108,   109,
     284,   124,   284,   107,   108,   109,   119,   120,   121,   295,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,   124,     5,
      99,   100,   101,   102,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    17,     3,
      99,   100,   101,   102,     5,    57,   123,   123,   123,   123,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,     7,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,     7,     7,     7,   125,     7,   124,   124,
     124,   124,   124,   124,     7,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,     7,     7,     7,   125,   127,
     122,     7,   125,     6,     6,     3,     5,     8,     4,     9,
       4,   123,     5,     5,     5,     5,     5,     3,     5,     3,
       5,   102,    10,     5,     3,     6,   123,   122,   124,   122,
     122,   129,   122,   122,   122,   122,   122,   122,   122,   122,
     122,     4,   122,   122,    10,   123,   123,   123,   123,     3,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,     3,
      11,   284,   123,    11,   122,     6,   372,   126,   125,   125,
     124,   126,   188,   304,   395,    11,   455,     3,   560,   508,
      11,     5,   245,   248,     6,     6,     5,    12,     4,     4,
       4,     3,     3,     3,   249,     4,    16,     4,     4,     4,
       4,    -1,    -1,   122,   122,   122,    -1,   122,    -1,    -1,
      -1,   125,    -1,    -1,    10,    -1,    -1,    -1,   125,    -1,
     125,    -1,    -1,    -1,     3,    -1,     4,     4,    -1,    -1,
      -1,    -1,   125,   123,   122,   122,   122,   122,   122,     4,
      -1,     4,   125,   124,   119,    -1,   128,   124,   128,   128,
     128,   123,   122,   124,   123,   122,   124,   123,   123,    14,
     122,   124,   123,   122,   124,   123,   122,   124,   123,   123,
     123,    14,   124,   123,    14,   124,   123,   123,    11,   124,
     123,    11,   124,   123,   123,   123,    11,   124,   123,   123,
     123,   123,    11,   123,   123,   123,   123,    15,     4,     4,
     123,     3,     3,     3,   122,   122,     4,     4,     4,   122,
       4,     4,    15,     4,   122,     4,     3,     3,    13,   122,
      10,    13,   123,   125,   124,   123,   123,   126,   123,   123,
     123,   123,   123,   122,   122,   122,   126,    13,    13,   125,
     125,   125,   125,     4,     4,     4,     4,   129,   129,   129,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   125,   123,   123,   123,   123,   123,   123,     4,   123,
     123,   123,   123,   123,   123,   123,   122,   122,   122,   126,
      13,     4,     3,   125,   125,   123,     4,   126,   123,   126,
     126,   123,   123,     4,     4,     4,   125,     4,   126,   125,
       4,     4,   126,   126,   126,   126,   126,     4,   123,   126,
     126,   123,   126,   123,   123,   123,     4,     4,   125,     4,
       4,     4,   126,     4,     4,     4,   125,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,   126,     4,
       4,   126,   126,   126,   126,   126,   125,   125,     4,   126,
       4,     4,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   125,     4,   126,   126,     4,     4,     4,     9,   126,
     126,   126,   126,   126,   126,   125,    66,   126,   126,   126,
     125,    -1,   126,   126,   126,   126,   126,   126,   125,    -1,
      -1,   126,   126,   144,   126,   126,   125
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    62,    63,    64,    65,    66,    67,    68,   131,   132,
     133,   134,   135,   182,   183,   223,   224,   122,   122,   122,
     122,   122,   122,   122,     0,   131,   124,   124,   124,     5,
      17,     3,     5,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    99,   100,   101,   102,   136,   137,   138,   139,   140,
     142,   143,   146,   147,   148,   149,   150,   154,   155,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,    57,   225,   226,   227,   123,   123,   123,   123,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   125,   136,   124,   124,   124,   124,   124,   124,
       7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,     7,     7,     7,     7,   125,
     184,   127,   228,   229,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   122,   221,     7,   125,   225,   228,
       6,     6,     3,     8,     9,     4,     4,     5,     5,     5,
       5,     5,     5,    10,     5,     3,     3,     5,     5,     3,
     123,     6,   141,   144,   145,   146,   147,   102,    11,    71,
      91,    97,    98,   151,   152,   153,   151,   152,   102,   103,
     104,   105,   106,   156,   157,   158,   159,   160,   123,    58,
      59,    60,    61,   122,   208,   228,   122,   209,   122,   210,
     122,   211,   122,   212,   122,   213,   122,   214,   122,   215,
     211,   211,   122,   216,   122,   217,   216,   122,   218,   122,
     219,   217,   219,   122,   220,   122,   222,   123,   124,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   126,   125,   125,   144,   122,   126,   122,   122,
     122,   122,   125,   125,   152,   125,   125,   122,   122,   122,
     122,   122,   125,   156,   124,   124,   128,   128,   128,   128,
       4,   123,   124,   171,   123,   124,   172,   123,   124,   173,
     123,     3,   123,   124,   174,   123,   124,   175,   123,   124,
     176,   123,   123,   123,   124,   177,   123,   124,   178,   123,
     123,   124,   179,   123,   124,   180,   123,   123,   123,   124,
     181,   123,     6,   123,    91,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   164,   165,   166,   167,   141,   123,
     123,     3,    11,    11,    10,    11,    11,   123,   123,   123,
     123,     3,     5,    12,   123,    11,   107,   108,   109,   161,
     162,   163,   161,   162,     6,     6,     5,    16,     4,     4,
       4,     3,     3,     3,     4,     4,     4,     4,     4,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   125,
     164,   124,   123,   126,   123,   123,   123,   123,   123,   123,
     123,   126,   122,   122,   122,   125,   125,   161,   125,   125,
     129,   129,   129,   129,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,    10,    14,    14,    14,     3,
       4,     4,     4,   173,    13,   119,   120,   121,   168,   169,
     170,   125,    11,    11,    11,    11,    15,   123,   123,   123,
     123,     4,     4,     4,     3,     3,     3,     4,     4,     4,
       4,     4,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   126,   122,   122,   122,   125,   125,   169,   123,   126,
     126,   123,   123,   123,   125,   126,   126,   125,   126,   126,
     126,   126,   126,   126,   126,    13,    13,    13,    13,   123,
     123,    10,    15,     4,     4,     3,     3,     4,     4,     4,
       4,     4,   126,   123,   123,   123,   125,   126,   125,   126,
     126,   126,   126,   126,   126,    13,     4,     3,     4,     4,
       4,     4,     4,   125,   125,   126,   126,   126,   126,   126,
       4,     4,     4,     4,     4,   126,   126,   126,   126,   126,
       4,     4,     4,     4,     4,   125,   126,   126,   126,   126,
       4,     4,     4,     4,   126,   126,   126,   126,     4,     4,
       4,     4,   125,   126,   126,   126,     4,     4,     4,   125,
     126,   126,     4,     4,   126,   126,     4,     4,   126,   126,
       4,     4,   125,   126,     4,   126,     4,   126,     4,   126,
       4,   125
};

/* Error token number */
#define YYTERROR 1


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (YYID (N))                                                     \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (YYID (0))
#endif

# define YYRHSLOC(Rhs, K) ((Rhs)[K].yystate.yyloc)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

__attribute__((__unused__))
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
#else
static unsigned
yy_location_print_ (yyo, yylocp)
    FILE *yyo;
    YYLTYPE const * const yylocp;
#endif
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += fprintf (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += fprintf (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += fprintf (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += fprintf (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += fprintf (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#define YYLEX yylex (&yylval, &yylloc, scanner)


#undef yynerrs
#define yynerrs (yystackp->yyerrcnt)
#undef yychar
#define yychar (yystackp->yyrawchar)
#undef yylval
#define yylval (yystackp->yyval)
#undef yylloc
#define yylloc (yystackp->yyloc)


static const int YYEOF = 0;
static const int YYEMPTY = -2;

typedef enum { yyok, yyaccept, yyabort, yyerr } YYRESULTTAG;

#define YYCHK(YYE)                                                           \
   do { YYRESULTTAG yyflag = YYE; if (yyflag != yyok) return yyflag; }       \
   while (YYID (0))

#if YYDEBUG

# ifndef YYFPRINTF
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseState* parse_state, yyscan_t scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (parse_state);
  YYUSE (scanner);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseState* parse_state, yyscan_t scanner)
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, parse_state, scanner);
  YYFPRINTF (yyoutput, ")");
}

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)          \
do {                                                            \
  if (yydebug)                                                  \
    {                                                           \
      YYFPRINTF (stderr, "%s ", Title);                         \
      yy_symbol_print (stderr, Type, Value, Location, parse_state, scanner);        \
      YYFPRINTF (stderr, "\n");                                 \
    }                                                           \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;

#else /* !YYDEBUG */

# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)

#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYMAXDEPTH * sizeof (GLRStackItem)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

/* Minimum number of free items on the stack allowed after an
   allocation.  This is to allow allocation and initialization
   to be completed by functions that call yyexpandGLRStack before the
   stack is expanded, thus insuring that all necessary pointers get
   properly redirected to new data.  */
#define YYHEADROOM 2

#ifndef YYSTACKEXPANDABLE
# if (! defined __cplusplus \
      || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
          && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL))
#  define YYSTACKEXPANDABLE 1
# else
#  define YYSTACKEXPANDABLE 0
# endif
#endif

#if YYSTACKEXPANDABLE
# define YY_RESERVE_GLRSTACK(Yystack)                   \
  do {                                                  \
    if (Yystack->yyspaceLeft < YYHEADROOM)              \
      yyexpandGLRStack (Yystack);                       \
  } while (YYID (0))
#else
# define YY_RESERVE_GLRSTACK(Yystack)                   \
  do {                                                  \
    if (Yystack->yyspaceLeft < YYHEADROOM)              \
      yyMemoryExhausted (Yystack);                      \
  } while (YYID (0))
#endif


#if YYERROR_VERBOSE

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static size_t
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return strlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* !YYERROR_VERBOSE */

/** State numbers, as in LALR(1) machine */
typedef int yyStateNum;

/** Rule numbers, as in LALR(1) machine */
typedef int yyRuleNum;

/** Grammar symbol */
typedef short int yySymbol;

/** Item references, as in LALR(1) machine */
typedef short int yyItemNum;

typedef struct yyGLRState yyGLRState;
typedef struct yyGLRStateSet yyGLRStateSet;
typedef struct yySemanticOption yySemanticOption;
typedef union yyGLRStackItem yyGLRStackItem;
typedef struct yyGLRStack yyGLRStack;

struct yyGLRState {
  /** Type tag: always true.  */
  yybool yyisState;
  /** Type tag for yysemantics.  If true, yysval applies, otherwise
   *  yyfirstVal applies.  */
  yybool yyresolved;
  /** Number of corresponding LALR(1) machine state.  */
  yyStateNum yylrState;
  /** Preceding state in this stack */
  yyGLRState* yypred;
  /** Source position of the first token produced by my symbol */
  size_t yyposn;
  union {
    /** First in a chain of alternative reductions producing the
     *  non-terminal corresponding to this state, threaded through
     *  yynext.  */
    yySemanticOption* yyfirstVal;
    /** Semantic value for this state.  */
    YYSTYPE yysval;
  } yysemantics;
  /** Source location for this state.  */
  YYLTYPE yyloc;
};

struct yyGLRStateSet {
  yyGLRState** yystates;
  /** During nondeterministic operation, yylookaheadNeeds tracks which
   *  stacks have actually needed the current lookahead.  During deterministic
   *  operation, yylookaheadNeeds[0] is not maintained since it would merely
   *  duplicate yychar != YYEMPTY.  */
  yybool* yylookaheadNeeds;
  size_t yysize, yycapacity;
};

struct yySemanticOption {
  /** Type tag: always false.  */
  yybool yyisState;
  /** Rule number for this reduction */
  yyRuleNum yyrule;
  /** The last RHS state in the list of states to be reduced.  */
  yyGLRState* yystate;
  /** The lookahead for this reduction.  */
  int yyrawchar;
  YYSTYPE yyval;
  YYLTYPE yyloc;
  /** Next sibling in chain of options.  To facilitate merging,
   *  options are chained in decreasing order by address.  */
  yySemanticOption* yynext;
};

/** Type of the items in the GLR stack.  The yyisState field
 *  indicates which item of the union is valid.  */
union yyGLRStackItem {
  yyGLRState yystate;
  yySemanticOption yyoption;
};

struct yyGLRStack {
  int yyerrState;
  /* To compute the location of the error token.  */
  yyGLRStackItem yyerror_range[3];

  int yyerrcnt;
  int yyrawchar;
  YYSTYPE yyval;
  YYLTYPE yyloc;

  YYJMP_BUF yyexception_buffer;
  yyGLRStackItem* yyitems;
  yyGLRStackItem* yynextFree;
  size_t yyspaceLeft;
  yyGLRState* yysplitPoint;
  yyGLRState* yylastDeleted;
  yyGLRStateSet yytops;
};

#if YYSTACKEXPANDABLE
static void yyexpandGLRStack (yyGLRStack* yystackp);
#endif

static void yyFail (yyGLRStack* yystackp, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner, const char* yymsg)
  __attribute__ ((__noreturn__));
static void
yyFail (yyGLRStack* yystackp, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner, const char* yymsg)
{
  if (yymsg != YY_NULL)
    yyerror (yylocp, parse_state, scanner, yymsg);
  YYLONGJMP (yystackp->yyexception_buffer, 1);
}

static void yyMemoryExhausted (yyGLRStack* yystackp)
  __attribute__ ((__noreturn__));
static void
yyMemoryExhausted (yyGLRStack* yystackp)
{
  YYLONGJMP (yystackp->yyexception_buffer, 2);
}

#if YYDEBUG || YYERROR_VERBOSE
/** A printable representation of TOKEN.  */
static inline const char*
yytokenName (yySymbol yytoken)
{
  if (yytoken == YYEMPTY)
    return "";

  return yytname[yytoken];
}
#endif

/** Fill in YYVSP[YYLOW1 .. YYLOW0-1] from the chain of states starting
 *  at YYVSP[YYLOW0].yystate.yypred.  Leaves YYVSP[YYLOW1].yystate.yypred
 *  containing the pointer to the next state in the chain.  */
static void yyfillin (yyGLRStackItem *, int, int) __attribute__ ((__unused__));
static void
yyfillin (yyGLRStackItem *yyvsp, int yylow0, int yylow1)
{
  int i;
  yyGLRState *s = yyvsp[yylow0].yystate.yypred;
  for (i = yylow0-1; i >= yylow1; i -= 1)
    {
      YYASSERT (s->yyresolved);
      yyvsp[i].yystate.yyresolved = yytrue;
      yyvsp[i].yystate.yysemantics.yysval = s->yysemantics.yysval;
      yyvsp[i].yystate.yyloc = s->yyloc;
      s = yyvsp[i].yystate.yypred = s->yypred;
    }
}

/* Do nothing if YYNORMAL or if *YYLOW <= YYLOW1.  Otherwise, fill in
 * YYVSP[YYLOW1 .. *YYLOW-1] as in yyfillin and set *YYLOW = YYLOW1.
 * For convenience, always return YYLOW1.  */
static inline int yyfill (yyGLRStackItem *, int *, int, yybool)
     __attribute__ ((__unused__));
static inline int
yyfill (yyGLRStackItem *yyvsp, int *yylow, int yylow1, yybool yynormal)
{
  if (!yynormal && yylow1 < *yylow)
    {
      yyfillin (yyvsp, *yylow, yylow1);
      *yylow = yylow1;
    }
  return yylow1;
}

/** Perform user action for rule number YYN, with RHS length YYRHSLEN,
 *  and top stack item YYVSP.  YYLVALP points to place to put semantic
 *  value ($$), and yylocp points to place for location information
 *  (@$).  Returns yyok for normal return, yyaccept for YYACCEPT,
 *  yyerr for YYERROR, yyabort for YYABORT.  */
/*ARGSUSED*/ static YYRESULTTAG
yyuserAction (yyRuleNum yyn, int yyrhslen, yyGLRStackItem* yyvsp,
              yyGLRStack* yystackp,
              YYSTYPE* yyvalp, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner)
{
  yybool yynormal __attribute__ ((__unused__)) =
    (yystackp->yysplitPoint == YY_NULL);
  int yylow;
  YYUSE (parse_state);
  YYUSE (scanner);
# undef yyerrok
# define yyerrok (yystackp->yyerrState = 0)
# undef YYACCEPT
# define YYACCEPT return yyaccept
# undef YYABORT
# define YYABORT return yyabort
# undef YYERROR
# define YYERROR return yyerrok, yyerr
# undef YYRECOVERING
# define YYRECOVERING() (yystackp->yyerrState != 0)
# undef yyclearin
# define yyclearin (yychar = YYEMPTY)
# undef YYFILL
# define YYFILL(N) yyfill (yyvsp, &yylow, N, yynormal)
# undef YYBACKUP
# define YYBACKUP(Token, Value)                                              \
  return yyerror (yylocp, parse_state, scanner, YY_("syntax error: cannot back up")),     \
         yyerrok, yyerr

  yylow = 1;
  if (yyrhslen == 0)
    *yyvalp = yyval_default;
  else
    *yyvalp = yyvsp[YYFILL (1-yyrhslen)].yystate.yysemantics.yysval;
  YYLLOC_DEFAULT ((*yylocp), (yyvsp - yyrhslen), yyrhslen);
  yystackp->yyerror_range[1].yystate.yyloc = *yylocp;

  switch (yyn)
    {
        case 2:
/* Line 868 of glr.c  */
#line 285 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { }
    break;

  case 3:
/* Line 868 of glr.c  */
#line 286 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 5:
/* Line 868 of glr.c  */
#line 291 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Technique; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 6:
/* Line 868 of glr.c  */
#line 292 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Parameters; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 7:
/* Line 868 of glr.c  */
#line 293 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Blocks; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 8:
/* Line 868 of glr.c  */
#line 297 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Separable; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 9:
/* Line 868 of glr.c  */
#line 298 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Sort; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 10:
/* Line 868 of glr.c  */
#line 299 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Priority; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 11:
/* Line 868 of glr.c  */
#line 300 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Transparent; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 12:
/* Line 868 of glr.c  */
#line 306 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 13:
/* Line 868 of glr.c  */
#line 311 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Technique); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 15:
/* Line 868 of glr.c  */
#line 319 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 17:
/* Line 868 of glr.c  */
#line 324 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Pass; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 19:
/* Line 868 of glr.c  */
#line 326 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Code; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 20:
/* Line 868 of glr.c  */
#line 330 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Renderer; ((*yyvalp).nodeOption).value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.strValue); }
    break;

  case 21:
/* Line 868 of glr.c  */
#line 331 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Language; ((*yyvalp).nodeOption).value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.strValue); }
    break;

  case 22:
/* Line 868 of glr.c  */
#line 332 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Tags; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 23:
/* Line 868 of glr.c  */
#line 336 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 24:
/* Line 868 of glr.c  */
#line 341 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Tags); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 26:
/* Line 868 of glr.c  */
#line 350 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			NodeOption entry; entry.type = OT_TagValue; entry.value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.strValue);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}
    break;

  case 27:
/* Line 868 of glr.c  */
#line 355 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			NodeOption entry; entry.type = OT_TagValue; entry.value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (3))].yystate.yysemantics.yysval.strValue);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}
    break;

  case 28:
/* Line 868 of glr.c  */
#line 364 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 29:
/* Line 868 of glr.c  */
#line 369 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Pass); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 31:
/* Line 868 of glr.c  */
#line 377 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 33:
/* Line 868 of glr.c  */
#line 382 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Code; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 34:
/* Line 868 of glr.c  */
#line 386 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Index; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 35:
/* Line 868 of glr.c  */
#line 387 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_FillMode; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 36:
/* Line 868 of glr.c  */
#line 388 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_CullMode; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 37:
/* Line 868 of glr.c  */
#line 389 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_DepthBias; ((*yyvalp).nodeOption).value.floatValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 38:
/* Line 868 of glr.c  */
#line 390 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_SDepthBias; ((*yyvalp).nodeOption).value.floatValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 39:
/* Line 868 of glr.c  */
#line 391 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_DepthClip; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 40:
/* Line 868 of glr.c  */
#line 392 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Scissor; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 41:
/* Line 868 of glr.c  */
#line 393 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Multisample; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 42:
/* Line 868 of glr.c  */
#line 394 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_AALine; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 43:
/* Line 868 of glr.c  */
#line 395 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_DepthRead; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 44:
/* Line 868 of glr.c  */
#line 396 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_DepthWrite; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 45:
/* Line 868 of glr.c  */
#line 397 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_CompareFunc; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 46:
/* Line 868 of glr.c  */
#line 398 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Stencil; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 47:
/* Line 868 of glr.c  */
#line 399 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_StencilReadMask; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 48:
/* Line 868 of glr.c  */
#line 400 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_StencilWriteMask; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 49:
/* Line 868 of glr.c  */
#line 401 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_StencilOpFront; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 50:
/* Line 868 of glr.c  */
#line 402 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_StencilOpBack; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 51:
/* Line 868 of glr.c  */
#line 403 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_StencilOpFront; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 52:
/* Line 868 of glr.c  */
#line 404 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_StencilOpBack; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 53:
/* Line 868 of glr.c  */
#line 405 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_AlphaToCoverage; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 54:
/* Line 868 of glr.c  */
#line 406 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_IndependantBlend; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 55:
/* Line 868 of glr.c  */
#line 407 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Target; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 56:
/* Line 868 of glr.c  */
#line 408 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_StencilRef; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 57:
/* Line 868 of glr.c  */
#line 415 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    {
		NodeOption index;
		index.type = OT_Index; 
		index.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((5) - (8))].yystate.yysemantics.yysval.intValue);

		nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &index);

		nodePop(parse_state); 
		((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (8))].yystate.yysemantics.yysval.nodePtr);
	}
    break;

  case 58:
/* Line 868 of glr.c  */
#line 429 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeVertex); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 59:
/* Line 868 of glr.c  */
#line 434 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeFragment); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 60:
/* Line 868 of glr.c  */
#line 439 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeGeometry); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 61:
/* Line 868 of glr.c  */
#line 444 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeHull); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 62:
/* Line 868 of glr.c  */
#line 449 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeDomain); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 63:
/* Line 868 of glr.c  */
#line 454 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeCompute); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 64:
/* Line 868 of glr.c  */
#line 459 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_CodeCommon); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 65:
/* Line 868 of glr.c  */
#line 469 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_StencilOp); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 66:
/* Line 868 of glr.c  */
#line 477 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_StencilOp); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 67:
/* Line 868 of glr.c  */
#line 485 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    {
			NodeOption fail; fail.type = OT_Fail; fail.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (7))].yystate.yysemantics.yysval.intValue);
			NodeOption zfail; zfail.type = OT_ZFail; zfail.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (7))].yystate.yysemantics.yysval.intValue);
			NodeOption pass; pass.type = OT_PassOp; pass.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((5) - (7))].yystate.yysemantics.yysval.intValue);
			NodeOption cmp; cmp.type = OT_CompareFunc; cmp.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((7) - (7))].yystate.yysemantics.yysval.intValue);

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &fail);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &zfail);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &pass);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &cmp);
		}
    break;

  case 69:
/* Line 868 of glr.c  */
#line 500 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 70:
/* Line 868 of glr.c  */
#line 504 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Fail; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 71:
/* Line 868 of glr.c  */
#line 505 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ZFail; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 72:
/* Line 868 of glr.c  */
#line 506 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_PassOp; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 73:
/* Line 868 of glr.c  */
#line 507 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_CompareFunc; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 74:
/* Line 868 of glr.c  */
#line 512 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 75:
/* Line 868 of glr.c  */
#line 517 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Target); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 77:
/* Line 868 of glr.c  */
#line 525 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 79:
/* Line 868 of glr.c  */
#line 533 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Index; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 80:
/* Line 868 of glr.c  */
#line 534 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Blend; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 81:
/* Line 868 of glr.c  */
#line 535 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Color; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 82:
/* Line 868 of glr.c  */
#line 536 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Alpha; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 83:
/* Line 868 of glr.c  */
#line 537 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Color; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 84:
/* Line 868 of glr.c  */
#line 538 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Alpha; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 85:
/* Line 868 of glr.c  */
#line 539 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_WriteMask; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 86:
/* Line 868 of glr.c  */
#line 545 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_BlendDef); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 87:
/* Line 868 of glr.c  */
#line 553 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_BlendDef); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 89:
/* Line 868 of glr.c  */
#line 561 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 90:
/* Line 868 of glr.c  */
#line 566 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    {
			NodeOption src; src.type = OT_Source; src.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.intValue);
			NodeOption dst; dst.type = OT_Dest; dst.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (5))].yystate.yysemantics.yysval.intValue);
			NodeOption op; op.type = OT_Op; op.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((5) - (5))].yystate.yysemantics.yysval.intValue);

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &src);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &dst);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &op);
		}
    break;

  case 91:
/* Line 868 of glr.c  */
#line 578 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Source; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 92:
/* Line 868 of glr.c  */
#line 579 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Dest; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 93:
/* Line 868 of glr.c  */
#line 580 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Op; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 95:
/* Line 868 of glr.c  */
#line 587 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 96:
/* Line 868 of glr.c  */
#line 591 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_AddrMode; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (1))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 97:
/* Line 868 of glr.c  */
#line 592 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MinFilter; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 98:
/* Line 868 of glr.c  */
#line 593 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MagFilter; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 99:
/* Line 868 of glr.c  */
#line 594 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MipFilter; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 100:
/* Line 868 of glr.c  */
#line 595 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MaxAniso; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 101:
/* Line 868 of glr.c  */
#line 596 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MipBias; ((*yyvalp).nodeOption).value.floatValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 102:
/* Line 868 of glr.c  */
#line 597 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MipMin; ((*yyvalp).nodeOption).value.floatValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 103:
/* Line 868 of glr.c  */
#line 598 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_MipMax; ((*yyvalp).nodeOption).value.floatValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 104:
/* Line 868 of glr.c  */
#line 599 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_BorderColor; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 105:
/* Line 868 of glr.c  */
#line 600 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_CompareFunc; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 106:
/* Line 868 of glr.c  */
#line 605 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 107:
/* Line 868 of glr.c  */
#line 606 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 108:
/* Line 868 of glr.c  */
#line 611 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_AddrMode); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 109:
/* Line 868 of glr.c  */
#line 619 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    {
			NodeOption u; u.type = OT_U; u.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.intValue);
			NodeOption v; v.type = OT_V; v.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (5))].yystate.yysemantics.yysval.intValue);
			NodeOption w; w.type = OT_W; w.value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((5) - (5))].yystate.yysemantics.yysval.intValue);

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &u);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &v);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &w);
		}
    break;

  case 111:
/* Line 868 of glr.c  */
#line 632 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 112:
/* Line 868 of glr.c  */
#line 636 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_U; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 113:
/* Line 868 of glr.c  */
#line 637 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_V; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 114:
/* Line 868 of glr.c  */
#line 638 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_W; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.intValue); }
    break;

  case 115:
/* Line 868 of glr.c  */
#line 643 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (5))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (5))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 116:
/* Line 868 of glr.c  */
#line 647 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (7))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (7))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (7))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 117:
/* Line 868 of glr.c  */
#line 651 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (9))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (9))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (9))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (9))].yystate.yysemantics.yysval.floatValue);}
    break;

  case 118:
/* Line 868 of glr.c  */
#line 655 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).intVectorValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (5))].yystate.yysemantics.yysval.intValue); ((*yyvalp).intVectorValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (5))].yystate.yysemantics.yysval.intValue); }
    break;

  case 119:
/* Line 868 of glr.c  */
#line 659 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).intVectorValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (7))].yystate.yysemantics.yysval.intValue); ((*yyvalp).intVectorValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (7))].yystate.yysemantics.yysval.intValue); ((*yyvalp).intVectorValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (7))].yystate.yysemantics.yysval.intValue); }
    break;

  case 120:
/* Line 868 of glr.c  */
#line 663 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).intVectorValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (9))].yystate.yysemantics.yysval.intValue); ((*yyvalp).intVectorValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (9))].yystate.yysemantics.yysval.intValue); ((*yyvalp).intVectorValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (9))].yystate.yysemantics.yysval.intValue); ((*yyvalp).intVectorValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (9))].yystate.yysemantics.yysval.intValue);}
    break;

  case 121:
/* Line 868 of glr.c  */
#line 669 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (13))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (13))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (13))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (13))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[4] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((10) - (13))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[5] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((12) - (13))].yystate.yysemantics.yysval.floatValue);
		}
    break;

  case 122:
/* Line 868 of glr.c  */
#line 679 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (17))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (17))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (17))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (17))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[4] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((10) - (17))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[5] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((12) - (17))].yystate.yysemantics.yysval.floatValue);
			((*yyvalp).matrixValue)[6] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((14) - (17))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[7] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((16) - (17))].yystate.yysemantics.yysval.floatValue);
		}
    break;

  case 123:
/* Line 868 of glr.c  */
#line 690 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (19))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (19))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (19))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (19))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[4] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((10) - (19))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[5] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((12) - (19))].yystate.yysemantics.yysval.floatValue);
			((*yyvalp).matrixValue)[6] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((14) - (19))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[7] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((16) - (19))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[8] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((18) - (19))].yystate.yysemantics.yysval.floatValue);
		}
    break;

  case 124:
/* Line 868 of glr.c  */
#line 701 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (25))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[4] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((10) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[5] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((12) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[6] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((14) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[7] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((16) - (25))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[8] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((18) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[9] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((20) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[10] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((22) - (25))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[11] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((24) - (25))].yystate.yysemantics.yysval.floatValue);
		}
    break;

  case 125:
/* Line 868 of glr.c  */
#line 713 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).matrixValue)[0] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[1] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[2] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((6) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[3] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((8) - (33))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[4] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((10) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[5] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((12) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[6] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((14) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[7] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((16) - (33))].yystate.yysemantics.yysval.floatValue); 
			((*yyvalp).matrixValue)[8] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((18) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[9] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((20) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[10] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((22) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[11] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((24) - (33))].yystate.yysemantics.yysval.floatValue);
			((*yyvalp).matrixValue)[12] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((26) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[13] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((28) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[14] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((30) - (33))].yystate.yysemantics.yysval.floatValue); ((*yyvalp).matrixValue)[15] = (((yyGLRStackItem const *)yyvsp)[YYFILL ((32) - (33))].yystate.yysemantics.yysval.floatValue);
		}
    break;

  case 126:
/* Line 868 of glr.c  */
#line 723 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 127:
/* Line 868 of glr.c  */
#line 728 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Parameters); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 129:
/* Line 868 of glr.c  */
#line 736 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 130:
/* Line 868 of glr.c  */
#line 740 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 131:
/* Line 868 of glr.c  */
#line 741 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 132:
/* Line 868 of glr.c  */
#line 742 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 133:
/* Line 868 of glr.c  */
#line 743 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 134:
/* Line 868 of glr.c  */
#line 744 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 135:
/* Line 868 of glr.c  */
#line 745 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 136:
/* Line 868 of glr.c  */
#line 746 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 137:
/* Line 868 of glr.c  */
#line 747 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 138:
/* Line 868 of glr.c  */
#line 748 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 139:
/* Line 868 of glr.c  */
#line 749 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 140:
/* Line 868 of glr.c  */
#line 750 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 141:
/* Line 868 of glr.c  */
#line 751 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 142:
/* Line 868 of glr.c  */
#line 752 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 143:
/* Line 868 of glr.c  */
#line 753 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 144:
/* Line 868 of glr.c  */
#line 754 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 145:
/* Line 868 of glr.c  */
#line 755 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 146:
/* Line 868 of glr.c  */
#line 756 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 147:
/* Line 868 of glr.c  */
#line 757 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 148:
/* Line 868 of glr.c  */
#line 758 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((3) - (4))].yystate.yysemantics.yysval.nodeOption)); nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (4))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 149:
/* Line 868 of glr.c  */
#line 759 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (3))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 150:
/* Line 868 of glr.c  */
#line 761 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			nodePop(parse_state);

			NodeOption samplerState;
			samplerState.type = OT_SamplerState;
			samplerState.value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (3))].yystate.yysemantics.yysval.nodePtr);

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &samplerState); 

			((*yyvalp).nodeOption).type = OT_Parameter; ((*yyvalp).nodeOption).value.nodePtr = parse_state->topNode; 
			nodePop(parse_state); 
		}
    break;

  case 151:
/* Line 868 of glr.c  */
#line 776 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 152:
/* Line 868 of glr.c  */
#line 780 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 153:
/* Line 868 of glr.c  */
#line 784 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 154:
/* Line 868 of glr.c  */
#line 788 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 155:
/* Line 868 of glr.c  */
#line 792 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 156:
/* Line 868 of glr.c  */
#line 796 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 157:
/* Line 868 of glr.c  */
#line 800 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 158:
/* Line 868 of glr.c  */
#line 804 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 159:
/* Line 868 of glr.c  */
#line 808 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 160:
/* Line 868 of glr.c  */
#line 812 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 161:
/* Line 868 of glr.c  */
#line 816 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 162:
/* Line 868 of glr.c  */
#line 820 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 163:
/* Line 868 of glr.c  */
#line 824 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 164:
/* Line 868 of glr.c  */
#line 828 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 165:
/* Line 868 of glr.c  */
#line 832 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 166:
/* Line 868 of glr.c  */
#line 836 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 167:
/* Line 868 of glr.c  */
#line 840 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 168:
/* Line 868 of glr.c  */
#line 844 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 169:
/* Line 868 of glr.c  */
#line 848 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 170:
/* Line 868 of glr.c  */
#line 849 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 171:
/* Line 868 of glr.c  */
#line 850 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 172:
/* Line 868 of glr.c  */
#line 851 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 173:
/* Line 868 of glr.c  */
#line 852 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 174:
/* Line 868 of glr.c  */
#line 856 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 175:
/* Line 868 of glr.c  */
#line 857 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 176:
/* Line 868 of glr.c  */
#line 858 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 177:
/* Line 868 of glr.c  */
#line 859 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 178:
/* Line 868 of glr.c  */
#line 860 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 179:
/* Line 868 of glr.c  */
#line 861 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 180:
/* Line 868 of glr.c  */
#line 862 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 181:
/* Line 868 of glr.c  */
#line 863 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 182:
/* Line 868 of glr.c  */
#line 864 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 183:
/* Line 868 of glr.c  */
#line 868 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 184:
/* Line 868 of glr.c  */
#line 869 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 185:
/* Line 868 of glr.c  */
#line 870 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 186:
/* Line 868 of glr.c  */
#line 871 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 187:
/* Line 868 of glr.c  */
#line 872 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 188:
/* Line 868 of glr.c  */
#line 873 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 189:
/* Line 868 of glr.c  */
#line 874 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ADD_PARAMETER(((*yyvalp).nodePtr), (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.intValue), (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue)); }
    break;

  case 190:
/* Line 868 of glr.c  */
#line 879 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_SamplerState); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 191:
/* Line 868 of glr.c  */
#line 886 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 192:
/* Line 868 of glr.c  */
#line 887 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; ((*yyvalp).nodeOption).value.floatValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.floatValue); }
    break;

  case 193:
/* Line 868 of glr.c  */
#line 891 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 194:
/* Line 868 of glr.c  */
#line 892 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 195:
/* Line 868 of glr.c  */
#line 896 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 196:
/* Line 868 of glr.c  */
#line 897 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 197:
/* Line 868 of glr.c  */
#line 901 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 198:
/* Line 868 of glr.c  */
#line 902 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 199:
/* Line 868 of glr.c  */
#line 906 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 200:
/* Line 868 of glr.c  */
#line 907 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intValue); }
    break;

  case 201:
/* Line 868 of glr.c  */
#line 911 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 202:
/* Line 868 of glr.c  */
#line 912 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.intVectorValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intVectorValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intVectorValue))); }
    break;

  case 203:
/* Line 868 of glr.c  */
#line 916 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 204:
/* Line 868 of glr.c  */
#line 917 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.intVectorValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intVectorValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intVectorValue))); }
    break;

  case 205:
/* Line 868 of glr.c  */
#line 921 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 206:
/* Line 868 of glr.c  */
#line 922 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.intVectorValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intVectorValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.intVectorValue))); }
    break;

  case 207:
/* Line 868 of glr.c  */
#line 926 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 208:
/* Line 868 of glr.c  */
#line 927 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 209:
/* Line 868 of glr.c  */
#line 931 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 210:
/* Line 868 of glr.c  */
#line 932 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 211:
/* Line 868 of glr.c  */
#line 936 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 212:
/* Line 868 of glr.c  */
#line 937 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 213:
/* Line 868 of glr.c  */
#line 941 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 214:
/* Line 868 of glr.c  */
#line 942 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 215:
/* Line 868 of glr.c  */
#line 946 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 216:
/* Line 868 of glr.c  */
#line 947 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamValue; memcpy(((*yyvalp).nodeOption).value.matrixValue, (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue), sizeof((((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.matrixValue))); }
    break;

  case 217:
/* Line 868 of glr.c  */
#line 951 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 218:
/* Line 868 of glr.c  */
#line 952 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { }
    break;

  case 219:
/* Line 868 of glr.c  */
#line 956 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_None; }
    break;

  case 220:
/* Line 868 of glr.c  */
#line 957 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_ParamStrValue; ((*yyvalp).nodeOption).value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue); }
    break;

  case 221:
/* Line 868 of glr.c  */
#line 962 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodePtr) = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (5))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 222:
/* Line 868 of glr.c  */
#line 967 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { 
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Blocks); 
			nodePush(parse_state, ((*yyvalp).nodePtr));
		}
    break;

  case 224:
/* Line 868 of glr.c  */
#line 975 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 225:
/* Line 868 of glr.c  */
#line 979 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodePop(parse_state); ((*yyvalp).nodeOption).type = OT_Block; ((*yyvalp).nodeOption).value.nodePtr = (((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (3))].yystate.yysemantics.yysval.nodePtr); }
    break;

  case 226:
/* Line 868 of glr.c  */
#line 984 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    {
			((*yyvalp).nodePtr) = nodeCreate(parse_state->memContext, NT_Block);
			nodePush(parse_state, ((*yyvalp).nodePtr));

			NodeOption blockName;
			blockName.type = OT_Identifier;
			blockName.value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((2) - (2))].yystate.yysemantics.yysval.strValue);

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &blockName);
		}
    break;

  case 228:
/* Line 868 of glr.c  */
#line 999 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &(((yyGLRStackItem const *)yyvsp)[YYFILL ((1) - (2))].yystate.yysemantics.yysval.nodeOption)); }
    break;

  case 229:
/* Line 868 of glr.c  */
#line 1003 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Alias; ((*yyvalp).nodeOption).value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (5))].yystate.yysemantics.yysval.strValue); }
    break;

  case 230:
/* Line 868 of glr.c  */
#line 1004 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Auto; ((*yyvalp).nodeOption).value.strValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (5))].yystate.yysemantics.yysval.strValue); }
    break;

  case 231:
/* Line 868 of glr.c  */
#line 1005 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Shared; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (5))].yystate.yysemantics.yysval.intValue); }
    break;

  case 232:
/* Line 868 of glr.c  */
#line 1006 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"
    { ((*yyvalp).nodeOption).type = OT_Usage; ((*yyvalp).nodeOption).value.intValue = (((yyGLRStackItem const *)yyvsp)[YYFILL ((4) - (5))].yystate.yysemantics.yysval.intValue); }
    break;


/* Line 868 of glr.c  */
#line 3061 "BsParserFX.c"
      default: break;
    }

  return yyok;
# undef yyerrok
# undef YYABORT
# undef YYACCEPT
# undef YYERROR
# undef YYBACKUP
# undef yyclearin
# undef YYRECOVERING
}


/*ARGSUSED*/ static void
yyuserMerge (int yyn, YYSTYPE* yy0, YYSTYPE* yy1)
{
  YYUSE (yy0);
  YYUSE (yy1);

  switch (yyn)
    {
      
      default: break;
    }
}

                              /* Bison grammar-table manipulation.  */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ParseState* parse_state, yyscan_t scanner)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (parse_state);
  YYUSE (scanner);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}

/** Number of symbols composing the right hand side of rule #RULE.  */
static inline int
yyrhsLength (yyRuleNum yyrule)
{
  return yyr2[yyrule];
}

static void
yydestroyGLRState (char const *yymsg, yyGLRState *yys, ParseState* parse_state, yyscan_t scanner)
{
  if (yys->yyresolved)
    yydestruct (yymsg, yystos[yys->yylrState],
                &yys->yysemantics.yysval, &yys->yyloc, parse_state, scanner);
  else
    {
#if YYDEBUG
      if (yydebug)
        {
          if (yys->yysemantics.yyfirstVal)
            YYFPRINTF (stderr, "%s unresolved ", yymsg);
          else
            YYFPRINTF (stderr, "%s incomplete ", yymsg);
          yy_symbol_print (stderr, yystos[yys->yylrState],
                           YY_NULL, &yys->yyloc, parse_state, scanner);
          YYFPRINTF (stderr, "\n");
        }
#endif

      if (yys->yysemantics.yyfirstVal)
        {
          yySemanticOption *yyoption = yys->yysemantics.yyfirstVal;
          yyGLRState *yyrh;
          int yyn;
          for (yyrh = yyoption->yystate, yyn = yyrhsLength (yyoption->yyrule);
               yyn > 0;
               yyrh = yyrh->yypred, yyn -= 1)
            yydestroyGLRState (yymsg, yyrh, parse_state, scanner);
        }
    }
}

/** Left-hand-side symbol for rule #RULE.  */
static inline yySymbol
yylhsNonterm (yyRuleNum yyrule)
{
  return yyr1[yyrule];
}

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-371)))

/** True iff LR state STATE has only a default reduction (regardless
 *  of token).  */
static inline yybool
yyisDefaultedState (yyStateNum yystate)
{
  return yypact_value_is_default (yypact[yystate]);
}

/** The default reduction for STATE, assuming it has one.  */
static inline yyRuleNum
yydefaultAction (yyStateNum yystate)
{
  return yydefact[yystate];
}

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

/** Set *YYACTION to the action to take in YYSTATE on seeing YYTOKEN.
 *  Result R means
 *    R < 0:  Reduce on rule -R.
 *    R = 0:  Error.
 *    R > 0:  Shift to state R.
 *  Set *CONFLICTS to a pointer into yyconfl to 0-terminated list of
 *  conflicting reductions.
 */
static inline void
yygetLRActions (yyStateNum yystate, int yytoken,
                int* yyaction, const short int** yyconflicts)
{
  int yyindex = yypact[yystate] + yytoken;
  if (yypact_value_is_default (yypact[yystate])
      || yyindex < 0 || YYLAST < yyindex || yycheck[yyindex] != yytoken)
    {
      *yyaction = -yydefact[yystate];
      *yyconflicts = yyconfl;
    }
  else if (! yytable_value_is_error (yytable[yyindex]))
    {
      *yyaction = yytable[yyindex];
      *yyconflicts = yyconfl + yyconflp[yyindex];
    }
  else
    {
      *yyaction = 0;
      *yyconflicts = yyconfl + yyconflp[yyindex];
    }
}

static inline yyStateNum
yyLRgotoState (yyStateNum yystate, yySymbol yylhs)
{
  int yyr;
  yyr = yypgoto[yylhs - YYNTOKENS] + yystate;
  if (0 <= yyr && yyr <= YYLAST && yycheck[yyr] == yystate)
    return yytable[yyr];
  else
    return yydefgoto[yylhs - YYNTOKENS];
}

static inline yybool
yyisShiftAction (int yyaction)
{
  return 0 < yyaction;
}

static inline yybool
yyisErrorAction (int yyaction)
{
  return yyaction == 0;
}

                                /* GLRStates */

/** Return a fresh GLRStackItem.  Callers should call
 * YY_RESERVE_GLRSTACK afterwards to make sure there is sufficient
 * headroom.  */

static inline yyGLRStackItem*
yynewGLRStackItem (yyGLRStack* yystackp, yybool yyisState)
{
  yyGLRStackItem* yynewItem = yystackp->yynextFree;
  yystackp->yyspaceLeft -= 1;
  yystackp->yynextFree += 1;
  yynewItem->yystate.yyisState = yyisState;
  return yynewItem;
}

/** Add a new semantic action that will execute the action for rule
 *  RULENUM on the semantic values in RHS to the list of
 *  alternative actions for STATE.  Assumes that RHS comes from
 *  stack #K of *STACKP. */
static void
yyaddDeferredAction (yyGLRStack* yystackp, size_t yyk, yyGLRState* yystate,
                     yyGLRState* rhs, yyRuleNum yyrule)
{
  yySemanticOption* yynewOption =
    &yynewGLRStackItem (yystackp, yyfalse)->yyoption;
  yynewOption->yystate = rhs;
  yynewOption->yyrule = yyrule;
  if (yystackp->yytops.yylookaheadNeeds[yyk])
    {
      yynewOption->yyrawchar = yychar;
      yynewOption->yyval = yylval;
      yynewOption->yyloc = yylloc;
    }
  else
    yynewOption->yyrawchar = YYEMPTY;
  yynewOption->yynext = yystate->yysemantics.yyfirstVal;
  yystate->yysemantics.yyfirstVal = yynewOption;

  YY_RESERVE_GLRSTACK (yystackp);
}

                                /* GLRStacks */

/** Initialize SET to a singleton set containing an empty stack.  */
static yybool
yyinitStateSet (yyGLRStateSet* yyset)
{
  yyset->yysize = 1;
  yyset->yycapacity = 16;
  yyset->yystates = (yyGLRState**) YYMALLOC (16 * sizeof yyset->yystates[0]);
  if (! yyset->yystates)
    return yyfalse;
  yyset->yystates[0] = YY_NULL;
  yyset->yylookaheadNeeds =
    (yybool*) YYMALLOC (16 * sizeof yyset->yylookaheadNeeds[0]);
  if (! yyset->yylookaheadNeeds)
    {
      YYFREE (yyset->yystates);
      return yyfalse;
    }
  return yytrue;
}

static void yyfreeStateSet (yyGLRStateSet* yyset)
{
  YYFREE (yyset->yystates);
  YYFREE (yyset->yylookaheadNeeds);
}

/** Initialize STACK to a single empty stack, with total maximum
 *  capacity for all stacks of SIZE.  */
static yybool
yyinitGLRStack (yyGLRStack* yystackp, size_t yysize)
{
  yystackp->yyerrState = 0;
  yynerrs = 0;
  yystackp->yyspaceLeft = yysize;
  yystackp->yyitems =
    (yyGLRStackItem*) YYMALLOC (yysize * sizeof yystackp->yynextFree[0]);
  if (!yystackp->yyitems)
    return yyfalse;
  yystackp->yynextFree = yystackp->yyitems;
  yystackp->yysplitPoint = YY_NULL;
  yystackp->yylastDeleted = YY_NULL;
  return yyinitStateSet (&yystackp->yytops);
}


#if YYSTACKEXPANDABLE
# define YYRELOC(YYFROMITEMS,YYTOITEMS,YYX,YYTYPE) \
  &((YYTOITEMS) - ((YYFROMITEMS) - (yyGLRStackItem*) (YYX)))->YYTYPE

/** If STACK is expandable, extend it.  WARNING: Pointers into the
    stack from outside should be considered invalid after this call.
    We always expand when there are 1 or fewer items left AFTER an
    allocation, so that we can avoid having external pointers exist
    across an allocation.  */
static void
yyexpandGLRStack (yyGLRStack* yystackp)
{
  yyGLRStackItem* yynewItems;
  yyGLRStackItem* yyp0, *yyp1;
  size_t yynewSize;
  size_t yyn;
  size_t yysize = yystackp->yynextFree - yystackp->yyitems;
  if (YYMAXDEPTH - YYHEADROOM < yysize)
    yyMemoryExhausted (yystackp);
  yynewSize = 2*yysize;
  if (YYMAXDEPTH < yynewSize)
    yynewSize = YYMAXDEPTH;
  yynewItems = (yyGLRStackItem*) YYMALLOC (yynewSize * sizeof yynewItems[0]);
  if (! yynewItems)
    yyMemoryExhausted (yystackp);
  for (yyp0 = yystackp->yyitems, yyp1 = yynewItems, yyn = yysize;
       0 < yyn;
       yyn -= 1, yyp0 += 1, yyp1 += 1)
    {
      *yyp1 = *yyp0;
      if (*(yybool *) yyp0)
        {
          yyGLRState* yys0 = &yyp0->yystate;
          yyGLRState* yys1 = &yyp1->yystate;
          if (yys0->yypred != YY_NULL)
            yys1->yypred =
              YYRELOC (yyp0, yyp1, yys0->yypred, yystate);
          if (! yys0->yyresolved && yys0->yysemantics.yyfirstVal != YY_NULL)
            yys1->yysemantics.yyfirstVal =
              YYRELOC (yyp0, yyp1, yys0->yysemantics.yyfirstVal, yyoption);
        }
      else
        {
          yySemanticOption* yyv0 = &yyp0->yyoption;
          yySemanticOption* yyv1 = &yyp1->yyoption;
          if (yyv0->yystate != YY_NULL)
            yyv1->yystate = YYRELOC (yyp0, yyp1, yyv0->yystate, yystate);
          if (yyv0->yynext != YY_NULL)
            yyv1->yynext = YYRELOC (yyp0, yyp1, yyv0->yynext, yyoption);
        }
    }
  if (yystackp->yysplitPoint != YY_NULL)
    yystackp->yysplitPoint = YYRELOC (yystackp->yyitems, yynewItems,
                                 yystackp->yysplitPoint, yystate);

  for (yyn = 0; yyn < yystackp->yytops.yysize; yyn += 1)
    if (yystackp->yytops.yystates[yyn] != YY_NULL)
      yystackp->yytops.yystates[yyn] =
        YYRELOC (yystackp->yyitems, yynewItems,
                 yystackp->yytops.yystates[yyn], yystate);
  YYFREE (yystackp->yyitems);
  yystackp->yyitems = yynewItems;
  yystackp->yynextFree = yynewItems + yysize;
  yystackp->yyspaceLeft = yynewSize - yysize;
}
#endif

static void
yyfreeGLRStack (yyGLRStack* yystackp)
{
  YYFREE (yystackp->yyitems);
  yyfreeStateSet (&yystackp->yytops);
}

/** Assuming that S is a GLRState somewhere on STACK, update the
 *  splitpoint of STACK, if needed, so that it is at least as deep as
 *  S.  */
static inline void
yyupdateSplit (yyGLRStack* yystackp, yyGLRState* yys)
{
  if (yystackp->yysplitPoint != YY_NULL && yystackp->yysplitPoint > yys)
    yystackp->yysplitPoint = yys;
}

/** Invalidate stack #K in STACK.  */
static inline void
yymarkStackDeleted (yyGLRStack* yystackp, size_t yyk)
{
  if (yystackp->yytops.yystates[yyk] != YY_NULL)
    yystackp->yylastDeleted = yystackp->yytops.yystates[yyk];
  yystackp->yytops.yystates[yyk] = YY_NULL;
}

/** Undelete the last stack that was marked as deleted.  Can only be
    done once after a deletion, and only when all other stacks have
    been deleted.  */
static void
yyundeleteLastStack (yyGLRStack* yystackp)
{
  if (yystackp->yylastDeleted == YY_NULL || yystackp->yytops.yysize != 0)
    return;
  yystackp->yytops.yystates[0] = yystackp->yylastDeleted;
  yystackp->yytops.yysize = 1;
  YYDPRINTF ((stderr, "Restoring last deleted stack as stack #0.\n"));
  yystackp->yylastDeleted = YY_NULL;
}

static inline void
yyremoveDeletes (yyGLRStack* yystackp)
{
  size_t yyi, yyj;
  yyi = yyj = 0;
  while (yyj < yystackp->yytops.yysize)
    {
      if (yystackp->yytops.yystates[yyi] == YY_NULL)
        {
          if (yyi == yyj)
            {
              YYDPRINTF ((stderr, "Removing dead stacks.\n"));
            }
          yystackp->yytops.yysize -= 1;
        }
      else
        {
          yystackp->yytops.yystates[yyj] = yystackp->yytops.yystates[yyi];
          /* In the current implementation, it's unnecessary to copy
             yystackp->yytops.yylookaheadNeeds[yyi] since, after
             yyremoveDeletes returns, the parser immediately either enters
             deterministic operation or shifts a token.  However, it doesn't
             hurt, and the code might evolve to need it.  */
          yystackp->yytops.yylookaheadNeeds[yyj] =
            yystackp->yytops.yylookaheadNeeds[yyi];
          if (yyj != yyi)
            {
              YYDPRINTF ((stderr, "Rename stack %lu -> %lu.\n",
                          (unsigned long int) yyi, (unsigned long int) yyj));
            }
          yyj += 1;
        }
      yyi += 1;
    }
}

/** Shift to a new state on stack #K of STACK, corresponding to LR state
 * LRSTATE, at input position POSN, with (resolved) semantic value SVAL.  */
static inline void
yyglrShift (yyGLRStack* yystackp, size_t yyk, yyStateNum yylrState,
            size_t yyposn,
            YYSTYPE* yyvalp, YYLTYPE* yylocp)
{
  yyGLRState* yynewState = &yynewGLRStackItem (yystackp, yytrue)->yystate;

  yynewState->yylrState = yylrState;
  yynewState->yyposn = yyposn;
  yynewState->yyresolved = yytrue;
  yynewState->yypred = yystackp->yytops.yystates[yyk];
  yynewState->yysemantics.yysval = *yyvalp;
  yynewState->yyloc = *yylocp;
  yystackp->yytops.yystates[yyk] = yynewState;

  YY_RESERVE_GLRSTACK (yystackp);
}

/** Shift stack #K of YYSTACK, to a new state corresponding to LR
 *  state YYLRSTATE, at input position YYPOSN, with the (unresolved)
 *  semantic value of YYRHS under the action for YYRULE.  */
static inline void
yyglrShiftDefer (yyGLRStack* yystackp, size_t yyk, yyStateNum yylrState,
                 size_t yyposn, yyGLRState* rhs, yyRuleNum yyrule)
{
  yyGLRState* yynewState = &yynewGLRStackItem (yystackp, yytrue)->yystate;

  yynewState->yylrState = yylrState;
  yynewState->yyposn = yyposn;
  yynewState->yyresolved = yyfalse;
  yynewState->yypred = yystackp->yytops.yystates[yyk];
  yynewState->yysemantics.yyfirstVal = YY_NULL;
  yystackp->yytops.yystates[yyk] = yynewState;

  /* Invokes YY_RESERVE_GLRSTACK.  */
  yyaddDeferredAction (yystackp, yyk, yynewState, rhs, yyrule);
}

/** Pop the symbols consumed by reduction #RULE from the top of stack
 *  #K of STACK, and perform the appropriate semantic action on their
 *  semantic values.  Assumes that all ambiguities in semantic values
 *  have been previously resolved.  Set *VALP to the resulting value,
 *  and *LOCP to the computed location (if any).  Return value is as
 *  for userAction.  */
static inline YYRESULTTAG
yydoAction (yyGLRStack* yystackp, size_t yyk, yyRuleNum yyrule,
            YYSTYPE* yyvalp, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner)
{
  int yynrhs = yyrhsLength (yyrule);

  if (yystackp->yysplitPoint == YY_NULL)
    {
      /* Standard special case: single stack.  */
      yyGLRStackItem* rhs = (yyGLRStackItem*) yystackp->yytops.yystates[yyk];
      YYASSERT (yyk == 0);
      yystackp->yynextFree -= yynrhs;
      yystackp->yyspaceLeft += yynrhs;
      yystackp->yytops.yystates[0] = & yystackp->yynextFree[-1].yystate;
      return yyuserAction (yyrule, yynrhs, rhs, yystackp,
                           yyvalp, yylocp, parse_state, scanner);
    }
  else
    {
      /* At present, doAction is never called in nondeterministic
       * mode, so this branch is never taken.  It is here in
       * anticipation of a future feature that will allow immediate
       * evaluation of selected actions in nondeterministic mode.  */
      int yyi;
      yyGLRState* yys;
      yyGLRStackItem yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
      yys = yyrhsVals[YYMAXRHS + YYMAXLEFT].yystate.yypred
        = yystackp->yytops.yystates[yyk];
      if (yynrhs == 0)
        /* Set default location.  */
        yyrhsVals[YYMAXRHS + YYMAXLEFT - 1].yystate.yyloc = yys->yyloc;
      for (yyi = 0; yyi < yynrhs; yyi += 1)
        {
          yys = yys->yypred;
          YYASSERT (yys);
        }
      yyupdateSplit (yystackp, yys);
      yystackp->yytops.yystates[yyk] = yys;
      return yyuserAction (yyrule, yynrhs, yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                           yystackp, yyvalp, yylocp, parse_state, scanner);
    }
}

#if !YYDEBUG
# define YY_REDUCE_PRINT(Args)
#else
# define YY_REDUCE_PRINT(Args)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print Args;               \
} while (YYID (0))

/*----------------------------------------------------------.
| Report that the RULE is going to be reduced on stack #K.  |
`----------------------------------------------------------*/

/*ARGSUSED*/ static inline void
yy_reduce_print (yyGLRStack* yystackp, size_t yyk, yyRuleNum yyrule,
                 YYSTYPE* yyvalp, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner)
{
  int yynrhs = yyrhsLength (yyrule);
  yybool yynormal __attribute__ ((__unused__)) =
    (yystackp->yysplitPoint == YY_NULL);
  yyGLRStackItem* yyvsp = (yyGLRStackItem*) yystackp->yytops.yystates[yyk];
  int yylow = 1;
  int yyi;
  YYUSE (yyvalp);
  YYUSE (yylocp);
  YYUSE (parse_state);
  YYUSE (scanner);
  YYFPRINTF (stderr, "Reducing stack %lu by rule %d (line %lu):\n",
             (unsigned long int) yyk, yyrule - 1,
             (unsigned long int) yyrline[yyrule]);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
                       &(((yyGLRStackItem const *)yyvsp)[YYFILL ((yyi + 1) - (yynrhs))].yystate.yysemantics.yysval)
                       , &(((yyGLRStackItem const *)yyvsp)[YYFILL ((yyi + 1) - (yynrhs))].yystate.yyloc)                       , parse_state, scanner);
      YYFPRINTF (stderr, "\n");
    }
}
#endif

/** Pop items off stack #K of STACK according to grammar rule RULE,
 *  and push back on the resulting nonterminal symbol.  Perform the
 *  semantic action associated with RULE and store its value with the
 *  newly pushed state, if FORCEEVAL or if STACK is currently
 *  unambiguous.  Otherwise, store the deferred semantic action with
 *  the new state.  If the new state would have an identical input
 *  position, LR state, and predecessor to an existing state on the stack,
 *  it is identified with that existing state, eliminating stack #K from
 *  the STACK.  In this case, the (necessarily deferred) semantic value is
 *  added to the options for the existing state's semantic value.
 */
static inline YYRESULTTAG
yyglrReduce (yyGLRStack* yystackp, size_t yyk, yyRuleNum yyrule,
             yybool yyforceEval, ParseState* parse_state, yyscan_t scanner)
{
  size_t yyposn = yystackp->yytops.yystates[yyk]->yyposn;

  if (yyforceEval || yystackp->yysplitPoint == YY_NULL)
    {
      YYSTYPE yysval;
      YYLTYPE yyloc;

      YY_REDUCE_PRINT ((yystackp, yyk, yyrule, &yysval, &yyloc, parse_state, scanner));
      YYCHK (yydoAction (yystackp, yyk, yyrule, &yysval, &yyloc, parse_state, scanner));
      YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyrule], &yysval, &yyloc);
      yyglrShift (yystackp, yyk,
                  yyLRgotoState (yystackp->yytops.yystates[yyk]->yylrState,
                                 yylhsNonterm (yyrule)),
                  yyposn, &yysval, &yyloc);
    }
  else
    {
      size_t yyi;
      int yyn;
      yyGLRState* yys, *yys0 = yystackp->yytops.yystates[yyk];
      yyStateNum yynewLRState;

      for (yys = yystackp->yytops.yystates[yyk], yyn = yyrhsLength (yyrule);
           0 < yyn; yyn -= 1)
        {
          yys = yys->yypred;
          YYASSERT (yys);
        }
      yyupdateSplit (yystackp, yys);
      yynewLRState = yyLRgotoState (yys->yylrState, yylhsNonterm (yyrule));
      YYDPRINTF ((stderr,
                  "Reduced stack %lu by rule #%d; action deferred.  Now in state %d.\n",
                  (unsigned long int) yyk, yyrule - 1, yynewLRState));
      for (yyi = 0; yyi < yystackp->yytops.yysize; yyi += 1)
        if (yyi != yyk && yystackp->yytops.yystates[yyi] != YY_NULL)
          {
            yyGLRState *yysplit = yystackp->yysplitPoint;
            yyGLRState *yyp = yystackp->yytops.yystates[yyi];
            while (yyp != yys && yyp != yysplit && yyp->yyposn >= yyposn)
              {
                if (yyp->yylrState == yynewLRState && yyp->yypred == yys)
                  {
                    yyaddDeferredAction (yystackp, yyk, yyp, yys0, yyrule);
                    yymarkStackDeleted (yystackp, yyk);
                    YYDPRINTF ((stderr, "Merging stack %lu into stack %lu.\n",
                                (unsigned long int) yyk,
                                (unsigned long int) yyi));
                    return yyok;
                  }
                yyp = yyp->yypred;
              }
          }
      yystackp->yytops.yystates[yyk] = yys;
      yyglrShiftDefer (yystackp, yyk, yynewLRState, yyposn, yys0, yyrule);
    }
  return yyok;
}

static size_t
yysplitStack (yyGLRStack* yystackp, size_t yyk)
{
  if (yystackp->yysplitPoint == YY_NULL)
    {
      YYASSERT (yyk == 0);
      yystackp->yysplitPoint = yystackp->yytops.yystates[yyk];
    }
  if (yystackp->yytops.yysize >= yystackp->yytops.yycapacity)
    {
      yyGLRState** yynewStates;
      yybool* yynewLookaheadNeeds;

      yynewStates = YY_NULL;

      if (yystackp->yytops.yycapacity
          > (YYSIZEMAX / (2 * sizeof yynewStates[0])))
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yycapacity *= 2;

      yynewStates =
        (yyGLRState**) YYREALLOC (yystackp->yytops.yystates,
                                  (yystackp->yytops.yycapacity
                                   * sizeof yynewStates[0]));
      if (yynewStates == YY_NULL)
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yystates = yynewStates;

      yynewLookaheadNeeds =
        (yybool*) YYREALLOC (yystackp->yytops.yylookaheadNeeds,
                             (yystackp->yytops.yycapacity
                              * sizeof yynewLookaheadNeeds[0]));
      if (yynewLookaheadNeeds == YY_NULL)
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yylookaheadNeeds = yynewLookaheadNeeds;
    }
  yystackp->yytops.yystates[yystackp->yytops.yysize]
    = yystackp->yytops.yystates[yyk];
  yystackp->yytops.yylookaheadNeeds[yystackp->yytops.yysize]
    = yystackp->yytops.yylookaheadNeeds[yyk];
  yystackp->yytops.yysize += 1;
  return yystackp->yytops.yysize-1;
}

/** True iff Y0 and Y1 represent identical options at the top level.
 *  That is, they represent the same rule applied to RHS symbols
 *  that produce the same terminal symbols.  */
static yybool
yyidenticalOptions (yySemanticOption* yyy0, yySemanticOption* yyy1)
{
  if (yyy0->yyrule == yyy1->yyrule)
    {
      yyGLRState *yys0, *yys1;
      int yyn;
      for (yys0 = yyy0->yystate, yys1 = yyy1->yystate,
           yyn = yyrhsLength (yyy0->yyrule);
           yyn > 0;
           yys0 = yys0->yypred, yys1 = yys1->yypred, yyn -= 1)
        if (yys0->yyposn != yys1->yyposn)
          return yyfalse;
      return yytrue;
    }
  else
    return yyfalse;
}

/** Assuming identicalOptions (Y0,Y1), destructively merge the
 *  alternative semantic values for the RHS-symbols of Y1 and Y0.  */
static void
yymergeOptionSets (yySemanticOption* yyy0, yySemanticOption* yyy1)
{
  yyGLRState *yys0, *yys1;
  int yyn;
  for (yys0 = yyy0->yystate, yys1 = yyy1->yystate,
       yyn = yyrhsLength (yyy0->yyrule);
       yyn > 0;
       yys0 = yys0->yypred, yys1 = yys1->yypred, yyn -= 1)
    {
      if (yys0 == yys1)
        break;
      else if (yys0->yyresolved)
        {
          yys1->yyresolved = yytrue;
          yys1->yysemantics.yysval = yys0->yysemantics.yysval;
        }
      else if (yys1->yyresolved)
        {
          yys0->yyresolved = yytrue;
          yys0->yysemantics.yysval = yys1->yysemantics.yysval;
        }
      else
        {
          yySemanticOption** yyz0p = &yys0->yysemantics.yyfirstVal;
          yySemanticOption* yyz1 = yys1->yysemantics.yyfirstVal;
          while (YYID (yytrue))
            {
              if (yyz1 == *yyz0p || yyz1 == YY_NULL)
                break;
              else if (*yyz0p == YY_NULL)
                {
                  *yyz0p = yyz1;
                  break;
                }
              else if (*yyz0p < yyz1)
                {
                  yySemanticOption* yyz = *yyz0p;
                  *yyz0p = yyz1;
                  yyz1 = yyz1->yynext;
                  (*yyz0p)->yynext = yyz;
                }
              yyz0p = &(*yyz0p)->yynext;
            }
          yys1->yysemantics.yyfirstVal = yys0->yysemantics.yyfirstVal;
        }
    }
}

/** Y0 and Y1 represent two possible actions to take in a given
 *  parsing state; return 0 if no combination is possible,
 *  1 if user-mergeable, 2 if Y0 is preferred, 3 if Y1 is preferred.  */
static int
yypreference (yySemanticOption* y0, yySemanticOption* y1)
{
  yyRuleNum r0 = y0->yyrule, r1 = y1->yyrule;
  int p0 = yydprec[r0], p1 = yydprec[r1];

  if (p0 == p1)
    {
      if (yymerger[r0] == 0 || yymerger[r0] != yymerger[r1])
        return 0;
      else
        return 1;
    }
  if (p0 == 0 || p1 == 0)
    return 0;
  if (p0 < p1)
    return 3;
  if (p1 < p0)
    return 2;
  return 0;
}

static YYRESULTTAG yyresolveValue (yyGLRState* yys,
                                   yyGLRStack* yystackp, ParseState* parse_state, yyscan_t scanner);


/** Resolve the previous N states starting at and including state S.  If result
 *  != yyok, some states may have been left unresolved possibly with empty
 *  semantic option chains.  Regardless of whether result = yyok, each state
 *  has been left with consistent data so that yydestroyGLRState can be invoked
 *  if necessary.  */
static YYRESULTTAG
yyresolveStates (yyGLRState* yys, int yyn,
                 yyGLRStack* yystackp, ParseState* parse_state, yyscan_t scanner)
{
  if (0 < yyn)
    {
      YYASSERT (yys->yypred);
      YYCHK (yyresolveStates (yys->yypred, yyn-1, yystackp, parse_state, scanner));
      if (! yys->yyresolved)
        YYCHK (yyresolveValue (yys, yystackp, parse_state, scanner));
    }
  return yyok;
}

/** Resolve the states for the RHS of OPT, perform its user action, and return
 *  the semantic value and location.  Regardless of whether result = yyok, all
 *  RHS states have been destroyed (assuming the user action destroys all RHS
 *  semantic values if invoked).  */
static YYRESULTTAG
yyresolveAction (yySemanticOption* yyopt, yyGLRStack* yystackp,
                 YYSTYPE* yyvalp, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner)
{
  yyGLRStackItem yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
  int yynrhs = yyrhsLength (yyopt->yyrule);
  YYRESULTTAG yyflag =
    yyresolveStates (yyopt->yystate, yynrhs, yystackp, parse_state, scanner);
  if (yyflag != yyok)
    {
      yyGLRState *yys;
      for (yys = yyopt->yystate; yynrhs > 0; yys = yys->yypred, yynrhs -= 1)
        yydestroyGLRState ("Cleanup: popping", yys, parse_state, scanner);
      return yyflag;
    }

  yyrhsVals[YYMAXRHS + YYMAXLEFT].yystate.yypred = yyopt->yystate;
  if (yynrhs == 0)
    /* Set default location.  */
    yyrhsVals[YYMAXRHS + YYMAXLEFT - 1].yystate.yyloc = yyopt->yystate->yyloc;
  {
    int yychar_current = yychar;
    YYSTYPE yylval_current = yylval;
    YYLTYPE yylloc_current = yylloc;
    yychar = yyopt->yyrawchar;
    yylval = yyopt->yyval;
    yylloc = yyopt->yyloc;
    yyflag = yyuserAction (yyopt->yyrule, yynrhs,
                           yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                           yystackp, yyvalp, yylocp, parse_state, scanner);
    yychar = yychar_current;
    yylval = yylval_current;
    yylloc = yylloc_current;
  }
  return yyflag;
}

#if YYDEBUG
static void
yyreportTree (yySemanticOption* yyx, int yyindent)
{
  int yynrhs = yyrhsLength (yyx->yyrule);
  int yyi;
  yyGLRState* yys;
  yyGLRState* yystates[1 + YYMAXRHS];
  yyGLRState yyleftmost_state;

  for (yyi = yynrhs, yys = yyx->yystate; 0 < yyi; yyi -= 1, yys = yys->yypred)
    yystates[yyi] = yys;
  if (yys == YY_NULL)
    {
      yyleftmost_state.yyposn = 0;
      yystates[0] = &yyleftmost_state;
    }
  else
    yystates[0] = yys;

  if (yyx->yystate->yyposn < yys->yyposn + 1)
    YYFPRINTF (stderr, "%*s%s -> <Rule %d, empty>\n",
               yyindent, "", yytokenName (yylhsNonterm (yyx->yyrule)),
               yyx->yyrule - 1);
  else
    YYFPRINTF (stderr, "%*s%s -> <Rule %d, tokens %lu .. %lu>\n",
               yyindent, "", yytokenName (yylhsNonterm (yyx->yyrule)),
               yyx->yyrule - 1, (unsigned long int) (yys->yyposn + 1),
               (unsigned long int) yyx->yystate->yyposn);
  for (yyi = 1; yyi <= yynrhs; yyi += 1)
    {
      if (yystates[yyi]->yyresolved)
        {
          if (yystates[yyi-1]->yyposn+1 > yystates[yyi]->yyposn)
            YYFPRINTF (stderr, "%*s%s <empty>\n", yyindent+2, "",
                       yytokenName (yyrhs[yyprhs[yyx->yyrule]+yyi-1]));
          else
            YYFPRINTF (stderr, "%*s%s <tokens %lu .. %lu>\n", yyindent+2, "",
                       yytokenName (yyrhs[yyprhs[yyx->yyrule]+yyi-1]),
                       (unsigned long int) (yystates[yyi - 1]->yyposn + 1),
                       (unsigned long int) yystates[yyi]->yyposn);
        }
      else
        yyreportTree (yystates[yyi]->yysemantics.yyfirstVal, yyindent+2);
    }
}
#endif

/*ARGSUSED*/ static YYRESULTTAG
yyreportAmbiguity (yySemanticOption* yyx0,
                   yySemanticOption* yyx1, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner)
{
  YYUSE (yyx0);
  YYUSE (yyx1);

#if YYDEBUG
  YYFPRINTF (stderr, "Ambiguity detected.\n");
  YYFPRINTF (stderr, "Option 1,\n");
  yyreportTree (yyx0, 2);
  YYFPRINTF (stderr, "\nOption 2,\n");
  yyreportTree (yyx1, 2);
  YYFPRINTF (stderr, "\n");
#endif

  yyerror (yylocp, parse_state, scanner, YY_("syntax is ambiguous"));
  return yyabort;
}

/** Starting at and including state S1, resolve the location for each of the
 *  previous N1 states that is unresolved.  The first semantic option of a state
 *  is always chosen.  */
static void
yyresolveLocations (yyGLRState* yys1, int yyn1,
                    yyGLRStack *yystackp, ParseState* parse_state, yyscan_t scanner)
{
  if (0 < yyn1)
    {
      yyresolveLocations (yys1->yypred, yyn1 - 1, yystackp, parse_state, scanner);
      if (!yys1->yyresolved)
        {
          yyGLRStackItem yyrhsloc[1 + YYMAXRHS];
          int yynrhs;
          yySemanticOption *yyoption = yys1->yysemantics.yyfirstVal;
          YYASSERT (yyoption != YY_NULL);
          yynrhs = yyrhsLength (yyoption->yyrule);
          if (yynrhs > 0)
            {
              yyGLRState *yys;
              int yyn;
              yyresolveLocations (yyoption->yystate, yynrhs,
                                  yystackp, parse_state, scanner);
              for (yys = yyoption->yystate, yyn = yynrhs;
                   yyn > 0;
                   yys = yys->yypred, yyn -= 1)
                yyrhsloc[yyn].yystate.yyloc = yys->yyloc;
            }
          else
            {
              /* Both yyresolveAction and yyresolveLocations traverse the GSS
                 in reverse rightmost order.  It is only necessary to invoke
                 yyresolveLocations on a subforest for which yyresolveAction
                 would have been invoked next had an ambiguity not been
                 detected.  Thus the location of the previous state (but not
                 necessarily the previous state itself) is guaranteed to be
                 resolved already.  */
              yyGLRState *yyprevious = yyoption->yystate;
              yyrhsloc[0].yystate.yyloc = yyprevious->yyloc;
            }
          {
            int yychar_current = yychar;
            YYSTYPE yylval_current = yylval;
            YYLTYPE yylloc_current = yylloc;
            yychar = yyoption->yyrawchar;
            yylval = yyoption->yyval;
            yylloc = yyoption->yyloc;
            YYLLOC_DEFAULT ((yys1->yyloc), yyrhsloc, yynrhs);
            yychar = yychar_current;
            yylval = yylval_current;
            yylloc = yylloc_current;
          }
        }
    }
}

/** Resolve the ambiguity represented in state S, perform the indicated
 *  actions, and set the semantic value of S.  If result != yyok, the chain of
 *  semantic options in S has been cleared instead or it has been left
 *  unmodified except that redundant options may have been removed.  Regardless
 *  of whether result = yyok, S has been left with consistent data so that
 *  yydestroyGLRState can be invoked if necessary.  */
static YYRESULTTAG
yyresolveValue (yyGLRState* yys, yyGLRStack* yystackp, ParseState* parse_state, yyscan_t scanner)
{
  yySemanticOption* yyoptionList = yys->yysemantics.yyfirstVal;
  yySemanticOption* yybest = yyoptionList;
  yySemanticOption** yypp;
  yybool yymerge = yyfalse;
  YYSTYPE yysval;
  YYRESULTTAG yyflag;
  YYLTYPE *yylocp = &yys->yyloc;

  for (yypp = &yyoptionList->yynext; *yypp != YY_NULL; )
    {
      yySemanticOption* yyp = *yypp;

      if (yyidenticalOptions (yybest, yyp))
        {
          yymergeOptionSets (yybest, yyp);
          *yypp = yyp->yynext;
        }
      else
        {
          switch (yypreference (yybest, yyp))
            {
            case 0:
              yyresolveLocations (yys, 1, yystackp, parse_state, scanner);
              return yyreportAmbiguity (yybest, yyp, yylocp, parse_state, scanner);
              break;
            case 1:
              yymerge = yytrue;
              break;
            case 2:
              break;
            case 3:
              yybest = yyp;
              yymerge = yyfalse;
              break;
            default:
              /* This cannot happen so it is not worth a YYASSERT (yyfalse),
                 but some compilers complain if the default case is
                 omitted.  */
              break;
            }
          yypp = &yyp->yynext;
        }
    }

  if (yymerge)
    {
      yySemanticOption* yyp;
      int yyprec = yydprec[yybest->yyrule];
      yyflag = yyresolveAction (yybest, yystackp, &yysval, yylocp, parse_state, scanner);
      if (yyflag == yyok)
        for (yyp = yybest->yynext; yyp != YY_NULL; yyp = yyp->yynext)
          {
            if (yyprec == yydprec[yyp->yyrule])
              {
                YYSTYPE yysval_other;
                YYLTYPE yydummy;
                yyflag = yyresolveAction (yyp, yystackp, &yysval_other, &yydummy, parse_state, scanner);
                if (yyflag != yyok)
                  {
                    yydestruct ("Cleanup: discarding incompletely merged value for",
                                yystos[yys->yylrState],
                                &yysval, yylocp, parse_state, scanner);
                    break;
                  }
                yyuserMerge (yymerger[yyp->yyrule], &yysval, &yysval_other);
              }
          }
    }
  else
    yyflag = yyresolveAction (yybest, yystackp, &yysval, yylocp, parse_state, scanner);

  if (yyflag == yyok)
    {
      yys->yyresolved = yytrue;
      yys->yysemantics.yysval = yysval;
    }
  else
    yys->yysemantics.yyfirstVal = YY_NULL;
  return yyflag;
}

static YYRESULTTAG
yyresolveStack (yyGLRStack* yystackp, ParseState* parse_state, yyscan_t scanner)
{
  if (yystackp->yysplitPoint != YY_NULL)
    {
      yyGLRState* yys;
      int yyn;

      for (yyn = 0, yys = yystackp->yytops.yystates[0];
           yys != yystackp->yysplitPoint;
           yys = yys->yypred, yyn += 1)
        continue;
      YYCHK (yyresolveStates (yystackp->yytops.yystates[0], yyn, yystackp
                             , parse_state, scanner));
    }
  return yyok;
}

static void
yycompressStack (yyGLRStack* yystackp)
{
  yyGLRState* yyp, *yyq, *yyr;

  if (yystackp->yytops.yysize != 1 || yystackp->yysplitPoint == YY_NULL)
    return;

  for (yyp = yystackp->yytops.yystates[0], yyq = yyp->yypred, yyr = YY_NULL;
       yyp != yystackp->yysplitPoint;
       yyr = yyp, yyp = yyq, yyq = yyp->yypred)
    yyp->yypred = yyr;

  yystackp->yyspaceLeft += yystackp->yynextFree - yystackp->yyitems;
  yystackp->yynextFree = ((yyGLRStackItem*) yystackp->yysplitPoint) + 1;
  yystackp->yyspaceLeft -= yystackp->yynextFree - yystackp->yyitems;
  yystackp->yysplitPoint = YY_NULL;
  yystackp->yylastDeleted = YY_NULL;

  while (yyr != YY_NULL)
    {
      yystackp->yynextFree->yystate = *yyr;
      yyr = yyr->yypred;
      yystackp->yynextFree->yystate.yypred = &yystackp->yynextFree[-1].yystate;
      yystackp->yytops.yystates[0] = &yystackp->yynextFree->yystate;
      yystackp->yynextFree += 1;
      yystackp->yyspaceLeft -= 1;
    }
}

static YYRESULTTAG
yyprocessOneStack (yyGLRStack* yystackp, size_t yyk,
                   size_t yyposn, YYLTYPE *yylocp, ParseState* parse_state, yyscan_t scanner)
{
  int yyaction;
  const short int* yyconflicts;
  yyRuleNum yyrule;

  while (yystackp->yytops.yystates[yyk] != YY_NULL)
    {
      yyStateNum yystate = yystackp->yytops.yystates[yyk]->yylrState;
      YYDPRINTF ((stderr, "Stack %lu Entering state %d\n",
                  (unsigned long int) yyk, yystate));

      YYASSERT (yystate != YYFINAL);

      if (yyisDefaultedState (yystate))
        {
          yyrule = yydefaultAction (yystate);
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, "Stack %lu dies.\n",
                          (unsigned long int) yyk));
              yymarkStackDeleted (yystackp, yyk);
              return yyok;
            }
          YYCHK (yyglrReduce (yystackp, yyk, yyrule, yyfalse, parse_state, scanner));
        }
      else
        {
          yySymbol yytoken;
          yystackp->yytops.yylookaheadNeeds[yyk] = yytrue;
          if (yychar == YYEMPTY)
            {
              YYDPRINTF ((stderr, "Reading a token: "));
              yychar = YYLEX;
            }

          if (yychar <= YYEOF)
            {
              yychar = yytoken = YYEOF;
              YYDPRINTF ((stderr, "Now at end of input.\n"));
            }
          else
            {
              yytoken = YYTRANSLATE (yychar);
              YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
            }

          yygetLRActions (yystate, yytoken, &yyaction, &yyconflicts);

          while (*yyconflicts != 0)
            {
              size_t yynewStack = yysplitStack (yystackp, yyk);
              YYDPRINTF ((stderr, "Splitting off stack %lu from %lu.\n",
                          (unsigned long int) yynewStack,
                          (unsigned long int) yyk));
              YYCHK (yyglrReduce (yystackp, yynewStack,
                                  *yyconflicts, yyfalse, parse_state, scanner));
              YYCHK (yyprocessOneStack (yystackp, yynewStack,
                                        yyposn, yylocp, parse_state, scanner));
              yyconflicts += 1;
            }

          if (yyisShiftAction (yyaction))
            break;
          else if (yyisErrorAction (yyaction))
            {
              YYDPRINTF ((stderr, "Stack %lu dies.\n",
                          (unsigned long int) yyk));
              yymarkStackDeleted (yystackp, yyk);
              break;
            }
          else
            YYCHK (yyglrReduce (yystackp, yyk, -yyaction,
                                yyfalse, parse_state, scanner));
        }
    }
  return yyok;
}

/*ARGSUSED*/ static void
yyreportSyntaxError (yyGLRStack* yystackp, ParseState* parse_state, yyscan_t scanner)
{
  if (yystackp->yyerrState != 0)
    return;
#if ! YYERROR_VERBOSE
  yyerror (&yylloc, parse_state, scanner, YY_("syntax error"));
#else
  {
  yySymbol yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);
  size_t yysize0 = yytnamerr (YY_NULL, yytokenName (yytoken));
  size_t yysize = yysize0;
  yybool yysize_overflow = yyfalse;
  char* yymsg = YY_NULL;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected").  */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[yystackp->yytops.yystates[0]->yylrState];
      yyarg[yycount++] = yytokenName (yytoken);
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for this
             state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;
          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytokenName (yyx);
                {
                  size_t yysz = yysize + yytnamerr (YY_NULL, yytokenName (yyx));
                  yysize_overflow |= yysz < yysize;
                  yysize = yysz;
                }
              }
        }
    }

  switch (yycount)
    {
#define YYCASE_(N, S)                   \
      case N:                           \
        yyformat = S;                   \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  {
    size_t yysz = yysize + strlen (yyformat);
    yysize_overflow |= yysz < yysize;
    yysize = yysz;
  }

  if (!yysize_overflow)
    yymsg = (char *) YYMALLOC (yysize);

  if (yymsg)
    {
      char *yyp = yymsg;
      int yyi = 0;
      while ((*yyp = *yyformat))
        {
          if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
            {
              yyp += yytnamerr (yyp, yyarg[yyi++]);
              yyformat += 2;
            }
          else
            {
              yyp++;
              yyformat++;
            }
        }
      yyerror (&yylloc, parse_state, scanner, yymsg);
      YYFREE (yymsg);
    }
  else
    {
      yyerror (&yylloc, parse_state, scanner, YY_("syntax error"));
      yyMemoryExhausted (yystackp);
    }
  }
#endif /* YYERROR_VERBOSE */
  yynerrs += 1;
}

/* Recover from a syntax error on *YYSTACKP, assuming that *YYSTACKP->YYTOKENP,
   yylval, and yylloc are the syntactic category, semantic value, and location
   of the lookahead.  */
/*ARGSUSED*/ static void
yyrecoverSyntaxError (yyGLRStack* yystackp, ParseState* parse_state, yyscan_t scanner)
{
  size_t yyk;
  int yyj;

  if (yystackp->yyerrState == 3)
    /* We just shifted the error token and (perhaps) took some
       reductions.  Skip tokens until we can proceed.  */
    while (YYID (yytrue))
      {
        yySymbol yytoken;
        if (yychar == YYEOF)
          yyFail (yystackp, &yylloc, parse_state, scanner, YY_NULL);
        if (yychar != YYEMPTY)
          {
            /* We throw away the lookahead, but the error range
               of the shifted error token must take it into account.  */
            yyGLRState *yys = yystackp->yytops.yystates[0];
            yyGLRStackItem yyerror_range[3];
            yyerror_range[1].yystate.yyloc = yys->yyloc;
            yyerror_range[2].yystate.yyloc = yylloc;
            YYLLOC_DEFAULT ((yys->yyloc), yyerror_range, 2);
            yytoken = YYTRANSLATE (yychar);
            yydestruct ("Error: discarding",
                        yytoken, &yylval, &yylloc, parse_state, scanner);
          }
        YYDPRINTF ((stderr, "Reading a token: "));
        yychar = YYLEX;
        if (yychar <= YYEOF)
          {
            yychar = yytoken = YYEOF;
            YYDPRINTF ((stderr, "Now at end of input.\n"));
          }
        else
          {
            yytoken = YYTRANSLATE (yychar);
            YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
          }
        yyj = yypact[yystackp->yytops.yystates[0]->yylrState];
        if (yypact_value_is_default (yyj))
          return;
        yyj += yytoken;
        if (yyj < 0 || YYLAST < yyj || yycheck[yyj] != yytoken)
          {
            if (yydefact[yystackp->yytops.yystates[0]->yylrState] != 0)
              return;
          }
        else if (! yytable_value_is_error (yytable[yyj]))
          return;
      }

  /* Reduce to one stack.  */
  for (yyk = 0; yyk < yystackp->yytops.yysize; yyk += 1)
    if (yystackp->yytops.yystates[yyk] != YY_NULL)
      break;
  if (yyk >= yystackp->yytops.yysize)
    yyFail (yystackp, &yylloc, parse_state, scanner, YY_NULL);
  for (yyk += 1; yyk < yystackp->yytops.yysize; yyk += 1)
    yymarkStackDeleted (yystackp, yyk);
  yyremoveDeletes (yystackp);
  yycompressStack (yystackp);

  /* Now pop stack until we find a state that shifts the error token.  */
  yystackp->yyerrState = 3;
  while (yystackp->yytops.yystates[0] != YY_NULL)
    {
      yyGLRState *yys = yystackp->yytops.yystates[0];
      yyj = yypact[yys->yylrState];
      if (! yypact_value_is_default (yyj))
        {
          yyj += YYTERROR;
          if (0 <= yyj && yyj <= YYLAST && yycheck[yyj] == YYTERROR
              && yyisShiftAction (yytable[yyj]))
            {
              /* Shift the error token.  */
              /* First adjust its location.*/
              YYLTYPE yyerrloc;
              yystackp->yyerror_range[2].yystate.yyloc = yylloc;
              YYLLOC_DEFAULT (yyerrloc, (yystackp->yyerror_range), 2);
              YY_SYMBOL_PRINT ("Shifting", yystos[yytable[yyj]],
                               &yylval, &yyerrloc);
              yyglrShift (yystackp, 0, yytable[yyj],
                          yys->yyposn, &yylval, &yyerrloc);
              yys = yystackp->yytops.yystates[0];
              break;
            }
        }
      yystackp->yyerror_range[1].yystate.yyloc = yys->yyloc;
      if (yys->yypred != YY_NULL)
        yydestroyGLRState ("Error: popping", yys, parse_state, scanner);
      yystackp->yytops.yystates[0] = yys->yypred;
      yystackp->yynextFree -= 1;
      yystackp->yyspaceLeft += 1;
    }
  if (yystackp->yytops.yystates[0] == YY_NULL)
    yyFail (yystackp, &yylloc, parse_state, scanner, YY_NULL);
}

#define YYCHK1(YYE)                                                          \
  do {                                                                       \
    switch (YYE) {                                                           \
    case yyok:                                                               \
      break;                                                                 \
    case yyabort:                                                            \
      goto yyabortlab;                                                       \
    case yyaccept:                                                           \
      goto yyacceptlab;                                                      \
    case yyerr:                                                              \
      goto yyuser_error;                                                     \
    default:                                                                 \
      goto yybuglab;                                                         \
    }                                                                        \
  } while (YYID (0))


/*----------.
| yyparse.  |
`----------*/

int
yyparse (ParseState* parse_state, yyscan_t scanner)
{
  int yyresult;
  yyGLRStack yystack;
  yyGLRStack* const yystackp = &yystack;
  size_t yyposn;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY;
  yylval = yyval_default;
  yylloc = yyloc_default;

  if (! yyinitGLRStack (yystackp, YYINITDEPTH))
    goto yyexhaustedlab;
  switch (YYSETJMP (yystack.yyexception_buffer))
    {
    case 0: break;
    case 1: goto yyabortlab;
    case 2: goto yyexhaustedlab;
    default: goto yybuglab;
    }
  yyglrShift (&yystack, 0, 0, 0, &yylval, &yylloc);
  yyposn = 0;

  while (YYID (yytrue))
    {
      /* For efficiency, we have two loops, the first of which is
         specialized to deterministic operation (single stack, no
         potential ambiguity).  */
      /* Standard mode */
      while (YYID (yytrue))
        {
          yyRuleNum yyrule;
          int yyaction;
          const short int* yyconflicts;

          yyStateNum yystate = yystack.yytops.yystates[0]->yylrState;
          YYDPRINTF ((stderr, "Entering state %d\n", yystate));
          if (yystate == YYFINAL)
            goto yyacceptlab;
          if (yyisDefaultedState (yystate))
            {
              yyrule = yydefaultAction (yystate);
              if (yyrule == 0)
                {
               yystack.yyerror_range[1].yystate.yyloc = yylloc;
                  yyreportSyntaxError (&yystack, parse_state, scanner);
                  goto yyuser_error;
                }
              YYCHK1 (yyglrReduce (&yystack, 0, yyrule, yytrue, parse_state, scanner));
            }
          else
            {
              yySymbol yytoken;
              if (yychar == YYEMPTY)
                {
                  YYDPRINTF ((stderr, "Reading a token: "));
                  yychar = YYLEX;
                }

              if (yychar <= YYEOF)
                {
                  yychar = yytoken = YYEOF;
                  YYDPRINTF ((stderr, "Now at end of input.\n"));
                }
              else
                {
                  yytoken = YYTRANSLATE (yychar);
                  YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
                }

              yygetLRActions (yystate, yytoken, &yyaction, &yyconflicts);
              if (*yyconflicts != 0)
                break;
              if (yyisShiftAction (yyaction))
                {
                  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
                  yychar = YYEMPTY;
                  yyposn += 1;
                  yyglrShift (&yystack, 0, yyaction, yyposn, &yylval, &yylloc);
                  if (0 < yystack.yyerrState)
                    yystack.yyerrState -= 1;
                }
              else if (yyisErrorAction (yyaction))
                {
               yystack.yyerror_range[1].yystate.yyloc = yylloc;
                  yyreportSyntaxError (&yystack, parse_state, scanner);
                  goto yyuser_error;
                }
              else
                YYCHK1 (yyglrReduce (&yystack, 0, -yyaction, yytrue, parse_state, scanner));
            }
        }

      while (YYID (yytrue))
        {
          yySymbol yytoken_to_shift;
          size_t yys;

          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            yystackp->yytops.yylookaheadNeeds[yys] = yychar != YYEMPTY;

          /* yyprocessOneStack returns one of three things:

              - An error flag.  If the caller is yyprocessOneStack, it
                immediately returns as well.  When the caller is finally
                yyparse, it jumps to an error label via YYCHK1.

              - yyok, but yyprocessOneStack has invoked yymarkStackDeleted
                (&yystack, yys), which sets the top state of yys to NULL.  Thus,
                yyparse's following invocation of yyremoveDeletes will remove
                the stack.

              - yyok, when ready to shift a token.

             Except in the first case, yyparse will invoke yyremoveDeletes and
             then shift the next token onto all remaining stacks.  This
             synchronization of the shift (that is, after all preceding
             reductions on all stacks) helps prevent double destructor calls
             on yylval in the event of memory exhaustion.  */

          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            YYCHK1 (yyprocessOneStack (&yystack, yys, yyposn, &yylloc, parse_state, scanner));
          yyremoveDeletes (&yystack);
          if (yystack.yytops.yysize == 0)
            {
              yyundeleteLastStack (&yystack);
              if (yystack.yytops.yysize == 0)
                yyFail (&yystack, &yylloc, parse_state, scanner, YY_("syntax error"));
              YYCHK1 (yyresolveStack (&yystack, parse_state, scanner));
              YYDPRINTF ((stderr, "Returning to deterministic operation.\n"));
           yystack.yyerror_range[1].yystate.yyloc = yylloc;
              yyreportSyntaxError (&yystack, parse_state, scanner);
              goto yyuser_error;
            }

          /* If any yyglrShift call fails, it will fail after shifting.  Thus,
             a copy of yylval will already be on stack 0 in the event of a
             failure in the following loop.  Thus, yychar is set to YYEMPTY
             before the loop to make sure the user destructor for yylval isn't
             called twice.  */
          yytoken_to_shift = YYTRANSLATE (yychar);
          yychar = YYEMPTY;
          yyposn += 1;
          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            {
              int yyaction;
              const short int* yyconflicts;
              yyStateNum yystate = yystack.yytops.yystates[yys]->yylrState;
              yygetLRActions (yystate, yytoken_to_shift, &yyaction,
                              &yyconflicts);
              /* Note that yyconflicts were handled by yyprocessOneStack.  */
              YYDPRINTF ((stderr, "On stack %lu, ", (unsigned long int) yys));
              YY_SYMBOL_PRINT ("shifting", yytoken_to_shift, &yylval, &yylloc);
              yyglrShift (&yystack, yys, yyaction, yyposn,
                          &yylval, &yylloc);
              YYDPRINTF ((stderr, "Stack %lu now in state #%d\n",
                          (unsigned long int) yys,
                          yystack.yytops.yystates[yys]->yylrState));
            }

          if (yystack.yytops.yysize == 1)
            {
              YYCHK1 (yyresolveStack (&yystack, parse_state, scanner));
              YYDPRINTF ((stderr, "Returning to deterministic operation.\n"));
              yycompressStack (&yystack);
              break;
            }
        }
      continue;
    yyuser_error:
      yyrecoverSyntaxError (&yystack, parse_state, scanner);
      yyposn = yystack.yytops.yystates[0]->yyposn;
    }

 yyacceptlab:
  yyresult = 0;
  goto yyreturn;

 yybuglab:
  YYASSERT (yyfalse);
  goto yyabortlab;

 yyabortlab:
  yyresult = 1;
  goto yyreturn;

 yyexhaustedlab:
  yyerror (&yylloc, parse_state, scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;

 yyreturn:
  if (yychar != YYEMPTY)
    yydestruct ("Cleanup: discarding lookahead",
                YYTRANSLATE (yychar), &yylval, &yylloc, parse_state, scanner);

  /* If the stack is well-formed, pop the stack until it is empty,
     destroying its entries as we go.  But free the stack regardless
     of whether it is well-formed.  */
  if (yystack.yyitems)
    {
      yyGLRState** yystates = yystack.yytops.yystates;
      if (yystates)
        {
          size_t yysize = yystack.yytops.yysize;
          size_t yyk;
          for (yyk = 0; yyk < yysize; yyk += 1)
            if (yystates[yyk])
              {
                while (yystates[yyk])
                  {
                    yyGLRState *yys = yystates[yyk];
                 yystack.yyerror_range[1].yystate.yyloc = yys->yyloc;
                  if (yys->yypred != YY_NULL)
                      yydestroyGLRState ("Cleanup: popping", yys, parse_state, scanner);
                    yystates[yyk] = yys->yypred;
                    yystack.yynextFree -= 1;
                    yystack.yyspaceLeft += 1;
                  }
                break;
              }
        }
      yyfreeGLRStack (&yystack);
    }

  /* Make sure YYID is used.  */
  return YYID (yyresult);
}

/* DEBUGGING ONLY */
#if YYDEBUG
static void yypstack (yyGLRStack* yystackp, size_t yyk)
  __attribute__ ((__unused__));
static void yypdumpstack (yyGLRStack* yystackp) __attribute__ ((__unused__));

static void
yy_yypstack (yyGLRState* yys)
{
  if (yys->yypred)
    {
      yy_yypstack (yys->yypred);
      YYFPRINTF (stderr, " -> ");
    }
  YYFPRINTF (stderr, "%d@%lu", yys->yylrState,
             (unsigned long int) yys->yyposn);
}

static void
yypstates (yyGLRState* yyst)
{
  if (yyst == YY_NULL)
    YYFPRINTF (stderr, "<null>");
  else
    yy_yypstack (yyst);
  YYFPRINTF (stderr, "\n");
}

static void
yypstack (yyGLRStack* yystackp, size_t yyk)
{
  yypstates (yystackp->yytops.yystates[yyk]);
}

#define YYINDEX(YYX)                                                         \
    ((YYX) == YY_NULL ? -1 : (yyGLRStackItem*) (YYX) - yystackp->yyitems)


static void
yypdumpstack (yyGLRStack* yystackp)
{
  yyGLRStackItem* yyp;
  size_t yyi;
  for (yyp = yystackp->yyitems; yyp < yystackp->yynextFree; yyp += 1)
    {
      YYFPRINTF (stderr, "%3lu. ",
                 (unsigned long int) (yyp - yystackp->yyitems));
      if (*(yybool *) yyp)
        {
          YYFPRINTF (stderr, "Res: %d, LR State: %d, posn: %lu, pred: %ld",
                     yyp->yystate.yyresolved, yyp->yystate.yylrState,
                     (unsigned long int) yyp->yystate.yyposn,
                     (long int) YYINDEX (yyp->yystate.yypred));
          if (! yyp->yystate.yyresolved)
            YYFPRINTF (stderr, ", firstVal: %ld",
                       (long int) YYINDEX (yyp->yystate
                                             .yysemantics.yyfirstVal));
        }
      else
        {
          YYFPRINTF (stderr, "Option. rule: %d, state: %ld, next: %ld",
                     yyp->yyoption.yyrule - 1,
                     (long int) YYINDEX (yyp->yyoption.yystate),
                     (long int) YYINDEX (yyp->yyoption.yynext));
        }
      YYFPRINTF (stderr, "\n");
    }
  YYFPRINTF (stderr, "Tops:");
  for (yyi = 0; yyi < yystackp->yytops.yysize; yyi += 1)
    YYFPRINTF (stderr, "%lu: %ld; ", (unsigned long int) yyi,
               (long int) YYINDEX (yystackp->yytops.yystates[yyi]));
  YYFPRINTF (stderr, "\n");
}
#endif
/* Line 2575 of glr.c  */
#line 1008 "C:/Projects/BansheeEngineDev/BansheeEngine/Source/BansheeSL/BsParserFX.y"


void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg) 
{ 
	parse_state->hasError = 1;
	parse_state->errorLine = locp->first_line;
	parse_state->errorColumn = locp->first_column;
	parse_state->errorMessage = mmalloc_strdup(parse_state->memContext, msg);
	parse_state->errorFile = locp->filename;
}