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

	/* Value types */

int				{ yylval->intValue = PT_Int; return TOKEN_INTTYPE; } 
int2			{ yylval->intValue = PT_Int2; return TOKEN_INT2TYPE; } 
int3			{ yylval->intValue = PT_Int3; return TOKEN_INT3TYPE; } 
int4			{ yylval->intValue = PT_Int4; return TOKEN_INT4TYPE; }

float			{ yylval->intValue = PT_Float; return TOKEN_FLOATTYPE; } 
float2			{ yylval->intValue = PT_Float2; return TOKEN_FLOAT2TYPE; } 
float3			{ yylval->intValue = PT_Float3; return TOKEN_FLOAT3TYPE; } 
float4			{ yylval->intValue = PT_Float4; return TOKEN_FLOAT4TYPE; }
color			{ yylval->intValue = PT_Color; return TOKEN_COLORTYPE; }

mat2x2			{ yylval->intValue = PT_Mat2x2; return TOKEN_MAT2x2TYPE; } 
mat2x3			{ yylval->intValue = PT_Mat2x3; return TOKEN_MAT2x3TYPE; } 
mat2x4			{ yylval->intValue = PT_Mat2x4; return TOKEN_MAT2x4TYPE; }

mat3x2			{ yylval->intValue = PT_Mat3x2; return TOKEN_MAT3x2TYPE; } 
mat3x3			{ yylval->intValue = PT_Mat3x3; return TOKEN_MAT3x3TYPE; } 
mat3x4			{ yylval->intValue = PT_Mat3x4; return TOKEN_MAT3x4TYPE; }

mat4x2			{ yylval->intValue = PT_Mat4x2; return TOKEN_MAT4x2TYPE; } 
mat4x3			{ yylval->intValue = PT_Mat4x3; return TOKEN_MAT4x3TYPE; } 
mat4x4			{ yylval->intValue = PT_Mat4x4; return TOKEN_MAT4x4TYPE; }

Sampler1D		{ yylval->intValue = PT_Sampler1D; return TOKEN_SAMPLER1D; } 
Sampler2D		{ yylval->intValue = PT_Sampler2D; return TOKEN_SAMPLER2D; } 
Sampler3D		{ yylval->intValue = PT_Sampler3D; return TOKEN_SAMPLER3D; } 
SamplerCUBE		{ yylval->intValue = PT_SamplerCUBE; return TOKEN_SAMPLERCUBE; } 
Sampler2DMS		{ yylval->intValue = PT_Sampler2DMS; return TOKEN_SAMPLER2DMS; }

Texture1D		{ yylval->intValue = PT_Texture1D; return TOKEN_TEXTURE1D; } 
Texture2D		{ yylval->intValue = PT_Texture2D; return TOKEN_TEXTURE2D; } 
Texture3D		{ yylval->intValue = PT_Texture3D; return TOKEN_TEXTURE3D; } 
TextureCUBE		{ yylval->intValue = PT_TextureCUBE; return TOKEN_TEXTURECUBE; } 
Texture2DMS		{ yylval->intValue = PT_Texture2DMS; return TOKEN_TEXTURE2DMS; }

RWTexture1D		{ yylval->intValue = PT_RWTexture1D; return TOKEN_RWTEXTURE1D; } 
RWTexture2D		{ yylval->intValue = PT_RWTexture2D; return TOKEN_RWTEXTURE2D; } 
RWTexture3D		{ yylval->intValue = PT_RWTexture3D; return TOKEN_RWTEXTURE3D; } 
RWTexture2DMS	{ yylval->intValue = PT_RWTexture2DMS; return TOKEN_RWTEXTURE2DMS; }

ByteBuffer		{ yylval->intValue = PT_ByteBuffer; return TOKEN_BYTEBUFFER; } 
StructBuffer	{ yylval->intValue = PT_StructBuffer; return TOKEN_STRUCTBUFFER; } 

RWTypedBuffer	{ yylval->intValue = PT_TypedBufferRW; return TOKEN_RWTYPEDBUFFER; } 
RWByteBuffer	{ yylval->intValue = PT_ByteBufferRW; return TOKEN_RWBYTEBUFFER; }
RWStructBuffer	{ yylval->intValue = PT_StructBufferRW; return TOKEN_RWSTRUCTBUFFER; } 
AppendBuffer	{ yylval->intValue = PT_AppendBuffer; return TOKEN_RWAPPENDBUFFER; } 
ConsumeBuffer	{ yylval->intValue = PT_ConsumeBuffer; return TOKEN_RWCONSUMEBUFFER; }

Block			{ return TOKEN_PARAMSBLOCK; }

	/* Shader keywords */
