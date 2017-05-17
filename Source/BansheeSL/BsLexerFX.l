%{
#include "BsParserFX.h"

#define YY_USER_ACTION yylloc->first_column = yycolumn + 1; yylloc->first_line = yylineno + 1; yycolumn += (int)yyleng; yylloc->filename = getCurrentFilename(yyextra);
#define YY_USER_INIT yylineno = 0; yycolumn = 0;
%}
 
%option yylineno reentrant noyywrap nounistd never-interactive warn nodefault bison-bridge bison-locations
%option outfile="BsLexerFX.c" header-file="BsLexerFX.h"
%option extra-type="struct tagParseState *"

INTEGER			-?[0-9][0-9]*
INTEGER_16		0[xX][0-9a-fA-F]+
FLOAT			[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?[fF]?
STRING			\"[^"\n]*\"
IDENTIFIER		[_a-zA-Z][_a-zA-Z0-9]*
WS				[ \r\n\t]*
SPACE			[ \t]
SINGLEWS		[ \r\n\t]
ENDLINE			[\r\n]
COMMENT			\/\/[^\n]*
DEFINE_EXPR		[^\r\n]*

	/* Start conditions */
%x INCLUDE
%x CODEBLOCK_HEADER
%x CODEBLOCK_EQUALS
%x CODEBLOCK
%x CODEBLOCK_END
%x DEFINE_COND
%x DEFINE_COND_EXPR
%x UNDEF_COND
%x CONDITIONAL_IF
%x CONDITIONAL_IFN
%x CONDITIONAL_ELIF
%x CONDITIONAL_IGNORE

%%

{WS}			{ /* Skip blank */ }
{INTEGER}       { yylval->intValue = atoi(yytext); return TOKEN_INTEGER; }
{INTEGER_16}    { yylval->intValue = (int)strtol(yytext, 0, 0); return TOKEN_INTEGER; }
{FLOAT}			{ yylval->floatValue = (float)atof(yytext); return TOKEN_FLOAT; }
{STRING}		{ yylval->strValue = mmalloc_strdup(yyextra->memContext, yytext); return TOKEN_STRING; }
true			{ yylval->intValue = 1; return TOKEN_BOOLEAN; }
false			{ yylval->intValue = 0; return TOKEN_BOOLEAN; }

	/* Shader keywords */
options			{ return TOKEN_OPTIONS; }
technique		{ return TOKEN_TECHNIQUE; }
mixin			{ return TOKEN_MIXIN; }

	/* Options keywords */
separable		{ return TOKEN_SEPARABLE; }
sort			{ return TOKEN_SORT; }
priority		{ return TOKEN_PRIORITY; }
transparent		{ return TOKEN_TRANSPARENT; }

	/* Technique keywords */
renderer		{ return TOKEN_RENDERER; }
tags			{ return TOKEN_TAGS; }
pass			{ return TOKEN_PASS; }

	/* Pass keywords */
blend			{ return TOKEN_BLEND; }
raster			{ return TOKEN_RASTER; }
depth			{ return TOKEN_DEPTH; }
stencil			{ return TOKEN_STENCIL; }

	/* Rasterizer state keywords */
fill			{ return TOKEN_FILLMODE; }
cull			{ return TOKEN_CULLMODE; }
bias			{ return TOKEN_DEPTHBIAS; }
scaledbias		{ return TOKEN_SDEPTHBIAS; }
clip			{ return TOKEN_DEPTHCLIP; }
scissor			{ return TOKEN_SCISSOR; }
multisample		{ return TOKEN_MULTISAMPLE; }
lineaa			{ return TOKEN_AALINE; }

	/* Depth state keywords */
read			{ return TOKEN_DEPTHREAD; }
write			{ return TOKEN_DEPTHWRITE; }
compare			{ return TOKEN_COMPAREFUNC; }

	/* Stencil state keywords */
reference			{ return TOKEN_STENCILREF; }
enabled				{ return TOKEN_ENABLED; }
readmask			{ return TOKEN_READMASK; }
writemask			{ return TOKEN_WRITEMASK; }
front				{ return TOKEN_STENCILOPFRONT; }
back				{ return TOKEN_STENCILOPBACK; }
fail				{ return TOKEN_FAIL; }
zfail				{ return TOKEN_ZFAIL; }

	/* Blend state keywords */
