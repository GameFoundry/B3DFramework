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

	/* State value types */
%token <intValue>	TOKEN_FILLMODEVALUE
%token <intValue>	TOKEN_CULLANDQUEUEVALUE
%token <intValue>	TOKEN_COMPFUNCVALUE
%token <intValue>	TOKEN_OPVALUE
%token <intValue>	TOKEN_BLENDOPVALUE
%token <intValue>	TOKEN_COLORMASK

	/* Shader keywords */
%token TOKEN_OPTIONS TOKEN_SHADER TOKEN_SUBSHADER TOKEN_MIXIN

	/* Options keywords */
%token TOKEN_SEPARABLE TOKEN_SORT TOKEN_PRIORITY TOKEN_TRANSPARENT
	
	/* Technique keywords */
%token TOKEN_FEATURESET TOKEN_PASS TOKEN_TAGS TOKEN_VARIATIONS

	/* Pass keywords */
%token TOKEN_CODE TOKEN_BLEND TOKEN_RASTER TOKEN_DEPTH TOKEN_STENCIL

	/* Variation keywords */
%token TOKEN_VARIATION

	/* Rasterizer state keywords */
%token TOKEN_FILLMODE TOKEN_CULLMODE TOKEN_DEPTHBIAS TOKEN_SDEPTHBIAS
%token TOKEN_DEPTHCLIP TOKEN_SCISSOR TOKEN_MULTISAMPLE TOKEN_AALINE

	/* Depth state keywords */
%token TOKEN_DEPTHREAD TOKEN_DEPTHWRITE TOKEN_COMPAREFUNC

	/* Stencil state keywords */
%token TOKEN_STENCILREF TOKEN_ENABLED TOKEN_READMASK TOKEN_WRITEMASK 
%token TOKEN_STENCILOPFRONT TOKEN_STENCILOPBACK TOKEN_FAIL TOKEN_ZFAIL

	/* Blend state keywords */
%token TOKEN_ALPHATOCOVERAGE TOKEN_INDEPENDANTBLEND TOKEN_TARGET TOKEN_INDEX
%token TOKEN_COLOR TOKEN_ALPHA TOKEN_SOURCE TOKEN_DEST TOKEN_OP

%type <nodePtr>		root;
%type <nodeOption>	root_statement;

%type <nodePtr>		options;
%type <nodePtr>		options_header;
%type <nodeOption>	options_option;

%type <nodePtr>		shader;
%type <nodePtr>		shader_header;
%type <nodeOption>	shader_statement;
%type <nodeOption>	shader_option;
%type <nodePtr>		tags;
%type <nodePtr>		tags_header;

%type <nodePtr>		subshader;
%type <nodePtr>		subshader_header;
%type <nodeOption>	subshader_statement;

%type <nodePtr>		pass;
%type <nodePtr>		pass_header;
%type <nodeOption>	pass_statement;
%type <nodeOption>	pass_option;

%type <nodePtr>		variations;
%type <nodePtr>		variations_header;
%type <nodeOption>	variation;
%type <nodePtr>		variation_header;
%type <nodeOption>  variation_option;

%type <nodePtr>		raster;
%type <nodePtr>		raster_header;
%type <nodeOption>	raster_option;

%type <nodePtr>		depth;
%type <nodePtr>		depth_header;
%type <nodeOption>	depth_option;

%type <nodePtr>		stencil;
%type <nodePtr>		stencil_header;
%type <nodeOption>	stencil_option;

%type <nodePtr>		blend;
%type <nodePtr>		blend_header;
%type <nodeOption>	blend_option;

%type <nodePtr>		code;
%type <nodePtr>		code_header;

%type <nodePtr>		stencil_op_front_header;
%type <nodePtr>		stencil_op_back_header;
%type <nodeOption>	stencil_op_option;

%type <nodePtr>		target;
%type <nodePtr>		target_header;
%type <nodeOption>	target_option;