Separable		{ return TOKEN_SEPARABLE; }
Sort			{ return TOKEN_SORT; }
Priority		{ return TOKEN_PRIORITY; }
Transparent		{ return TOKEN_TRANSPARENT; }
Technique		{ return TOKEN_TECHNIQUE; }
Parameters		{ return TOKEN_PARAMETERS; }
Blocks			{ return TOKEN_BLOCKS; }

	/* Technique keywords */
Renderer		{ return TOKEN_RENDERER; }
Language		{ return TOKEN_LANGUAGE; }
Tags			{ return TOKEN_TAGS; }
Pass			{ return TOKEN_PASS; }

	/* Pass keywords */
StencilRef		{ return TOKEN_STENCILREF; }

	/* Rasterizer state keywords */
Fill			{ return TOKEN_FILLMODE; }
Cull			{ return TOKEN_CULLMODE; }
DepthBias		{ return TOKEN_DEPTHBIAS; }
ScaledDepthBias	{ return TOKEN_SDEPTHBIAS; }
DepthClip		{ return TOKEN_DEPTHCLIP; }
Scissor			{ return TOKEN_SCISSOR; }
Multisample		{ return TOKEN_MULTISAMPLE; }
AALine			{ return TOKEN_AALINE; }

	/* Depth-stencil state keywords */
DepthRead			{ return TOKEN_DEPTHREAD; }
DepthWrite			{ return TOKEN_DEPTHWRITE; }
CompareFunc			{ return TOKEN_COMPAREFUNC; }
Stencil				{ return TOKEN_STENCIL; }
StencilReadMask		{ return TOKEN_STENCILREADMASK; }
StencilWriteMask	{ return TOKEN_STENCILWRITEMASK; }
StencilOpFront		{ return TOKEN_STENCILOPFRONT; }
StencilOpBack		{ return TOKEN_STENCILOPBACK; }
Fail				{ return TOKEN_FAIL; }
ZFail				{ return TOKEN_ZFAIL; }

	/* Blend state keywords */
AlphaToCoverage		{ return TOKEN_ALPHATOCOVERAGE; }
IndependantBlend	{ return TOKEN_INDEPENDANTBLEND; }
Target				{ return TOKEN_TARGET; }
Index				{ return TOKEN_INDEX; }
Blend				{ return TOKEN_BLEND; }
Color				{ return TOKEN_COLOR; }
Alpha				{ return TOKEN_ALPHA; }
WriteMask			{ return TOKEN_WRITEMASK; }
Source				{ return TOKEN_SOURCE; }
Dest				{ return TOKEN_DEST; }
Op					{ return TOKEN_OP; }

	/* Sampler state keywords */
AddressMode			{ return TOKEN_ADDRMODE; }
MinFilter			{ return TOKEN_MINFILTER; }
MagFilter			{ return TOKEN_MAGFILTER; }
MipFilter			{ return TOKEN_MIPFILTER; }
MaxAniso			{ return TOKEN_MAXANISO; }
MipmapBias			{ return TOKEN_MIPBIAS; }
MipMin				{ return TOKEN_MIPMIN; }
MipMax				{ return TOKEN_MIPMAX; }
BorderColor			{ return TOKEN_BORDERCOLOR; }
U					{ return TOKEN_U; }
V					{ return TOKEN_V; }
W					{ return TOKEN_W; }

	/* Qualifiers */
auto				{ return TOKEN_AUTO; }
alias				{ return TOKEN_ALIAS; }
shared				{ return TOKEN_SHARED; }
usage				{ return TOKEN_USAGE; }
base				{ return TOKEN_BASE; }
inherits			{ return TOKEN_INHERITS; }

	/* State values */
WIRE			{ yylval->intValue = FMV_Wire; return TOKEN_FILLMODEVALUE; }
SOLID			{ yylval->intValue = FMV_Solid; return TOKEN_FILLMODEVALUE; }

NOCULL			{ yylval->intValue = CMV_None; return TOKEN_CULLMODEVALUE; }
CW				{ yylval->intValue = CMV_CW; return TOKEN_CULLMODEVALUE; }
CCW				{ yylval->intValue = CMV_CCW; return TOKEN_CULLMODEVALUE; }

FAIL			{ yylval->intValue = CFV_Fail; return TOKEN_COMPFUNCVALUE; }
PASS			{ yylval->intValue = CFV_Pass; return TOKEN_COMPFUNCVALUE; }
LT				{ yylval->intValue = CFV_LT; return TOKEN_COMPFUNCVALUE; }
LTE				{ yylval->intValue = CFV_LTE; return TOKEN_COMPFUNCVALUE; }
EQ				{ yylval->intValue = CFV_EQ; return TOKEN_COMPFUNCVALUE; }
NEQ				{ yylval->intValue = CFV_NEQ; return TOKEN_COMPFUNCVALUE; }
GTE				{ yylval->intValue = CFV_GTE; return TOKEN_COMPFUNCVALUE; }
GT				{ yylval->intValue = CFV_GT; return TOKEN_COMPFUNCVALUE; }