dither				{ return TOKEN_ALPHATOCOVERAGE; }
independant			{ return TOKEN_INDEPENDANTBLEND; }
target				{ return TOKEN_TARGET; }
index				{ return TOKEN_INDEX; }
color				{ return TOKEN_COLOR; }
alpha				{ return TOKEN_ALPHA; }
source				{ return TOKEN_SOURCE; }
dest				{ return TOKEN_DEST; }
op					{ return TOKEN_OP; }

	/* State values */
wire			{ yylval->intValue = FMV_Wire; return TOKEN_FILLMODEVALUE; }
solid			{ yylval->intValue = FMV_Solid; return TOKEN_FILLMODEVALUE; }

none			{ yylval->intValue = CASV_None; return TOKEN_CULLANDQUEUEVALUE; }
cw				{ yylval->intValue = CASV_CW; return TOKEN_CULLANDQUEUEVALUE; }
ccw				{ yylval->intValue = CASV_CCW; return TOKEN_CULLANDQUEUEVALUE; }
fronttoback		{ yylval->intValue = CASV_FrontToBack; return TOKEN_CULLANDQUEUEVALUE; }
backtofront		{ yylval->intValue = CASV_BackToFront; return TOKEN_CULLANDQUEUEVALUE; }

never			{ yylval->intValue = CFV_Fail; return TOKEN_COMPFUNCVALUE; }
always			{ yylval->intValue = CFV_Pass; return TOKEN_COMPFUNCVALUE; }
lt				{ yylval->intValue = CFV_LT; return TOKEN_COMPFUNCVALUE; }
lte				{ yylval->intValue = CFV_LTE; return TOKEN_COMPFUNCVALUE; }
eq				{ yylval->intValue = CFV_EQ; return TOKEN_COMPFUNCVALUE; }
neq				{ yylval->intValue = CFV_NEQ; return TOKEN_COMPFUNCVALUE; }
gte				{ yylval->intValue = CFV_GTE; return TOKEN_COMPFUNCVALUE; }
gt				{ yylval->intValue = CFV_GT; return TOKEN_COMPFUNCVALUE; }

keep			{ yylval->intValue = OV_Keep; return TOKEN_OPVALUE; }
zero			{ yylval->intValue = OV_Zero; return TOKEN_OPVALUE; }
replace			{ yylval->intValue = OV_Replace; return TOKEN_OPVALUE; }
inc				{ yylval->intValue = OV_Incr; return TOKEN_OPVALUE; }
dec				{ yylval->intValue = OV_Decr; return TOKEN_OPVALUE; }
incwrap			{ yylval->intValue = OV_IncrWrap; return TOKEN_OPVALUE; }
decwrap			{ yylval->intValue = OV_DecrWrap; return TOKEN_OPVALUE; }
inverse			{ yylval->intValue = OV_Invert; return TOKEN_OPVALUE; }
one				{ yylval->intValue = OV_One; return TOKEN_OPVALUE; }
dstRGB			{ yylval->intValue = OV_DestColor; return TOKEN_OPVALUE; }
srcRGB			{ yylval->intValue = OV_SrcColor; return TOKEN_OPVALUE; }
dstiRGB			{ yylval->intValue = OV_InvDestColor; return TOKEN_OPVALUE; }
srciRGB			{ yylval->intValue = OV_InvSrcColor; return TOKEN_OPVALUE; }
dstA			{ yylval->intValue = OV_DestAlpha; return TOKEN_OPVALUE; }
srcA			{ yylval->intValue = OV_SrcAlpha; return TOKEN_OPVALUE; }
dstIA			{ yylval->intValue = OV_InvDestAlpha; return TOKEN_OPVALUE; }
srcIA			{ yylval->intValue = OV_InvSrcAlpha; return TOKEN_OPVALUE; }

