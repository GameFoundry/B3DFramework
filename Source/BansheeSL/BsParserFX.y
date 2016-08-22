%{
#include "BsParserFX.h"
#include "BsLexerFX.h"
#define inline

void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg);
%}

%code requires{
#include "BsMMAlloc.h"
#include "BsASTFX.h"
#include "BsIncludeHandler.h"

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

#define ADD_PARAMETER(OUTPUT, TYPE, NAME)															\
			OUTPUT = nodeCreate(parse_state->memContext, NT_Parameter);								\
			nodePush(parse_state, OUTPUT);															\
																									\
			NodeOption paramType;																	\
			paramType.type = OT_ParamType;															\
			paramType.value.intValue = TYPE;														\
																									\
			NodeOption paramName;																	\
			paramName.type = OT_Identifier;															\
			paramName.value.strValue = NAME;														\
																									\
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &paramType);		\
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &paramName);		\

}

%output  "BsParserFX.c"
%defines "BsParserFX.h"

%define api.pure
%locations
%lex-param { yyscan_t scanner }
%parse-param { ParseState* parse_state }
%parse-param { yyscan_t scanner }
%glr-parser

%union {
	int intValue;
	float floatValue;
	float matrixValue[16];
	int intVectorValue[4];
	const char* strValue;
	ASTFXNode* nodePtr;
	NodeOption nodeOption;
}

	/* Value types */
%token <intValue>	TOKEN_INTEGER
%token <floatValue> TOKEN_FLOAT
%token <intValue>	TOKEN_BOOLEAN
%token <strValue>	TOKEN_STRING
%token <strValue>	TOKEN_IDENTIFIER

%token <intValue>	TOKEN_FILLMODEVALUE
%token <intValue>	TOKEN_CULLMODEVALUE
%token <intValue>	TOKEN_COMPFUNCVALUE
%token <intValue>	TOKEN_OPVALUE
%token <intValue>	TOKEN_COLORMASK
%token <intValue>	TOKEN_ADDRMODEVALUE
%token <intValue>	TOKEN_FILTERVALUE
%token <intValue>	TOKEN_BLENDOPVALUE
%token <intValue>	TOKEN_BUFFERUSAGE
%token <intValue>	TOKEN_QUEUETYPE

%token <intValue> TOKEN_FLOATTYPE 
%token <intValue> TOKEN_FLOAT2TYPE 
%token <intValue> TOKEN_FLOAT3TYPE 
%token <intValue> TOKEN_FLOAT4TYPE
%token <intValue> TOKEN_INTTYPE 
%token <intValue> TOKEN_INT2TYPE 
%token <intValue> TOKEN_INT3TYPE 
%token <intValue> TOKEN_INT4TYPE
%token <intValue> TOKEN_COLORTYPE

%token <intValue> TOKEN_MAT2x2TYPE 
%token <intValue> TOKEN_MAT2x3TYPE 
%token <intValue> TOKEN_MAT2x4TYPE

%token <intValue> TOKEN_MAT3x2TYPE 
%token <intValue> TOKEN_MAT3x3TYPE 
%token <intValue> TOKEN_MAT3x4TYPE

%token <intValue> TOKEN_MAT4x2TYPE 
%token <intValue> TOKEN_MAT4x3TYPE 
%token <intValue> TOKEN_MAT4x4TYPE

%token <intValue> TOKEN_SAMPLER1D 
%token <intValue> TOKEN_SAMPLER2D 
%token <intValue> TOKEN_SAMPLER3D 
%token <intValue> TOKEN_SAMPLERCUBE 
%token <intValue> TOKEN_SAMPLER2DMS

%token <intValue> TOKEN_TEXTURE1D 
%token <intValue> TOKEN_TEXTURE2D 
%token <intValue> TOKEN_TEXTURE3D 
%token <intValue> TOKEN_TEXTURECUBE 
%token <intValue> TOKEN_TEXTURE2DMS

%token <intValue> TOKEN_RWTEXTURE1D 
%token <intValue> TOKEN_RWTEXTURE2D 
%token <intValue> TOKEN_RWTEXTURE3D 
%token <intValue> TOKEN_RWTEXTURE2DMS

%token <intValue> TOKEN_BYTEBUFFER 
%token <intValue> TOKEN_STRUCTBUFFER 
%token <intValue> TOKEN_RWTYPEDBUFFER 
%token <intValue> TOKEN_RWBYTEBUFFER
%token <intValue> TOKEN_RWSTRUCTBUFFER 
%token <intValue> TOKEN_RWAPPENDBUFFER 
%token <intValue> TOKEN_RWCONSUMEBUFFER