KEEP				{ yylval->intValue = OV_Keep; return TOKEN_OPVALUE; }
ZERO				{ yylval->intValue = OV_Zero; return TOKEN_OPVALUE; }
REPLACE				{ yylval->intValue = OV_Replace; return TOKEN_OPVALUE; }
INC					{ yylval->intValue = OV_Incr; return TOKEN_OPVALUE; }
DEC					{ yylval->intValue = OV_Decr; return TOKEN_OPVALUE; }
INCWRAP				{ yylval->intValue = OV_IncrWrap; return TOKEN_OPVALUE; }
DECWRAP				{ yylval->intValue = OV_DecrWrap; return TOKEN_OPVALUE; }
INV					{ yylval->intValue = OV_Invert; return TOKEN_OPVALUE; }
ONE					{ yylval->intValue = OV_One; return TOKEN_OPVALUE; }
DSTRGB				{ yylval->intValue = OV_DestColor; return TOKEN_OPVALUE; }
SRCRGB				{ yylval->intValue = OV_SrcColor; return TOKEN_OPVALUE; }
DSTIRGB				{ yylval->intValue = OV_InvDestColor; return TOKEN_OPVALUE; }
SRCIRGB				{ yylval->intValue = OV_InvSrcColor; return TOKEN_OPVALUE; }
DSTA				{ yylval->intValue = OV_DestAlpha; return TOKEN_OPVALUE; }
SRCA				{ yylval->intValue = OV_SrcAlpha; return TOKEN_OPVALUE; }
DSTIA				{ yylval->intValue = OV_InvDestAlpha; return TOKEN_OPVALUE; }
SRCIA				{ yylval->intValue = OV_InvSrcAlpha; return TOKEN_OPVALUE; }

ADD					{ yylval->intValue = BOV_Add; return TOKEN_BLENDOPVALUE; }
SUB					{ yylval->intValue = BOV_Subtract; return TOKEN_BLENDOPVALUE; }
RSUB				{ yylval->intValue = BOV_RevSubtract; return TOKEN_BLENDOPVALUE; }
MIN					{ yylval->intValue = BOV_Min; return TOKEN_BLENDOPVALUE; }
MAX					{ yylval->intValue = BOV_Max; return TOKEN_BLENDOPVALUE; }

NOCOLOR			{ yylval->intValue = 0x0; return TOKEN_COLORMASK; }
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

WRAP			{ yylval->intValue = AMV_Wrap; return TOKEN_ADDRMODEVALUE; }
MIRROR			{ yylval->intValue = AMV_Mirror; return TOKEN_ADDRMODEVALUE; }
CLAMP			{ yylval->intValue = AMV_Clamp; return TOKEN_ADDRMODEVALUE; }
BORDER			{ yylval->intValue = AMV_Border; return TOKEN_ADDRMODEVALUE; }

NOFILTER		{ yylval->intValue = FV_None; return TOKEN_FILTERVALUE; }
POINT			{ yylval->intValue = FV_Point; return TOKEN_FILTERVALUE; }
LINEAR			{ yylval->intValue = FV_Linear; return TOKEN_FILTERVALUE; }
ANISO			{ yylval->intValue = FV_Anisotropic; return TOKEN_FILTERVALUE; }
POINTC			{ yylval->intValue = FV_PointCmp; return TOKEN_FILTERVALUE; }
LINEARC			{ yylval->intValue = FV_LinearCmp; return TOKEN_FILTERVALUE; }
ANISOC			{ yylval->intValue = FV_AnisotropicCmp; return TOKEN_FILTERVALUE; }

STATIC			{ yylval->intValue = BUV_Static; return TOKEN_BUFFERUSAGE; }
DYNAMIC			{ yylval->intValue = BUV_Dynamic; return TOKEN_BUFFERUSAGE; }

FRONTTOBACK		{ yylval->intValue = QST_FrontToBack; return TOKEN_QUEUETYPE; }
BACKTOFRONT		{ yylval->intValue = QST_BackToFront; return TOKEN_QUEUETYPE; }
NOSORT			{ yylval->intValue = QST_None; return TOKEN_QUEUETYPE; }

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

	/* Code blocks */
Vertex			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_VERTEX; }
Fragment		{ BEGIN(CODEBLOCK_HEADER); return TOKEN_FRAGMENT; }
Geometry		{ BEGIN(CODEBLOCK_HEADER); return TOKEN_GEOMETRY; }
Hull			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_HULL; }
Domain			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_DOMAIN; }
Compute			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_COMPUTE; }
Common			{ BEGIN(CODEBLOCK_HEADER); return TOKEN_COMMON; }

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