add				{ yylval->intValue = BOV_Add; return TOKEN_BLENDOPVALUE; }
sub				{ yylval->intValue = BOV_Subtract; return TOKEN_BLENDOPVALUE; }
rsub			{ yylval->intValue = BOV_RevSubtract; return TOKEN_BLENDOPVALUE; }
min				{ yylval->intValue = BOV_Min; return TOKEN_BLENDOPVALUE; }
max				{ yylval->intValue = BOV_Max; return TOKEN_BLENDOPVALUE; }

R				{ yylval->intValue = 0x1; return TOKEN_COLORMASK; }
G				{ yylval->intValue = 0x2; return TOKEN_COLORMASK; }
B				{ yylval->intValue = 0x4; return TOKEN_COLORMASK; }
A				{ yylval->intValue = 0x8; return TOKEN_COLORMASK; }
RG				{ yylval->intValue = 0x3; return TOKEN_COLORMASK; }
RB				{ yylval->intValue = 0x5; return TOKEN_COLORMASK; }
RA				{ yylval->intValue = 0x9; return TOKEN_COLORMASK; }
GB				{ yylval->intValue = 0x6; return TOKEN_COLORMASK; }
GA				{ yylval->intValue = 0xA; return TOKEN_COLORMASK; }
BA				{ yylval->intValue = 0xC; return TOKEN_COLORMASK; }
RGB				{ yylval->intValue = 0x7; return TOKEN_COLORMASK; }
RGA				{ yylval->intValue = 0xB; return TOKEN_COLORMASK; }
RBA				{ yylval->intValue = 0xD; return TOKEN_COLORMASK; }
GBA				{ yylval->intValue = 0xE; return TOKEN_COLORMASK; }
RGBA			{ yylval->intValue = 0xF; return TOKEN_COLORMASK; }

	/* Preprocessor */
#include				{ BEGIN(INCLUDE); }

<INCLUDE>{WS}			{ /* Skip blank */ }
<INCLUDE>{STRING}		{
	int size = 0;
	char* includeBuffer = includePush(yyextra, yytext, yylineno, yycolumn, &size);
	if(!includeBuffer)
		yyterminate();

	YY_BUFFER_STATE currentBuffer = YY_CURRENT_BUFFER;
	YY_BUFFER_STATE newBuffer = yy_scan_buffer(includeBuffer, size, yyscanner);

	yy_switch_to_buffer(currentBuffer, yyscanner);
	yypush_buffer_state(newBuffer, yyscanner);

	yylineno = 0; 
	yycolumn = 0;

	BEGIN(INITIAL);
	}
<INCLUDE>.				{ return yytext[0]; }

<<EOF>>					{
	if(!yyextra->includeStack)
		yyterminate();

	yypop_buffer_state(yyscanner);
	includePop(yyextra);
}

#define							{ BEGIN(DEFINE_COND); }
<DEFINE_COND>{SPACE}			{ /* Skip blank */ }
<DEFINE_COND>{IDENTIFIER}		{ addDefine(yyextra, yytext); BEGIN(DEFINE_COND_EXPR); }
<DEFINE_COND>{ENDLINE}			{ BEGIN(INITIAL); }
<DEFINE_COND>.					{ return yytext[0]; }

<DEFINE_COND_EXPR>{DEFINE_EXPR}		{ addDefineExpr(yyextra, yytext); BEGIN(INITIAL); }
<DEFINE_COND_EXPR>{ENDLINE}			{ BEGIN(INITIAL); }

#undef							{ BEGIN(UNDEF_COND); }
<UNDEF_COND>{WS}				{ /* Skip blank */ }
<UNDEF_COND>{IDENTIFIER}		{ removeDefine(yyextra, yytext); BEGIN(INITIAL); }
<UNDEF_COND>.					{ return yytext[0]; }