%token TOKEN_PARAMSBLOCK

	/* Qualifiers */
%token TOKEN_AUTO TOKEN_ALIAS TOKEN_SHARED TOKEN_USAGE

	/* Shader keywords */
%token TOKEN_SEPARABLE TOKEN_SORT TOKEN_PRIORITY TOKEN_TRANSPARENT
%token TOKEN_PARAMETERS TOKEN_BLOCKS TOKEN_TECHNIQUE

	/* Technique keywords */
%token	TOKEN_RENDERER TOKEN_LANGUAGE TOKEN_PASS TOKEN_TAGS

	/* Pass keywords */
%token	TOKEN_VERTEX TOKEN_FRAGMENT TOKEN_GEOMETRY TOKEN_HULL TOKEN_DOMAIN TOKEN_COMPUTE TOKEN_COMMON
%token	TOKEN_STENCILREF

%token	TOKEN_FILLMODE TOKEN_CULLMODE TOKEN_DEPTHBIAS TOKEN_SDEPTHBIAS
%token	TOKEN_DEPTHCLIP TOKEN_SCISSOR TOKEN_MULTISAMPLE TOKEN_AALINE

%token	TOKEN_DEPTHREAD TOKEN_DEPTHWRITE TOKEN_COMPAREFUNC TOKEN_STENCIL
%token	TOKEN_STENCILREADMASK TOKEN_STENCILWRITEMASK TOKEN_STENCILOPFRONT TOKEN_STENCILOPBACK
%token	TOKEN_FAIL TOKEN_ZFAIL

%token	TOKEN_ALPHATOCOVERAGE TOKEN_INDEPENDANTBLEND TOKEN_TARGET TOKEN_INDEX
%token	TOKEN_BLEND TOKEN_COLOR TOKEN_ALPHA TOKEN_WRITEMASK
%token	TOKEN_SOURCE TOKEN_DEST TOKEN_OP

	/* Sampler state keywords */
%token	TOKEN_ADDRMODE TOKEN_MINFILTER TOKEN_MAGFILTER TOKEN_MIPFILTER
%token	TOKEN_MAXANISO TOKEN_MIPBIAS TOKEN_MIPMIN TOKEN_MIPMAX
%token	TOKEN_BORDERCOLOR TOKEN_U TOKEN_V TOKEN_W

%type <nodePtr>		shader;
%type <nodeOption>	shader_statement;
%type <nodeOption>	shader_option;

%type <nodePtr>		technique;
%type <nodePtr>		technique_header;
%type <nodeOption>	technique_statement;
%type <nodeOption>	technique_option;
%type <nodePtr>		tags;
%type <nodePtr>		tags_header;

%type <nodePtr>		pass;
%type <nodePtr>		pass_header;
%type <nodeOption>	pass_statement;
%type <nodeOption>	pass_option;

%type <nodePtr>		code;
%type <nodePtr>		code_header;

%type <nodePtr>		stencil_op_front_header;
%type <nodePtr>		stencil_op_back_header;
%type <nodeOption>	stencil_op_option;

%type <nodePtr>		target;
%type <nodePtr>		target_header;
%type <nodeOption>	target_statement;
%type <nodeOption>	target_option;

%type <nodePtr>		blend_color_header;
%type <nodePtr>		blend_alpha_header;
%type <nodeOption>	blenddef_option;

%type <nodeOption>	sampler_state_option;

%type <nodePtr>		addr_mode;
%type <nodePtr>		addr_mode_header;
%type <nodeOption>	addr_mode_option;

%type <nodePtr> parameters
%type <nodePtr> parameters_header
%type <nodeOption> parameter

%type <nodePtr> block_header
%type <nodeOption> block
%type <nodePtr> blocks_header
%type <nodePtr> blocks

%type <nodeOption> qualifier

%type <matrixValue> float2;
%type <matrixValue> float3;
%type <matrixValue> float4;
%type <intVectorValue> int2;
%type <intVectorValue> int3;
%type <intVectorValue> int4;
%type <matrixValue> mat6;
%type <matrixValue> mat8;
%type <matrixValue> mat9;
%type <matrixValue> mat12;
%type <matrixValue> mat16;

