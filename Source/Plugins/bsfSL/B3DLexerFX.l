%{
#include "B3DParserFX.h"

#define YY_USER_ACTION yylloc->first_column = yycolumn + 1; yylloc->first_line = yylineno + 1; yycolumn += (int)yyleng; yylloc->filename = GetCurrentFilename(yyextra);
#define YY_USER_INIT yylineno = 0; yycolumn = 0;
%}

%option yylineno reentrant noyywrap nounistd never-interactive warn nodefault bison-bridge bison-locations
%option outfile="B3DLexerFX.c" header-file="B3DLexerFX.h"
%option extra-type="struct tagParseState *"

INTEGER			-?[0-9][0-9]*
INTEGER_16		0[xX][0-9a-fA-F]+
FLOAT			[-+]?[0-9]*\.?[0-9]+([eE][+-]?[0-9]+)?[fF]?
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
%x CODEBLOCK
%x CODEBLOCK_END
%x SUBSHADER_HEADER
%x SUBSHADER_BODY
%x SUBSHADER_END
%x DEFINE_COND
%x DEFINE_COND_EXPR
%x UNDEF_COND
%x CONDITIONAL_IF
%x CONDITIONAL_IFDEF
%x CONDITIONAL_IFNDEF
%x CONDITIONAL_ELIF
%x CONDITIONAL_IGNORE

%%

{WS}			{ /* Skip blank */ }
{INTEGER}       { yylval->IntValue = atoi(yytext); return TOKEN_INTEGER; }
{INTEGER_16}    { yylval->IntValue = (int)strtol(yytext, 0, 0); return TOKEN_INTEGER; }
{FLOAT}			{ yylval->FloatValue = (float)atof(yytext); return TOKEN_FLOAT; }
{STRING}		{ yylval->StrValue = MmallocStrdup(yyextra->MemContext, yytext); return TOKEN_STRING; }
true			{ yylval->IntValue = 1; return TOKEN_BOOLEAN; }
false			{ yylval->IntValue = 0; return TOKEN_BOOLEAN; }

	/* Shader keywords */
options			{ return TOKEN_OPTIONS; }
shader			{ return TOKEN_SHADER; }
mixin			{ return TOKEN_MIXIN; }

	/* Options keywords */
separable		{ return TOKEN_SEPARABLE; }
sort			{ return TOKEN_SORT; }
priority		{ return TOKEN_PRIORITY; }
transparent		{ return TOKEN_TRANSPARENT; }
forward			{ return TOKEN_FORWARD; }

	/* Shader keywords */
featureset		{ return TOKEN_FEATURESET; }
tags			{ return TOKEN_TAGS; }
pass			{ return TOKEN_PASS; }
variations		{ return TOKEN_VARIATIONS; }

	/* Pass keywords */
blend			{ return TOKEN_BLEND; }
raster			{ return TOKEN_RASTER; }
depth			{ return TOKEN_DEPTH; }
stencil			{ return TOKEN_STENCIL; }

	/* Variation keywords */
variation		{ return TOKEN_VARIATION; }

	/* Rasterizer state keywords */
fill			{ return TOKEN_FILLMODE; }
cull			{ return TOKEN_CULLMODE; }
scissor			{ return TOKEN_SCISSOR; }
multisample		{ return TOKEN_MULTISAMPLE; }
lineaa			{ return TOKEN_AALINE; }

	/* Depth state keywords */
read			{ return TOKEN_DEPTHREAD; }
write			{ return TOKEN_DEPTHWRITE; }
compare			{ return TOKEN_COMPAREFUNC; }
bias			{ return TOKEN_DEPTHBIAS; }
scaledbias		{ return TOKEN_SDEPTHBIAS; }
clip			{ return TOKEN_DEPTHCLIP; }

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
wire			{ yylval->IntValue = FMV_Wire; return TOKEN_FILLMODEVALUE; }
solid			{ yylval->IntValue = FMV_Solid; return TOKEN_FILLMODEVALUE; }