#ifdef							{ BEGIN(CONDITIONAL_IF); }
<CONDITIONAL_IF>{WS}			{ /* Skip blank */ }
<CONDITIONAL_IF>{IDENTIFIER}	{ 
	int isEnabled = pushConditional(yyextra, hasDefine(yyextra, yytext));
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_IF>.				{ return yytext[0]; }

#ifndef							{ BEGIN(CONDITIONAL_IFN); }
<CONDITIONAL_IFN>{WS}			{ /* Skip blank */ }
<CONDITIONAL_IFN>{IDENTIFIER}	{ 
	int isEnabled = pushConditional(yyextra, !hasDefine(yyextra, yytext));
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_IFN>.				{ return yytext[0]; }

#else							{ BEGIN(CONDITIONAL_IGNORE); }
#elif							{ BEGIN(CONDITIONAL_IGNORE); }

#endif							{ popConditional(yyextra); }

<CONDITIONAL_IGNORE>{WS}		{ /* Skip */ }
<CONDITIONAL_IGNORE>#ifdef		{ pushConditional(yyextra, 0); }
<CONDITIONAL_IGNORE>#ifndef		{ pushConditional(yyextra, 0); }
<CONDITIONAL_IGNORE>#else		{ 
	if(switchConditional(yyextra))
		BEGIN(INITIAL);
}
<CONDITIONAL_IGNORE>#elif		{ BEGIN(CONDITIONAL_ELIF); }
<CONDITIONAL_IGNORE>#endif		{ 
	if(popConditional(yyextra))
		BEGIN(INITIAL);
}
<CONDITIONAL_IGNORE>.			{ /* Skip */ }

<CONDITIONAL_ELIF>{WS}			{ /* Skip blank */ }
<CONDITIONAL_ELIF>{IDENTIFIER}	{ 
	int isEnabled = setConditional(yyextra, hasDefine(yyextra, yytext));
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_ELIF>.				{ return yytext[0]; }

	/* Code block */
code			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_CODE; }

	/* Track when the code block begins, insert all code block characters into our own buffer, record a sequential index */
	/* of all code blocks in the text, and track bracket open/closed state so we know when we're done with the code block. */
	/* And finally output a sequential code block index to the parser (it shouldn't be aware of anything else in the block). */
<CODEBLOCK_HEADER>=		{ BEGIN(CODEBLOCK_EQUALS); return yytext[0]; }
<CODEBLOCK_HEADER>{WS}	{ /* Skip blank */ }
<CODEBLOCK_HEADER>.		{ return yytext[0]; }

<CODEBLOCK_EQUALS>\{	{ BEGIN(CODEBLOCK); beginCodeBlock(yyextra); yyextra->numOpenBrackets = 1; return yytext[0]; }
<CODEBLOCK_EQUALS>{WS}	{ /* Skip blank */ }
<CODEBLOCK_EQUALS>.		{ return yytext[0]; }

<CODEBLOCK>\{			{ yyextra->numOpenBrackets++; appendCodeBlock(yyextra, yytext, 1); }
<CODEBLOCK>\}			{ 
	yyextra->numOpenBrackets--; 

	if(yyextra->numOpenBrackets == 0)
	{
		BEGIN(CODEBLOCK_END);
		unput('0');
	}
	else
		appendCodeBlock(yyextra, yytext, 1);
}
<CODEBLOCK>.|{SINGLEWS}		{ appendCodeBlock(yyextra, yytext, 1); }

	/* Logic for manually inserting "Index = codeBlockIndex;". We insert arbitrary numbers which allows us to sequentially */
	/* output all the tokens we need. We use only single-character values so we don't override anything in the text buffer */
	/* (since the starting value was also a single character "{"). */
<CODEBLOCK_END>0	{ unput('1'); return TOKEN_INDEX; }
<CODEBLOCK_END>1	{ unput('2'); return '='; }
<CODEBLOCK_END>2	{ yylval->intValue = getCodeBlockIndex(yyextra); unput('3'); return TOKEN_INTEGER; }
<CODEBLOCK_END>3	{ unput('4'); return ';'; }
<CODEBLOCK_END>4	{ BEGIN(INITIAL); return '}'; }
<CODEBLOCK_END>.|{WS}	{ /* Never reached */ }

	/* Catch all rules */
{COMMENT}			{ }
{IDENTIFIER}		{ yylval->strValue = mmalloc_strdup(yyextra->memContext, yytext); return TOKEN_IDENTIFIER; }
.					{ return yytext[0]; }

%%