%type <nodePtr> param_header_float
%type <nodePtr> param_header_float2
%type <nodePtr> param_header_float3
%type <nodePtr> param_header_float4
%type <nodePtr> param_header_int
%type <nodePtr> param_header_int2
%type <nodePtr> param_header_int3
%type <nodePtr> param_header_int4
%type <nodePtr> param_header_color
%type <nodePtr> param_header_mat2x2
%type <nodePtr> param_header_mat2x3
%type <nodePtr> param_header_mat2x4
%type <nodePtr> param_header_mat3x2
%type <nodePtr> param_header_mat3x3
%type <nodePtr> param_header_mat3x4
%type <nodePtr> param_header_mat4x2
%type <nodePtr> param_header_mat4x3
%type <nodePtr> param_header_mat4x4
%type <nodePtr> param_header_sampler
%type <nodePtr> param_header_texture
%type <nodePtr> param_header_buffer
%type <nodePtr> param_header_qualified_sampler

%type <nodeOption> param_body_float
%type <nodeOption> param_body_float2
%type <nodeOption> param_body_float3
%type <nodeOption> param_body_float4
%type <nodeOption> param_body_int
%type <nodeOption> param_body_int2
%type <nodeOption> param_body_int3
%type <nodeOption> param_body_int4
%type <nodeOption> param_body_mat6
%type <nodeOption> param_body_mat8
%type <nodeOption> param_body_mat9
%type <nodeOption> param_body_mat12
%type <nodeOption> param_body_mat16
%type <nodeOption> param_body_sampler
%type <nodeOption> param_body_tex

%%

	/* Shader */

shader
	: /* empty */				{ }
	| shader_statement shader	{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

shader_statement
	: shader_option
	| technique			{ $$.type = OT_Technique; $$.value.nodePtr = $1; }
	| parameters		{ $$.type = OT_Parameters; $$.value.nodePtr = $1; }
	| blocks			{ $$.type = OT_Blocks; $$.value.nodePtr = $1; }
	;

shader_option
	: TOKEN_SEPARABLE '=' TOKEN_BOOLEAN ';'		{ $$.type = OT_Separable; $$.value.intValue = $3; }
	| TOKEN_SORT '=' TOKEN_QUEUETYPE ';'		{ $$.type = OT_Sort; $$.value.intValue = $3; }
	| TOKEN_PRIORITY '=' TOKEN_INTEGER ';'		{ $$.type = OT_Priority; $$.value.intValue = $3; }
	| TOKEN_TRANSPARENT '=' TOKEN_BOOLEAN ';'	{ $$.type = OT_Transparent; $$.value.intValue = $3; }
	;

	/* Technique */

technique
	: technique_header '{' technique_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

technique_header
	: TOKEN_TECHNIQUE '=' 
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Technique); 
			nodePush(parse_state, $$);
		}
	;

technique_body
	: /* empty */
	| technique_statement technique_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

technique_statement
	: technique_option
	| pass				{ $$.type = OT_Pass; $$.value.nodePtr = $1; }
	| pass_option
	| code				{ $$.type = OT_Code; $$.value.nodePtr = $1; }
	;

technique_option
	: TOKEN_RENDERER '=' TOKEN_STRING ';'	{ $$.type = OT_Renderer; $$.value.strValue = $3; }
	| TOKEN_LANGUAGE '=' TOKEN_STRING ';'	{ $$.type = OT_Language; $$.value.strValue = $3; }
	| tags									{ $$.type = OT_Tags; $$.value.nodePtr = $1; }
	;
	
tags
	: tags_header '{' tags_body '}' ';'	{ nodePop(parse_state); $$ = $1; }
	;
	
tags_header
	: TOKEN_TAGS '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Tags); 
			nodePush(parse_state, $$);
		}
	;
	
tags_body
	: /* empty */
	| TOKEN_STRING
		{ 
			NodeOption entry; entry.type = OT_TagValue; entry.value.strValue = $1;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}		
	| TOKEN_STRING ',' tags_body		
		{ 
			NodeOption entry; entry.type = OT_TagValue; entry.value.strValue = $1;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}	
	;

	/* Pass */

pass
	: pass_header '{' pass_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

pass_header
	: TOKEN_PASS '=' 
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Pass); 
			nodePush(parse_state, $$);
		}
	;

pass_body
	: /* empty */
	| pass_statement pass_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

pass_statement
	: pass_option
	| code				{ $$.type = OT_Code; $$.value.nodePtr = $1; }
	;