none			{ yylval->IntValue = CASV_None; return TOKEN_CULLANDQUEUEVALUE; }
cw				{ yylval->IntValue = CASV_CW; return TOKEN_CULLANDQUEUEVALUE; }
ccw				{ yylval->IntValue = CASV_CCW; return TOKEN_CULLANDQUEUEVALUE; }
fronttoback		{ yylval->IntValue = CASV_FrontToBack; return TOKEN_CULLANDQUEUEVALUE; }
backtofront		{ yylval->IntValue = CASV_BackToFront; return TOKEN_CULLANDQUEUEVALUE; }

never			{ yylval->IntValue = CFV_Fail; return TOKEN_COMPFUNCVALUE; }
always			{ yylval->IntValue = CFV_Pass; return TOKEN_COMPFUNCVALUE; }
lt				{ yylval->IntValue = CFV_LT; return TOKEN_COMPFUNCVALUE; }
lte				{ yylval->IntValue = CFV_LTE; return TOKEN_COMPFUNCVALUE; }
eq				{ yylval->IntValue = CFV_EQ; return TOKEN_COMPFUNCVALUE; }
neq				{ yylval->IntValue = CFV_NEQ; return TOKEN_COMPFUNCVALUE; }
gte				{ yylval->IntValue = CFV_GTE; return TOKEN_COMPFUNCVALUE; }
gt				{ yylval->IntValue = CFV_GT; return TOKEN_COMPFUNCVALUE; }

keep			{ yylval->IntValue = OV_Keep; return TOKEN_OPVALUE; }
zero			{ yylval->IntValue = OV_Zero; return TOKEN_OPVALUE; }
replace			{ yylval->IntValue = OV_Replace; return TOKEN_OPVALUE; }
inc				{ yylval->IntValue = OV_Incr; return TOKEN_OPVALUE; }
dec				{ yylval->IntValue = OV_Decr; return TOKEN_OPVALUE; }
incwrap			{ yylval->IntValue = OV_IncrWrap; return TOKEN_OPVALUE; }
decwrap			{ yylval->IntValue = OV_DecrWrap; return TOKEN_OPVALUE; }
inverse			{ yylval->IntValue = OV_Invert; return TOKEN_OPVALUE; }
one				{ yylval->IntValue = OV_One; return TOKEN_OPVALUE; }
dstRGB			{ yylval->IntValue = OV_DestColor; return TOKEN_OPVALUE; }
srcRGB			{ yylval->IntValue = OV_SrcColor; return TOKEN_OPVALUE; }
dstIRGB			{ yylval->IntValue = OV_InvDestColor; return TOKEN_OPVALUE; }
srcIRGB			{ yylval->IntValue = OV_InvSrcColor; return TOKEN_OPVALUE; }
dstA			{ yylval->IntValue = OV_DestAlpha; return TOKEN_OPVALUE; }
srcA			{ yylval->IntValue = OV_SrcAlpha; return TOKEN_OPVALUE; }
dstIA			{ yylval->IntValue = OV_InvDestAlpha; return TOKEN_OPVALUE; }
srcIA			{ yylval->IntValue = OV_InvSrcAlpha; return TOKEN_OPVALUE; }

add				{ yylval->IntValue = BOV_Add; return TOKEN_BLENDOPVALUE; }
sub				{ yylval->IntValue = BOV_Subtract; return TOKEN_BLENDOPVALUE; }
rsub			{ yylval->IntValue = BOV_RevSubtract; return TOKEN_BLENDOPVALUE; }
min				{ yylval->IntValue = BOV_Min; return TOKEN_BLENDOPVALUE; }
max				{ yylval->IntValue = BOV_Max; return TOKEN_BLENDOPVALUE; }