%type <nodePtr>		blend_color_header;
%type <nodePtr>		blend_alpha_header;
%type <nodeOption>	blenddef_option;

%%

	/* Root */

root
	: /* empty */				{ }
	| root_statement root	{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

root_statement
	: options			{ $$.type = OT_Options; $$.value.nodePtr = $1; }
	| shader			{ $$.type = OT_Shader; $$.value.nodePtr = $1; }
	| subshader			{ $$.type = OT_SubShader; $$.value.nodePtr = $1; }
	;

	/* Options */
options
	: options_header '{' options_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

options_header
	: TOKEN_OPTIONS
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Options); 
			nodePush(parse_state, $$);
		}
	;	
	
options_body
	: /* empty */
	| options_option options_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;
	
options_option
	: TOKEN_SEPARABLE '=' TOKEN_BOOLEAN ';'			{ $$.type = OT_Separable; $$.value.intValue = $3; }
	| TOKEN_SORT '=' TOKEN_CULLANDQUEUEVALUE ';'	{ $$.type = OT_Sort; $$.value.intValue = $3; }
	| TOKEN_PRIORITY '=' TOKEN_INTEGER ';'			{ $$.type = OT_Priority; $$.value.intValue = $3; }
	| TOKEN_TRANSPARENT '=' TOKEN_BOOLEAN ';'		{ $$.type = OT_Transparent; $$.value.intValue = $3; }
	;

	/* Shader */

shader
	: shader_header '{' shader_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

shader_header
	: TOKEN_SHADER TOKEN_IDENTIFIER
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Shader); 
			nodePush(parse_state, $$);
			
			NodeOption entry; entry.type = OT_Identifier; entry.value.strValue = $2;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}
	| TOKEN_MIXIN TOKEN_IDENTIFIER
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Mixin); 
			nodePush(parse_state, $$);
			
			NodeOption entry; entry.type = OT_Identifier; entry.value.strValue = $2;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}
	;

shader_body
	: /* empty */
	| shader_statement shader_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

shader_statement
	: shader_option
	| pass				{ $$.type = OT_Pass; $$.value.nodePtr = $1; }
	| pass_option
	| code				{ $$.type = OT_Code; $$.value.nodePtr = $1; }
	;

shader_option
	: TOKEN_FEATURESET '=' TOKEN_IDENTIFIER ';'	{ $$.type = OT_FeatureSet; $$.value.strValue = $3; }
	| TOKEN_MIXIN TOKEN_IDENTIFIER ';'			{ $$.type = OT_Mixin; $$.value.strValue = $2; }
	| tags										{ $$.type = OT_Tags; $$.value.nodePtr = $1; }
	| variations								{ $$.type = OT_Variations; $$.value.nodePtr = $1; }
	;
	
	/* Shader tags */
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
	
	/* Subshader */

subshader
	: subshader_header '{' subshader_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

subshader_header
	: TOKEN_SUBSHADER TOKEN_IDENTIFIER
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_SubShader); 
			nodePush(parse_state, $$);
			
			NodeOption entry; entry.type = OT_Identifier; entry.value.strValue = $2;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		}
	;

subshader_body
	: /* empty */
	| subshader_statement subshader_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

subshader_statement
	: shader			{ $$.type = OT_Shader; $$.value.nodePtr = $1; }
	;

	/* Pass */

pass
	: pass_header '{' pass_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

pass_header
	: TOKEN_PASS
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
	| code							{ $$.type = OT_Code; $$.value.nodePtr = $1; }
	;

pass_option
	: raster						{ $$.type = OT_Raster; $$.value.nodePtr = $1; }
	| depth							{ $$.type = OT_Depth; $$.value.nodePtr = $1; }
	| stencil						{ $$.type = OT_Stencil; $$.value.nodePtr = $1; }
	| blend							{ $$.type = OT_Blend; $$.value.nodePtr = $1; }
	;
	
	/* Variations */