pass_option
	: TOKEN_INDEX '=' TOKEN_INTEGER ';'							{ $$.type = OT_Index; $$.value.intValue = $3; }
	| TOKEN_FILLMODE '=' TOKEN_FILLMODEVALUE ';'				{ $$.type = OT_FillMode; $$.value.intValue = $3; }
	| TOKEN_CULLMODE '=' TOKEN_CULLMODEVALUE ';'				{ $$.type = OT_CullMode; $$.value.intValue = $3; }
	| TOKEN_DEPTHBIAS '=' TOKEN_FLOAT ';'						{ $$.type = OT_DepthBias; $$.value.floatValue = $3; }
	| TOKEN_SDEPTHBIAS '=' TOKEN_FLOAT ';'						{ $$.type = OT_SDepthBias; $$.value.floatValue = $3; }
	| TOKEN_DEPTHCLIP '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_DepthClip; $$.value.intValue = $3; }
	| TOKEN_SCISSOR '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_Scissor; $$.value.intValue = $3; }
	| TOKEN_MULTISAMPLE '=' TOKEN_BOOLEAN ';'					{ $$.type = OT_Multisample; $$.value.intValue = $3; }
	| TOKEN_AALINE '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_AALine; $$.value.intValue = $3; }
	| TOKEN_DEPTHREAD '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_DepthRead; $$.value.intValue = $3; }
	| TOKEN_DEPTHWRITE '=' TOKEN_BOOLEAN ';'					{ $$.type = OT_DepthWrite; $$.value.intValue = $3; }
	| TOKEN_COMPAREFUNC '=' TOKEN_COMPFUNCVALUE ';'				{ $$.type = OT_CompareFunc; $$.value.intValue = $3; }
	| TOKEN_STENCIL '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_Stencil; $$.value.intValue = $3; }
	| TOKEN_STENCILREADMASK '=' TOKEN_INTEGER ';'				{ $$.type = OT_StencilReadMask; $$.value.intValue = $3; }
	| TOKEN_STENCILWRITEMASK '=' TOKEN_INTEGER ';'				{ $$.type = OT_StencilWriteMask; $$.value.intValue = $3; }
	| stencil_op_front_header '{' stencil_op_body '}' ';'		{ nodePop(parse_state); $$.type = OT_StencilOpFront; $$.value.nodePtr = $1; }
	| stencil_op_back_header '{' stencil_op_body '}' ';'		{ nodePop(parse_state); $$.type = OT_StencilOpBack; $$.value.nodePtr = $1; }
	| stencil_op_front_header '{' stencil_op_body_init '}' ';'	{ nodePop(parse_state); $$.type = OT_StencilOpFront; $$.value.nodePtr = $1; }
	| stencil_op_back_header '{' stencil_op_body_init '}' ';'	{ nodePop(parse_state); $$.type = OT_StencilOpBack; $$.value.nodePtr = $1; }
	| TOKEN_ALPHATOCOVERAGE '=' TOKEN_BOOLEAN ';'				{ $$.type = OT_AlphaToCoverage; $$.value.intValue = $3; }
	| TOKEN_INDEPENDANTBLEND '=' TOKEN_BOOLEAN ';'				{ $$.type = OT_IndependantBlend; $$.value.intValue = $3; }
	| target													{ $$.type = OT_Target; $$.value.nodePtr = $1; }
	| TOKEN_STENCILREF '=' TOKEN_INTEGER ';'					{ $$.type = OT_StencilRef; $$.value.intValue = $3; }
	;

	/* Code blocks */

code
	: code_header '{' TOKEN_INDEX '=' TOKEN_INTEGER ';' '}' ';'
	{
		NodeOption index;
		index.type = OT_Index; 
		index.value.intValue = $5;

		nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &index);

		nodePop(parse_state); 
		$$ = $1;
	}
	;

code_header
	: TOKEN_VERTEX '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeVertex); 
			nodePush(parse_state, $$);
		}
	| TOKEN_FRAGMENT '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeFragment); 
			nodePush(parse_state, $$);
		}
	| TOKEN_GEOMETRY '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeGeometry); 
			nodePush(parse_state, $$);
		}
	| TOKEN_HULL '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeHull); 
			nodePush(parse_state, $$);
		}
	| TOKEN_DOMAIN '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeDomain); 
			nodePush(parse_state, $$);
		}
	| TOKEN_COMPUTE '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeCompute); 
			nodePush(parse_state, $$);
		}
	| TOKEN_COMMON '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_CodeCommon); 
			nodePush(parse_state, $$);
		}
	;

	/* Stencil op */