empty			{ yylval->IntValue = 0x0; return TOKEN_COLORMASK; }
R				{ yylval->IntValue = 0x1; return TOKEN_COLORMASK; }
G				{ yylval->IntValue = 0x2; return TOKEN_COLORMASK; }
B				{ yylval->IntValue = 0x4; return TOKEN_COLORMASK; }
A				{ yylval->IntValue = 0x8; return TOKEN_COLORMASK; }
RG				{ yylval->IntValue = 0x3; return TOKEN_COLORMASK; }
RB				{ yylval->IntValue = 0x5; return TOKEN_COLORMASK; }
RA				{ yylval->IntValue = 0x9; return TOKEN_COLORMASK; }
GB				{ yylval->IntValue = 0x6; return TOKEN_COLORMASK; }
GA				{ yylval->IntValue = 0xA; return TOKEN_COLORMASK; }
BA				{ yylval->IntValue = 0xC; return TOKEN_COLORMASK; }
RGB				{ yylval->IntValue = 0x7; return TOKEN_COLORMASK; }
RGA				{ yylval->IntValue = 0xB; return TOKEN_COLORMASK; }
RBA				{ yylval->IntValue = 0xD; return TOKEN_COLORMASK; }
GBA				{ yylval->IntValue = 0xE; return TOKEN_COLORMASK; }
RGBA			{ yylval->IntValue = 0xF; return TOKEN_COLORMASK; }

	/* Attribute keywords */
name			{ return TOKEN_NAME; }
show			{ return TOKEN_SHOW; }

	/* Preprocessor */
#include				{ BEGIN(INCLUDE); }