variations
	: variations_header '{' variations_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

variations_header
	: TOKEN_VARIATIONS
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Variations); 
			nodePush(parse_state, $$);
		}
	;

variations_body
	: /* empty */
	| variation variations_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

variation
	: variation_header '{' '}' ';'		{ nodePop(parse_state); $$.type = OT_Variation; $$.value.nodePtr = $1; }
	| variation_header '{' TOKEN_BOOLEAN variation_body '}' ';'		
		{ 
			NodeOption entry; entry.type = OT_VariationValue; entry.value.intValue = $3;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 
		
			nodePop(parse_state); 
			$$.type = OT_Variation; $$.value.nodePtr = $1;
		}
	| variation_header '{' TOKEN_INTEGER variation_body '}' ';'		
		{ 
			NodeOption entry; entry.type = OT_VariationValue; entry.value.intValue = $3;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry);
			
			nodePop(parse_state); 
			$$.type = OT_Variation; $$.value.nodePtr = $1;
		}
	;	
	
variation_header
	: TOKEN_IDENTIFIER '='
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Variation); 
			nodePush(parse_state, $$);
			
			NodeOption entry; entry.type = OT_Identifier; entry.value.strValue = $1;
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &entry); 		
		}
	;
	
variation_body
	: /* empty */
	| variation_option variation_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;
	
variation_option
	: ',' TOKEN_BOOLEAN 	{ $$.type = OT_VariationValue; $$.value.intValue = $2; }
	| ',' TOKEN_INTEGER 	{ $$.type = OT_VariationValue; $$.value.intValue = $2; }
	;

	/* Raster state */
raster
	: raster_header '{' raster_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

raster_header
	: TOKEN_RASTER
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Raster); 
			nodePush(parse_state, $$);
		}
	;

raster_body
	: /* empty */
	| raster_option raster_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

raster_option
	: TOKEN_FILLMODE '=' TOKEN_FILLMODEVALUE ';'				{ $$.type = OT_FillMode; $$.value.intValue = $3; }
	| TOKEN_CULLMODE '=' TOKEN_CULLANDQUEUEVALUE ';'			{ $$.type = OT_CullMode; $$.value.intValue = $3; }
	| TOKEN_SCISSOR '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_Scissor; $$.value.intValue = $3; }
	| TOKEN_MULTISAMPLE '=' TOKEN_BOOLEAN ';'					{ $$.type = OT_Multisample; $$.value.intValue = $3; }
	| TOKEN_AALINE '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_AALine; $$.value.intValue = $3; }
	| TOKEN_DEPTHBIAS '=' TOKEN_FLOAT ';'						{ $$.type = OT_DepthBias; $$.value.floatValue = $3; }
	| TOKEN_SDEPTHBIAS '=' TOKEN_FLOAT ';'						{ $$.type = OT_SDepthBias; $$.value.floatValue = $3; }
	| TOKEN_DEPTHCLIP '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_DepthClip; $$.value.intValue = $3; }	
	;	
	
	/* Depth state */
depth
	: depth_header '{' depth_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

depth_header
	: TOKEN_DEPTH
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Depth); 
			nodePush(parse_state, $$);
		}
	;

depth_body
	: /* empty */
	| depth_option depth_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

depth_option
	: TOKEN_DEPTHREAD '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_DepthRead; $$.value.intValue = $3; }
	| TOKEN_DEPTHWRITE '=' TOKEN_BOOLEAN ';'					{ $$.type = OT_DepthWrite; $$.value.intValue = $3; }
	| TOKEN_COMPAREFUNC '=' TOKEN_COMPFUNCVALUE ';'				{ $$.type = OT_CompareFunc; $$.value.intValue = $3; }
	;
	
	/* Stencil state */
stencil
	: stencil_header '{' stencil_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

stencil_header
	: TOKEN_STENCIL
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Stencil); 
			nodePush(parse_state, $$);
		}
	;