stencil_op_front_header
	: TOKEN_STENCILOPFRONT '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_StencilOp); 
			nodePush(parse_state, $$);
		}
	;

stencil_op_back_header
	: TOKEN_STENCILOPBACK '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_StencilOp); 
			nodePush(parse_state, $$);
		}
	;

stencil_op_body_init
	: TOKEN_OPVALUE ',' TOKEN_OPVALUE ',' TOKEN_OPVALUE ',' TOKEN_COMPFUNCVALUE
		{
			NodeOption fail; fail.type = OT_Fail; fail.value.intValue = $1;
			NodeOption zfail; zfail.type = OT_ZFail; zfail.value.intValue = $3;
			NodeOption pass; pass.type = OT_PassOp; pass.value.intValue = $5;
			NodeOption cmp; cmp.type = OT_CompareFunc; cmp.value.intValue = $7;

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &fail);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &zfail);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &pass);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &cmp);
		}
	;

stencil_op_body
	: /* empty */
	| stencil_op_option stencil_op_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

stencil_op_option
	: TOKEN_FAIL '=' TOKEN_OPVALUE ';'					{ $$.type = OT_Fail; $$.value.intValue = $3; }
	| TOKEN_ZFAIL '=' TOKEN_OPVALUE ';'					{ $$.type = OT_ZFail; $$.value.intValue = $3; }
	| TOKEN_PASS '=' TOKEN_OPVALUE ';'					{ $$.type = OT_PassOp; $$.value.intValue = $3; }
	| TOKEN_COMPAREFUNC '=' TOKEN_COMPFUNCVALUE ';'		{ $$.type = OT_CompareFunc; $$.value.intValue = $3; }
	; 

	/* Target */
target
	: target_header '{' target_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

target_header
	: TOKEN_TARGET '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Target); 
			nodePush(parse_state, $$);
		}
	;

target_body
	: /* empty */
	| target_statement target_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

target_statement
	: target_option
	;

target_option
	: TOKEN_INDEX '=' TOKEN_INTEGER ';'					{ $$.type = OT_Index; $$.value.intValue = $3; }
	| TOKEN_BLEND '=' TOKEN_BOOLEAN ';'					{ $$.type = OT_Blend; $$.value.intValue = $3; }
	| blend_color_header '{' blenddef_body '}' ';'		{ nodePop(parse_state); $$.type = OT_Color; $$.value.nodePtr = $1; }
	| blend_alpha_header '{' blenddef_body '}' ';'		{ nodePop(parse_state); $$.type = OT_Alpha; $$.value.nodePtr = $1; }
	| blend_color_header '{' blenddef_body_init '}' ';'		{ nodePop(parse_state); $$.type = OT_Color; $$.value.nodePtr = $1; }
	| blend_alpha_header '{' blenddef_body_init '}' ';'		{ nodePop(parse_state); $$.type = OT_Alpha; $$.value.nodePtr = $1; }
	| TOKEN_WRITEMASK '=' TOKEN_COLORMASK ';'			{ $$.type = OT_WriteMask; $$.value.intValue = $3; }
	;

	/* Blend definition */
blend_color_header
	: TOKEN_COLOR '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_BlendDef); 
			nodePush(parse_state, $$);
		}
	;

blend_alpha_header
	: TOKEN_ALPHA '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_BlendDef); 
			nodePush(parse_state, $$);
		}
	;

blenddef_body
	: /* empty */
	| blenddef_option blenddef_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

blenddef_body_init
	: TOKEN_OPVALUE ',' TOKEN_OPVALUE ',' TOKEN_BLENDOPVALUE
		{
			NodeOption src; src.type = OT_Source; src.value.intValue = $1;
			NodeOption dst; dst.type = OT_Dest; dst.value.intValue = $3;
			NodeOption op; op.type = OT_Op; op.value.intValue = $5;

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &src);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &dst);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &op);
		}
	;

blenddef_option
	: TOKEN_SOURCE '=' TOKEN_OPVALUE ';'					{ $$.type = OT_Source; $$.value.intValue = $3; }
	| TOKEN_DEST '=' TOKEN_OPVALUE ';'						{ $$.type = OT_Dest; $$.value.intValue = $3; }
	| TOKEN_OP '=' TOKEN_BLENDOPVALUE ';'					{ $$.type = OT_Op; $$.value.intValue = $3; }
	;

	/* Sampler state */