<INCLUDE>{WS}			{ /* Skip blank */ }
<INCLUDE>{STRING}		{
	int size = 0;
	char* includeBuffer = IncludePush(yyextra, yytext, yylineno, yycolumn, &size);
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
<INCLUDE>{COMMENT}		{ /* Skip comment */ }
<INCLUDE>.				{ return yytext[0]; }

<<EOF>>					{
	if(!yyextra->IncludeStack)
		yyterminate();

	yypop_buffer_state(yyscanner);
	IncludePop(yyextra);
}

#define							{ BEGIN(DEFINE_COND); }
<DEFINE_COND>{SPACE}			{ /* Skip blank */ }
<DEFINE_COND>{IDENTIFIER}		{ AddDefine(yyextra, yytext); BEGIN(DEFINE_COND_EXPR); }
<DEFINE_COND>{ENDLINE}			{ BEGIN(INITIAL); }
<DEFINE_COND>{COMMENT}			{ /* Skip comment */ }
<DEFINE_COND>.					{ return yytext[0]; }

<DEFINE_COND_EXPR>{DEFINE_EXPR}		{ AddDefineExpr(yyextra, yytext); BEGIN(INITIAL); }
<DEFINE_COND_EXPR>{ENDLINE}			{ BEGIN(INITIAL); }

#undef							{ BEGIN(UNDEF_COND); }
<UNDEF_COND>{WS}				{ /* Skip blank */ }
<UNDEF_COND>{IDENTIFIER}		{ RemoveDefine(yyextra, yytext); BEGIN(INITIAL); }
<UNDEF_COND>{COMMENT}			{ /* Skip comment */ }
<UNDEF_COND>.					{ return yytext[0]; }

#if								{ BEGIN(CONDITIONAL_IF); }
<CONDITIONAL_IF>{IDENTIFIER}	{ PushConditional(yyextra, yytext); }
<CONDITIONAL_IF>==				{ SetConditionalOp(yyextra, CO_Equals); }
<CONDITIONAL_IF>!=				{ SetConditionalOp(yyextra, CO_NotEquals); }
<CONDITIONAL_IF>>				{ SetConditionalOp(yyextra, CO_Greater); }
<CONDITIONAL_IF><				{ SetConditionalOp(yyextra, CO_Lesser); }
<CONDITIONAL_IF>>=				{ SetConditionalOp(yyextra, CO_GreaterEqual); }
<CONDITIONAL_IF><=				{ SetConditionalOp(yyextra, CO_LesserEqual); }
<CONDITIONAL_IF>{INTEGER}		{ SetConditionalVal(yyextra, yytext); }
<CONDITIONAL_IF>{ENDLINE}		{ 
	int isEnabled = EvalConditional(yyextra);
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_IF>{SPACE}			{ /* Skip blank */ }
<CONDITIONAL_IF>{COMMENT}		{ /* Skip comment */ }
<CONDITIONAL_IF>.				{ return yytext[0]; }

#ifdef							{ BEGIN(CONDITIONAL_IFDEF); }
<CONDITIONAL_IFDEF>{WS}			{ /* Skip blank */ }
<CONDITIONAL_IFDEF>{IDENTIFIER}	{ 
	int isEnabled = PushConditionalDef(yyextra, HasDefine(yyextra, yytext));
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_IFDEF>{COMMENT}		{ /* Skip comment */ }
<CONDITIONAL_IFDEF>.				{ return yytext[0]; }

#ifndef								{ BEGIN(CONDITIONAL_IFNDEF); }
<CONDITIONAL_IFNDEF>{WS}			{ /* Skip blank */ }
<CONDITIONAL_IFNDEF>{IDENTIFIER}	{ 
	int isEnabled = PushConditionalDef(yyextra, !HasDefine(yyextra, yytext));
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_IFNDEF>{COMMENT}		{ /* Skip comment */ }
<CONDITIONAL_IFNDEF>.				{ return yytext[0]; }

#else							{ 
	if(!SwitchConditional(yyextra))
		BEGIN(CONDITIONAL_IGNORE); 
}
#elif							{ BEGIN(CONDITIONAL_IGNORE); }

#endif							{ PopConditional(yyextra); }

<CONDITIONAL_IGNORE>{WS}		{ /* Skip */ }
<CONDITIONAL_IGNORE>#if			{ PushConditional(yyextra, 0); }
<CONDITIONAL_IGNORE>#ifdef		{ PushConditional(yyextra, 0); }
<CONDITIONAL_IGNORE>#ifndef		{ PushConditional(yyextra, 0); }
<CONDITIONAL_IGNORE>#else		{ 
	if(SwitchConditional(yyextra))
		BEGIN(INITIAL);
}
<CONDITIONAL_IGNORE>#elif		{ BEGIN(CONDITIONAL_ELIF); }
<CONDITIONAL_IGNORE>#endif		{ 
	if(PopConditional(yyextra))
		BEGIN(INITIAL);
}
<CONDITIONAL_IGNORE>.			{ /* Skip */ }

<CONDITIONAL_ELIF>{IDENTIFIER}	{ SetConditional(yyextra, yytext); }
<CONDITIONAL_ELIF>==			{ SetConditionalOp(yyextra, CO_Equals); }
<CONDITIONAL_ELIF>!=			{ SetConditionalOp(yyextra, CO_NotEquals); }
<CONDITIONAL_ELIF>>				{ SetConditionalOp(yyextra, CO_Greater); }
<CONDITIONAL_ELIF><				{ SetConditionalOp(yyextra, CO_Lesser); }
<CONDITIONAL_ELIF>>=			{ SetConditionalOp(yyextra, CO_GreaterEqual); }
<CONDITIONAL_ELIF><=			{ SetConditionalOp(yyextra, CO_LesserEqual); }
<CONDITIONAL_ELIF>{INTEGER}		{ SetConditionalVal(yyextra, yytext); }
<CONDITIONAL_ELIF>{ENDLINE}		{ 
	int isEnabled = EvalConditional(yyextra);
	if(!isEnabled)
		BEGIN(CONDITIONAL_IGNORE);
	else
		BEGIN(INITIAL);
}
<CONDITIONAL_ELIF>{SPACE}		{ /* Skip blank */ }
<CONDITIONAL_ELIF>{COMMENT}		{ /* Skip blank */ }
<CONDITIONAL_ELIF>.				{ return yytext[0]; }

	/* Code block */
code			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_CODE; }

	/* Track when the code block begins, insert all code block characters into our own buffer, record a sequential index */
	/* of all code blocks in the text, and track bracket open/closed state so we know when we're done with the code block. */
	/* And finally output a sequential code block index to the parser (it shouldn't be aware of anything else in the block). */