stencil_body
	: /* empty */
	| stencil_option stencil_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

stencil_option
	: TOKEN_ENABLED '=' TOKEN_BOOLEAN ';'						{ $$.type = OT_Enabled; $$.value.intValue = $3; }
	| TOKEN_READMASK '=' TOKEN_INTEGER ';'						{ $$.type = OT_StencilReadMask; $$.value.intValue = $3; }
	| TOKEN_WRITEMASK '=' TOKEN_INTEGER ';'						{ $$.type = OT_StencilWriteMask; $$.value.intValue = $3; }
	| stencil_op_front_header '{' stencil_op_body '}' ';'		{ nodePop(parse_state); $$.type = OT_StencilOpFront; $$.value.nodePtr = $1; }
	| stencil_op_back_header '{' stencil_op_body '}' ';'		{ nodePop(parse_state); $$.type = OT_StencilOpBack; $$.value.nodePtr = $1; }
	| stencil_op_front_header '{' stencil_op_body_init '}' ';'	{ nodePop(parse_state); $$.type = OT_StencilOpFront; $$.value.nodePtr = $1; }
	| stencil_op_back_header '{' stencil_op_body_init '}' ';'	{ nodePop(parse_state); $$.type = OT_StencilOpBack; $$.value.nodePtr = $1; }
	| TOKEN_STENCILREF '=' TOKEN_INTEGER ';'					{ $$.type = OT_StencilRef; $$.value.intValue = $3; }
	;	
	
	/* Blend state */
blend
	: blend_header '{' blend_body '}' ';' { nodePop(parse_state); $$ = $1; }
	;

blend_header
	: TOKEN_BLEND
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Blend); 
			nodePush(parse_state, $$);
		}
	;

blend_body
	: /* empty */
	| blend_option blend_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

blend_option
	: TOKEN_ALPHATOCOVERAGE '=' TOKEN_BOOLEAN ';'				{ $$.type = OT_AlphaToCoverage; $$.value.intValue = $3; }
	| TOKEN_INDEPENDANTBLEND '=' TOKEN_BOOLEAN ';'				{ $$.type = OT_IndependantBlend; $$.value.intValue = $3; }
	| target													{ $$.type = OT_Target; $$.value.nodePtr = $1; }
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
	: TOKEN_CODE
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Code); 
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
	: TOKEN_TARGET
		{ 
			$$ = nodeCreate(parse_state->memContext, NT_Target); 
			nodePush(parse_state, $$);
		}
	;

target_body
	: /* empty */
	| target_option target_body		{ nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &$1); }
	;

target_option
	: TOKEN_INDEX '=' TOKEN_INTEGER ';'					{ $$.type = OT_Index; $$.value.intValue = $3; }
	| TOKEN_ENABLED '=' TOKEN_BOOLEAN ';'				{ $$.type = OT_Enabled; $$.value.intValue = $3; }
	| blend_color_header '{' blenddef_body '}' ';'		{ nodePop(parse_state); $$.type = OT_Color; $$.value.nodePtr = $1; }
	| blend_alpha_header '{' blenddef_body '}' ';'		{ nodePop(parse_state); $$.type = OT_Alpha; $$.value.nodePtr = $1; }
	| blend_color_header '{' blenddef_body_init '}' ';'	{ nodePop(parse_state); $$.type = OT_Color; $$.value.nodePtr = $1; }
	| blend_alpha_header '{' blenddef_body_init '}' ';'	{ nodePop(parse_state); $$.type = OT_Alpha; $$.value.nodePtr = $1; }
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

%%

void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg) 
{ 
	parse_state->hasError = 1;
	parse_state->errorLine = locp->first_line;
	parse_state->errorColumn = locp->first_column;
	parse_state->errorMessage = mmalloc_strdup(parse_state->memContext, msg);
	parse_state->errorFile = locp->filename;
}