sampler_state_body
	: /* empty */
	| sampler_state_option sampler_state_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

sampler_state_option
	: addr_mode												{ $$.type = OT_AddrMode; $$.value.nodePtr = $1; }
	| TOKEN_MINFILTER '=' TOKEN_FILTERVALUE ';'				{ $$.type = OT_MinFilter; $$.value.intValue = $3; }
	| TOKEN_MAGFILTER '=' TOKEN_FILTERVALUE ';'				{ $$.type = OT_MagFilter; $$.value.intValue = $3; }
	| TOKEN_MIPFILTER '=' TOKEN_FILTERVALUE ';'				{ $$.type = OT_MipFilter; $$.value.intValue = $3; }
	| TOKEN_MAXANISO '=' TOKEN_INTEGER ';'					{ $$.type = OT_MaxAniso; $$.value.intValue = $3; }
	| TOKEN_MIPBIAS '=' TOKEN_FLOAT ';'						{ $$.type = OT_MipBias; $$.value.floatValue = $3; }
	| TOKEN_MIPMIN '=' TOKEN_FLOAT ';'						{ $$.type = OT_MipMin; $$.value.floatValue = $3; }
	| TOKEN_MIPMAX '=' TOKEN_FLOAT ';'						{ $$.type = OT_MipMax; $$.value.floatValue = $3; }
	| TOKEN_BORDERCOLOR '=' float4 ';'						{ $$.type = OT_BorderColor; memcpy($$.value.matrixValue, $3, sizeof($3)); }
	| TOKEN_COMPAREFUNC '=' TOKEN_COMPFUNCVALUE ';'			{ $$.type = OT_CompareFunc; $$.value.intValue = $3; }
	;

	/* Addresing mode */
addr_mode
	: addr_mode_header '{' addr_mode_body '}' ';'		{ nodePop(parse_state); $$ = $1; }
	| addr_mode_header '{' addr_mode_body_init '}' ';'	{ nodePop(parse_state); $$ = $1; }
	;

addr_mode_header
	: TOKEN_ADDRMODE '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_AddrMode); 
			nodePush(parse_state, $$);
		}
	;

addr_mode_body_init
	: TOKEN_ADDRMODEVALUE ',' TOKEN_ADDRMODEVALUE ',' TOKEN_ADDRMODEVALUE
		{
			NodeOption u; u.type = OT_U; u.value.intValue = $1;
			NodeOption v; v.type = OT_V; v.value.intValue = $3;
			NodeOption w; w.type = OT_W; w.value.intValue = $5;

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &u);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &v);
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &w);
		}
	;

addr_mode_body
	: /* empty */
	| addr_mode_option addr_mode_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

addr_mode_option
	: TOKEN_U '=' TOKEN_ADDRMODEVALUE ';'					{ $$.type = OT_U; $$.value.intValue = $3; }
	| TOKEN_V '=' TOKEN_ADDRMODEVALUE ';'					{ $$.type = OT_V; $$.value.intValue = $3; }
	| TOKEN_W '=' TOKEN_ADDRMODEVALUE ';'					{ $$.type = OT_W; $$.value.intValue = $3; }
	;

	/* Value types */
float2
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	{ $$[0] = $2; $$[1] = $4; }
	;

float3
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	{ $$[0] = $2; $$[1] = $4; $$[2] = $6; }
	;

float4
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	{ $$[0] = $2; $$[1] = $4; $$[2] = $6; $$[3] = $8;}
	;

int2
	: '{' TOKEN_INTEGER ',' TOKEN_INTEGER '}'	{ $$[0] = $2; $$[1] = $4; }
	;

int3
	: '{' TOKEN_INTEGER ',' TOKEN_INTEGER ',' TOKEN_INTEGER '}'	{ $$[0] = $2; $$[1] = $4; $$[2] = $6; }
	;

int4
	: '{' TOKEN_INTEGER ',' TOKEN_INTEGER ',' TOKEN_INTEGER ',' TOKEN_INTEGER '}'	{ $$[0] = $2; $$[1] = $4; $$[2] = $6; $$[3] = $8;}
	;

mat6
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	
		{ 
			$$[0] = $2; $$[1] = $4; $$[2] = $6; 
			$$[3] = $8; $$[4] = $10; $$[5] = $12;
		}
	;

mat8
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT '}'	
		{ 
			$$[0] = $2; $$[1] = $4; $$[2] = $6; 
			$$[3] = $8; $$[4] = $10; $$[5] = $12;
			$$[6] = $14; $$[7] = $16;
		}
	;