<CODEBLOCK_HEADER>\{		{ BEGIN(CODEBLOCK); BeginCodeBlock(yyextra, RCT_CodeBlock); yyextra->NumOpenBrackets = 1; return yytext[0]; }
<CODEBLOCK_HEADER>{WS}		{ /* Skip blank */ }
<CODEBLOCK_HEADER>{COMMENT} { /* Skip comment */ }
<CODEBLOCK_HEADER>.			{ return yytext[0]; }

<CODEBLOCK>\{			{ yyextra->NumOpenBrackets++; AppendCodeBlock(yyextra, RCT_CodeBlock, yytext, 1); }
<CODEBLOCK>\}			{ 
	yyextra->NumOpenBrackets--; 

	if(yyextra->NumOpenBrackets == 0)
	{
		BEGIN(CODEBLOCK_END);
		unput('0');
	}
	else
		AppendCodeBlock(yyextra, RCT_CodeBlock, yytext, 1);
}
<CODEBLOCK>.|{SINGLEWS}		{ AppendCodeBlock(yyextra, RCT_CodeBlock, yytext, 1); }

	/* Logic for manually inserting "Index = codeBlockIndex;". We insert arbitrary numbers which allows us to sequentially */
	/* output all the tokens we need. We use only single-character values so we don't override anything in the text buffer */
	/* (since the starting value was also a single character "{"). */
<CODEBLOCK_END>0	{ unput('1'); return TOKEN_INDEX; }
<CODEBLOCK_END>1	{ unput('2'); return '='; }
<CODEBLOCK_END>2	{ yylval->IntValue = GetCodeBlockIndex(yyextra, RCT_CodeBlock); unput('3'); return TOKEN_INTEGER; }
<CODEBLOCK_END>3	{ unput('4'); return ';'; }
<CODEBLOCK_END>4	{ BEGIN(INITIAL); return '}'; }
<CODEBLOCK_END>.|{WS}	{ /* Never reached */ }

	/* Subshader */
subshader				{ BEGIN(SUBSHADER_HEADER); return TOKEN_SUBSHADER; }

	/* Track when the subshader begins, insert all subshader characters into our own buffer, record a sequential index */
	/* of all subshaders in the text, and track bracket open/closed state so we know when we're done with the subshader. */
	/* And finally output a sequential subshader index to the parser (it shouldn't be aware of anything else in the block). */
<SUBSHADER_HEADER>\{				{ BEGIN(SUBSHADER_BODY); BeginCodeBlock(yyextra, RCT_SubShaderBlock); yyextra->NumOpenBrackets = 1; return yytext[0]; }
<SUBSHADER_HEADER>{WS}				{ /* Skip blank */ }
<SUBSHADER_HEADER>{COMMENT}			{ /* Skip comment */ }
<SUBSHADER_HEADER>{IDENTIFIER}		{ yylval->StrValue = MmallocStrdup(yyextra->MemContext, yytext); return TOKEN_IDENTIFIER; }
<SUBSHADER_HEADER>.					{ return yytext[0]; }

<SUBSHADER_BODY>\{			{ yyextra->NumOpenBrackets++; AppendCodeBlock(yyextra, RCT_SubShaderBlock, yytext, 1); }
<SUBSHADER_BODY>\}			{ 
	yyextra->NumOpenBrackets--; 

	if(yyextra->NumOpenBrackets == 0)
	{
		BEGIN(SUBSHADER_END);
		unput('0');
	}
	else
		AppendCodeBlock(yyextra, RCT_SubShaderBlock, yytext, 1);
}
<SUBSHADER_BODY>.|{SINGLEWS}	{ AppendCodeBlock(yyextra, RCT_SubShaderBlock, yytext, 1); }

	/* Logic for manually inserting "Index = subshaderIndex;". We insert arbitrary numbers which allows us to sequentially */
	/* output all the tokens we need. We use only single-character values so we don't override anything in the text buffer */
	/* (since the starting value was also a single character "{"). */
<SUBSHADER_END>0	{ unput('1'); return TOKEN_INDEX; }
<SUBSHADER_END>1	{ unput('2'); return '='; }
<SUBSHADER_END>2	{ yylval->IntValue = GetCodeBlockIndex(yyextra, RCT_SubShaderBlock); unput('3'); return TOKEN_INTEGER; }
<SUBSHADER_END>3	{ unput('4'); return ';'; }
<SUBSHADER_END>4	{ BEGIN(INITIAL); return '}'; }
<SUBSHADER_END>.|{WS}	{ /* Never reached */ }

	/* Catch all rules */
{COMMENT}			{ }
{IDENTIFIER}		{ yylval->StrValue = MmallocStrdup(yyextra->MemContext, yytext); return TOKEN_IDENTIFIER; }
.					{ return yytext[0]; }

%%