mat9
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	
		{ 
			$$[0] = $2; $$[1] = $4; $$[2] = $6; 
			$$[3] = $8; $$[4] = $10; $$[5] = $12;
			$$[6] = $14; $$[7] = $16; $$[8] = $18;
		}
	;

mat12
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	
		{ 
			$$[0] = $2; $$[1] = $4; $$[2] = $6; $$[3] = $8; 
			$$[4] = $10; $$[5] = $12; $$[6] = $14; $$[7] = $16; 
			$$[8] = $18; $$[9] = $20; $$[10] = $22; $$[11] = $24;
		}
	;

mat16
	: '{' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' 
		  TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT ',' TOKEN_FLOAT '}'	
		{ 
			$$[0] = $2; $$[1] = $4; $$[2] = $6; $$[3] = $8; 
			$$[4] = $10; $$[5] = $12; $$[6] = $14; $$[7] = $16; 
			$$[8] = $18; $$[9] = $20; $$[10] = $22; $$[11] = $24;
			$$[12] = $26; $$[13] = $28; $$[14] = $30; $$[15] = $32;
		}
	;

	/* Parameters */
parameters
	: parameters_header '{' parameters_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

parameters_header
	: TOKEN_PARAMETERS '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Parameters); 
			nodePush(parse_state, $$);
		}
	;

parameters_body
	: /* empty */
	| parameter parameters_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

parameter
	: param_header_float	qualifier_list param_body_float		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_float2	qualifier_list param_body_float2	';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_float3	qualifier_list param_body_float3	';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_float4	qualifier_list param_body_float4	';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_int		qualifier_list param_body_int		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_int2		qualifier_list param_body_int2		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_int3		qualifier_list param_body_int3		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_int4		qualifier_list param_body_int4		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_color	qualifier_list param_body_float4	';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat2x2	qualifier_list param_body_float4	';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat2x3	qualifier_list param_body_mat6		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat2x4	qualifier_list param_body_mat8		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat3x2	qualifier_list param_body_mat6		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat3x3	qualifier_list param_body_mat9		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat3x4	qualifier_list param_body_mat12		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat4x2	qualifier_list param_body_mat8		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat4x3	qualifier_list param_body_mat12		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_mat4x4	qualifier_list param_body_mat16		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_texture	qualifier_list param_body_tex		';' { nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$3); nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_buffer	qualifier_list						';' { nodePop(parse_state); $$.type = OT_Parameter; $$.value.nodePtr = $1; }
	| param_header_qualified_sampler		param_body_sampler	';' 
		{ 
			nodePop(parse_state);

			NodeOption samplerState;
			samplerState.type = OT_SamplerState;
			samplerState.value.nodePtr = $1;

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &samplerState); 

			$$.type = OT_Parameter; $$.value.nodePtr = parse_state->topNode; 
			nodePop(parse_state); 
		}
	;

param_header_float 
	: TOKEN_FLOATTYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_float2
	: TOKEN_FLOAT2TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_float3
	: TOKEN_FLOAT3TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_float4 
	: TOKEN_FLOAT4TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_int 
	: TOKEN_INTTYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_int2
	: TOKEN_INT2TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_int3
	: TOKEN_INT3TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_int4
	: TOKEN_INT4TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_color
	: TOKEN_COLORTYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat2x2
	: TOKEN_MAT2x2TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat2x3
	: TOKEN_MAT2x3TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat2x4
	: TOKEN_MAT2x4TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat3x2
	: TOKEN_MAT3x2TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat3x3
	: TOKEN_MAT3x3TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat3x4
	: TOKEN_MAT3x4TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat4x2
	: TOKEN_MAT4x2TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat4x3
	: TOKEN_MAT4x3TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_mat4x4
	: TOKEN_MAT4x4TYPE TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_sampler
	: TOKEN_SAMPLER1D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_SAMPLER2D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_SAMPLER3D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_SAMPLERCUBE		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_SAMPLER2DMS		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_texture
	: TOKEN_TEXTURE1D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_TEXTURE2D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_TEXTURE3D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_TEXTURECUBE		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_TEXTURE2DMS		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWTEXTURE1D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWTEXTURE2D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWTEXTURE3D		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWTEXTURE2DMS	TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_buffer
	: TOKEN_BYTEBUFFER		TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_STRUCTBUFFER	TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWTYPEDBUFFER	TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWBYTEBUFFER	TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWSTRUCTBUFFER	TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWAPPENDBUFFER	TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	| TOKEN_RWCONSUMEBUFFER TOKEN_IDENTIFIER { ADD_PARAMETER($$, $1, $2); }
	;

param_header_qualified_sampler
	: param_header_sampler qualifier_list	
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_SamplerState); 
			nodePush(parse_state, $$);
		}
	;

param_body_float 
	: /* empty */		{ $$.type = OT_None; }
	| '=' TOKEN_FLOAT	{ $$.type = OT_ParamValue; $$.value.floatValue = $2; }
	;

param_body_float2
	: /* empty */		{ $$.type = OT_None; }
	| '=' float2		{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_float3
	: /* empty */		{ $$.type = OT_None; }
	| '=' float3		{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_float4
	: /* empty */		{ $$.type = OT_None; }
	| '=' float4		{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_int
	: /* empty */		{ $$.type = OT_None; }
	| '=' TOKEN_INTEGER	{ $$.type = OT_ParamValue; $$.value.intValue = $2; }
	;

param_body_int2
	: /* empty */		{ $$.type = OT_None; }
	| '=' int2			{ $$.type = OT_ParamValue; memcpy($$.value.intVectorValue, $2, sizeof($2)); }
	;

param_body_int3
	: /* empty */		{ $$.type = OT_None; }
	| '=' int3			{ $$.type = OT_ParamValue; memcpy($$.value.intVectorValue, $2, sizeof($2)); }
	;

param_body_int4
	: /* empty */		{ $$.type = OT_None; }
	| '=' int4			{ $$.type = OT_ParamValue; memcpy($$.value.intVectorValue, $2, sizeof($2)); }
	;

param_body_mat6
	: /* empty */		{ $$.type = OT_None; }
	| '=' mat6			{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_mat8
	: /* empty */		{ $$.type = OT_None; }
	| '=' mat8			{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_mat9
	: /* empty */		{ $$.type = OT_None; }
	| '=' mat9			{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_mat12
	: /* empty */		{ $$.type = OT_None; }
	| '=' mat12			{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_mat16
	: /* empty */		{ $$.type = OT_None; }
	| '=' mat16			{ $$.type = OT_ParamValue; memcpy($$.value.matrixValue, $2, sizeof($2)); }
	;

param_body_sampler
	: /* empty */		{ $$.type = OT_None; }
	| '=' '{' sampler_state_body '}' { }
	;

param_body_tex
	: /* empty */		{ $$.type = OT_None; }
	| '=' TOKEN_STRING	{ $$.type = OT_ParamStrValue; $$.value.strValue = $2; }
	;

	/* Blocks */
blocks
	: blocks_header '{' blocks_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

blocks_header
	: TOKEN_BLOCKS '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Blocks); 
			nodePush(parse_state, $$);
		}
	;

blocks_body
	: /* empty */
	| block blocks_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

block
	: block_header qualifier_list ';' { nodePop(parse_state); $$.type = OT_Block; $$.value.nodePtr = $1; }
	;

block_header
	: TOKEN_PARAMSBLOCK TOKEN_IDENTIFIER
		{
			$$ = nodeCreate(parse_state->memContext, NT_Block);
			nodePush(parse_state, $$);

			NodeOption blockName;
			blockName.type = OT_Identifier;
			blockName.value.strValue = $2;

			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &blockName);
		}
	;

	/* Qualifiers */
qualifier_list
	: /* empty */
	| qualifier qualifier_list		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

qualifier
	: ':' TOKEN_ALIAS '(' TOKEN_STRING ')'		{ $$.type = OT_Alias; $$.value.strValue = $4; }
	| ':' TOKEN_AUTO '(' TOKEN_STRING ')'		{ $$.type = OT_Auto; $$.value.strValue = $4; }
	| ':' TOKEN_SHARED '(' TOKEN_BOOLEAN ')'	{ $$.type = OT_Shared; $$.value.intValue = $4; }
	| ':' TOKEN_USAGE '(' TOKEN_BUFFERUSAGE ')'	{ $$.type = OT_Usage; $$.value.intValue = $4; }
	;
%%

void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg) 
{ 
	parse_state->hasError = 1;
	parse_state->errorLine = locp->first_line;
	parse_state->errorColumn = locp->first_column;
	parse_state->errorMessage = mmalloc_strdup(parse_state->memContext, msg);
	parse_state->errorFile = locp->filename;
}