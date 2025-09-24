%{
#include "B3DParserFX.h"
#include "B3DLexerFX.h"
#define inline

void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg);
%}

%code requires{
#include "B3DMMAlloc.h"
#include "B3DASTFX.h"
#include "B3DIncludeHandler.h"

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

%output  "B3DParserFX.c"
%defines "B3DParserFX.h"

%define api.pure
%locations
%lex-param { yyscan_t scanner }
%parse-param { ParseState* parse_state }
%parse-param { yyscan_t scanner }
%glr-parser

%union {
	int IntValue;
	float FloatValue;
	float MatrixValue[16];
	int IntVectorValue[4];
	const char* StrValue;
	ASTFXNode* NodePtr;
	NodeOption NodeOptionValue;
}

	/* Value types */
%token <IntValue>	TOKEN_INTEGER
%token <FloatValue> TOKEN_FLOAT
%token <IntValue>	TOKEN_BOOLEAN
%token <StrValue>	TOKEN_STRING
%token <StrValue>	TOKEN_IDENTIFIER

	/* State value types */
%token <IntValue>	TOKEN_FILLMODEVALUE
%token <IntValue>	TOKEN_CULLANDQUEUEVALUE
%token <IntValue>	TOKEN_COMPFUNCVALUE
%token <IntValue>	TOKEN_OPVALUE
%token <IntValue>	TOKEN_BLENDOPVALUE
%token <IntValue>	TOKEN_COLORMASK

	/* Shader keywords */
%token TOKEN_OPTIONS TOKEN_SHADER TOKEN_SUBSHADER TOKEN_MIXIN

	/* Options keywords */
%token TOKEN_SEPARABLE TOKEN_SORT TOKEN_PRIORITY TOKEN_TRANSPARENT TOKEN_FORWARD
	
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

	/* Attribute keywords */
%token TOKEN_NAME TOKEN_SHOW

%type <NodePtr>		root;
%type <NodeOptionValue>	root_statement;

%type <NodePtr>		options;
%type <NodePtr>		options_header;
%type <NodeOptionValue>	options_option;

%type <NodePtr>		shader;
%type <NodePtr>		shader_header;
%type <NodeOptionValue>	shader_statement;
%type <NodeOptionValue>	shader_option;
%type <NodePtr>		tags;
%type <NodePtr>		tags_header;

%type <NodePtr>		subshader;
%type <NodePtr>		subshader_header;

%type <NodePtr>		pass;
%type <NodePtr>		pass_header;
%type <NodeOptionValue>	pass_statement;
%type <NodeOptionValue>	pass_option;

%type <NodePtr>		variations;
%type <NodePtr>		variations_header;
%type <NodePtr>		variation_header_with_attr
%type <NodeOptionValue>	variation;
%type <NodePtr>		variation_header;
%type <NodePtr>  	variation_option;
%type <NodePtr>		variation_option_with_attr;
%type <NodeOptionValue>  variation_option_value;

%type <NodePtr>		raster;
%type <NodePtr>		raster_header;
%type <NodeOptionValue>	raster_option;

%type <NodePtr>		depth;
%type <NodePtr>		depth_header;
%type <NodeOptionValue>	depth_option;

%type <NodePtr>		stencil;
%type <NodePtr>		stencil_header;
%type <NodeOptionValue>	stencil_option;

%type <NodePtr>		blend;
%type <NodePtr>		blend_header;
%type <NodeOptionValue>	blend_option;

%type <NodePtr>		code;
%type <NodePtr>		code_header;

%type <NodePtr>		stencil_op_front_header;
%type <NodePtr>		stencil_op_back_header;
%type <NodeOptionValue>	stencil_op_option;

%type <NodePtr>		target;
%type <NodePtr>		target_header;
%type <NodeOptionValue>	target_option;

%type <NodePtr>		blend_color_header;
%type <NodePtr>		blend_alpha_header;
%type <NodeOptionValue>	blenddef_option;

%type <NodePtr>		attributes;
%type <NodePtr>		attributes_header;
%type <NodeOptionValue>	attribute;

%%

	/* Root */

root
	: /* empty */				{ }
	| root_statement root	{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

root_statement
	: options			{ $$.Type = OT_Options; $$.Value.NodePtr = $1; }
	| shader			{ $$.Type = OT_Shader; $$.Value.NodePtr = $1; }
	| subshader			{ $$.Type = OT_SubShader; $$.Value.NodePtr = $1; }
	;

	/* Options */
options
	: options_header '{' options_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

options_header
	: TOKEN_OPTIONS
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Options); 
			NodePush(parse_state, $$);
		}
	;	
	
options_body
	: /* empty */
	| options_option options_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;
	
options_option
	: TOKEN_SEPARABLE '=' TOKEN_BOOLEAN ';'			{ $$.Type = OT_Separable; $$.Value.IntValue = $3; }
	| TOKEN_SORT '=' TOKEN_CULLANDQUEUEVALUE ';'	{ $$.Type = OT_Sort; $$.Value.IntValue = $3; }
	| TOKEN_PRIORITY '=' TOKEN_INTEGER ';'			{ $$.Type = OT_Priority; $$.Value.IntValue = $3; }
	| TOKEN_TRANSPARENT '=' TOKEN_BOOLEAN ';'		{ $$.Type = OT_Transparent; $$.Value.IntValue = $3; }
	| TOKEN_FORWARD '=' TOKEN_BOOLEAN ';'			{ $$.Type = OT_Forward; $$.Value.IntValue = $3; }
	;

	/* Shader */

shader
	: shader_header '{' shader_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

shader_header
	: TOKEN_SHADER TOKEN_IDENTIFIER
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Shader); 
			NodePush(parse_state, $$);
			
			NodeOption entry; entry.Type = OT_Identifier; entry.Value.StrValue = $2;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry); 
		}
	| TOKEN_MIXIN TOKEN_IDENTIFIER
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Mixin); 
			NodePush(parse_state, $$);
			
			NodeOption entry; entry.Type = OT_Identifier; entry.Value.StrValue = $2;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry); 
		}
	;

shader_body
	: /* empty */
	| shader_statement shader_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

shader_statement
	: shader_option
	| pass				{ $$.Type = OT_Pass; $$.Value.NodePtr = $1; }
	| pass_option
	| code				{ $$.Type = OT_Code; $$.Value.NodePtr = $1; }
	;

shader_option
	: TOKEN_FEATURESET '=' TOKEN_IDENTIFIER ';'	{ $$.Type = OT_FeatureSet; $$.Value.StrValue = $3; }
	| TOKEN_MIXIN TOKEN_IDENTIFIER ';'			{ $$.Type = OT_Mixin; $$.Value.StrValue = $2; }
	| tags										{ $$.Type = OT_Tags; $$.Value.NodePtr = $1; }
	| variations								{ $$.Type = OT_Variations; $$.Value.NodePtr = $1; }
	;
	
	/* Shader tags */
tags
	: tags_header '{' tags_body '}' ';'	{ NodePop(parse_state); $$ = $1; }
	;
	
tags_header
	: TOKEN_TAGS '='
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Tags); 
			NodePush(parse_state, $$);
		}
	;
	
tags_body
	: /* empty */
	| TOKEN_STRING
		{ 
			NodeOption entry; entry.Type = OT_TagValue; entry.Value.StrValue = $1;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry); 
		}		
	| TOKEN_STRING ',' tags_body		
		{ 
			NodeOption entry; entry.Type = OT_TagValue; entry.Value.StrValue = $1;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry); 
		}	
	;
	
	/* Subshader */

subshader
	: subshader_header '{' TOKEN_INDEX '=' TOKEN_INTEGER ';' '}' ';' 
	{ 
		NodeOption index;
		index.Type = OT_Index; 
		index.Value.IntValue = $5;

		NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &index);	
	
		NodePop(parse_state); 
		$$ = $1; 
	}
	;

subshader_header
	: TOKEN_SUBSHADER TOKEN_IDENTIFIER
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_SubShader); 
			NodePush(parse_state, $$);
			
			NodeOption entry; entry.Type = OT_Identifier; entry.Value.StrValue = $2;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry); 
		}
	;

	/* Pass */

pass
	: pass_header '{' pass_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

pass_header
	: TOKEN_PASS
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Pass); 
			NodePush(parse_state, $$);
		}
	;

pass_body
	: /* empty */
	| pass_statement pass_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

pass_statement
	: pass_option
	| code							{ $$.Type = OT_Code; $$.Value.NodePtr = $1; }
	;

pass_option
	: raster						{ $$.Type = OT_Raster; $$.Value.NodePtr = $1; }
	| depth							{ $$.Type = OT_Depth; $$.Value.NodePtr = $1; }
	| stencil						{ $$.Type = OT_Stencil; $$.Value.NodePtr = $1; }
	| blend							{ $$.Type = OT_Blend; $$.Value.NodePtr = $1; }
	;
	
	/* Variations */

variations
	: variations_header '{' variations_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

variations_header
	: TOKEN_VARIATIONS
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Variations); 
			NodePush(parse_state, $$);
		}
	;

variations_body
	: /* empty */
	| variation variations_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

variation
	: variation_header_with_attr '{' variation_body '}' ';'		{ NodePop(parse_state); $$.Type = OT_Variation; $$.Value.NodePtr = $1; }	
	;	
	
variation_header_with_attr
	: variation_header				{ $$ = $1; }
	| attributes variation_header
		{ 
			NodeOption attrEntry; attrEntry.Type = OT_Attributes; attrEntry.Value.NodePtr = $1;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &attrEntry);

			$$ = $2;			
		}
	;
	
variation_header
	: TOKEN_IDENTIFIER '='
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Variation); 
			NodePush(parse_state, $$);
			
			NodeOption entry; entry.Type = OT_Identifier; entry.Value.StrValue = $1;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry); 		
		}
	;
	
variation_body
	: /* empty */
	| variation_option_with_attr						
		{ 
			NodeOption entry; entry.Type = OT_VariationOption; entry.Value.NodePtr = $1; 
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry);
		}
	| variation_option_with_attr ',' variation_body		
		{ 
			NodeOption entry; entry.Type = OT_VariationOption; entry.Value.NodePtr = $1; 
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &entry);
		}
	;
	
variation_option_with_attr
	: variation_option 				
		{ 
			$$ = $1;
			NodePop(parse_state);
		}
	| attributes variation_option 	
		{ 
			NodeOption attrEntry; attrEntry.Type = OT_Attributes; attrEntry.Value.NodePtr = $1;
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &attrEntry);
		
			$$ = $2;
			NodePop(parse_state); 
		}
	;
	
variation_option
	: variation_option_value 	
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_VariationOption); 
			NodePush(parse_state, $$);
			
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); 	 
		}
	;
	
variation_option_value
	: TOKEN_BOOLEAN 	{ $$.Type = OT_VariationValue; $$.Value.IntValue = $1; }
	| TOKEN_INTEGER 	{ $$.Type = OT_VariationValue; $$.Value.IntValue = $1; }
	;

	/* Raster state */
raster
	: raster_header '{' raster_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

raster_header
	: TOKEN_RASTER
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Raster); 
			NodePush(parse_state, $$);
		}
	;

raster_body
	: /* empty */
	| raster_option raster_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

raster_option
	: TOKEN_FILLMODE '=' TOKEN_FILLMODEVALUE ';'				{ $$.Type = OT_FillMode; $$.Value.IntValue = $3; }
	| TOKEN_CULLMODE '=' TOKEN_CULLANDQUEUEVALUE ';'			{ $$.Type = OT_CullMode; $$.Value.IntValue = $3; }
	| TOKEN_SCISSOR '=' TOKEN_BOOLEAN ';'						{ $$.Type = OT_Scissor; $$.Value.IntValue = $3; }
	| TOKEN_MULTISAMPLE '=' TOKEN_BOOLEAN ';'					{ $$.Type = OT_Multisample; $$.Value.IntValue = $3; }
	| TOKEN_AALINE '=' TOKEN_BOOLEAN ';'						{ $$.Type = OT_AALine; $$.Value.IntValue = $3; }
	| TOKEN_DEPTHBIAS '=' TOKEN_FLOAT ';'						{ $$.Type = OT_DepthBias; $$.Value.FloatValue = $3; }
	| TOKEN_SDEPTHBIAS '=' TOKEN_FLOAT ';'						{ $$.Type = OT_SDepthBias; $$.Value.FloatValue = $3; }
	| TOKEN_DEPTHCLIP '=' TOKEN_BOOLEAN ';'						{ $$.Type = OT_DepthClip; $$.Value.IntValue = $3; }	
	;	
	
	/* Depth state */
depth
	: depth_header '{' depth_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

depth_header
	: TOKEN_DEPTH
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Depth); 
			NodePush(parse_state, $$);
		}
	;

depth_body
	: /* empty */
	| depth_option depth_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

depth_option
	: TOKEN_DEPTHREAD '=' TOKEN_BOOLEAN ';'						{ $$.Type = OT_DepthRead; $$.Value.IntValue = $3; }
	| TOKEN_DEPTHWRITE '=' TOKEN_BOOLEAN ';'					{ $$.Type = OT_DepthWrite; $$.Value.IntValue = $3; }
	| TOKEN_COMPAREFUNC '=' TOKEN_COMPFUNCVALUE ';'				{ $$.Type = OT_CompareFunc; $$.Value.IntValue = $3; }
	;
	
	/* Stencil state */
stencil
	: stencil_header '{' stencil_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

stencil_header
	: TOKEN_STENCIL
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Stencil); 
			NodePush(parse_state, $$);
		}
	;

stencil_body
	: /* empty */
	| stencil_option stencil_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

stencil_option
	: TOKEN_ENABLED '=' TOKEN_BOOLEAN ';'						{ $$.Type = OT_Enabled; $$.Value.IntValue = $3; }
	| TOKEN_READMASK '=' TOKEN_INTEGER ';'						{ $$.Type = OT_StencilReadMask; $$.Value.IntValue = $3; }
	| TOKEN_WRITEMASK '=' TOKEN_INTEGER ';'						{ $$.Type = OT_StencilWriteMask; $$.Value.IntValue = $3; }
	| stencil_op_front_header '{' stencil_op_body '}' ';'		{ NodePop(parse_state); $$.Type = OT_StencilOpFront; $$.Value.NodePtr = $1; }
	| stencil_op_back_header '{' stencil_op_body '}' ';'		{ NodePop(parse_state); $$.Type = OT_StencilOpBack; $$.Value.NodePtr = $1; }
	| stencil_op_front_header '{' stencil_op_body_init '}' ';'	{ NodePop(parse_state); $$.Type = OT_StencilOpFront; $$.Value.NodePtr = $1; }
	| stencil_op_back_header '{' stencil_op_body_init '}' ';'	{ NodePop(parse_state); $$.Type = OT_StencilOpBack; $$.Value.NodePtr = $1; }
	| TOKEN_STENCILREF '=' TOKEN_INTEGER ';'					{ $$.Type = OT_StencilRef; $$.Value.IntValue = $3; }
	;	
	
	/* Blend state */
blend
	: blend_header '{' blend_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

blend_header
	: TOKEN_BLEND
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Blend); 
			NodePush(parse_state, $$);
		}
	;

blend_body
	: /* empty */
	| blend_option blend_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

blend_option
	: TOKEN_ALPHATOCOVERAGE '=' TOKEN_BOOLEAN ';'				{ $$.Type = OT_AlphaToCoverage; $$.Value.IntValue = $3; }
	| TOKEN_INDEPENDANTBLEND '=' TOKEN_BOOLEAN ';'				{ $$.Type = OT_IndependantBlend; $$.Value.IntValue = $3; }
	| target													{ $$.Type = OT_Target; $$.Value.NodePtr = $1; }
	;
	
	/* Code blocks */

code
	: code_header '{' TOKEN_INDEX '=' TOKEN_INTEGER ';' '}' ';'
	{
		NodeOption index;
		index.Type = OT_Index; 
		index.Value.IntValue = $5;

		NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &index);

		NodePop(parse_state); 
		$$ = $1;
	}
	;

code_header
	: TOKEN_CODE
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Code); 
			NodePush(parse_state, $$);
		}
	;

	/* Stencil op */

stencil_op_front_header
	: TOKEN_STENCILOPFRONT '='
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_StencilOp); 
			NodePush(parse_state, $$);
		}
	;

stencil_op_back_header
	: TOKEN_STENCILOPBACK '='
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_StencilOp); 
			NodePush(parse_state, $$);
		}
	;

stencil_op_body_init
	: TOKEN_OPVALUE ',' TOKEN_OPVALUE ',' TOKEN_OPVALUE ',' TOKEN_COMPFUNCVALUE
		{
			NodeOption fail; fail.Type = OT_Fail; fail.Value.IntValue = $1;
			NodeOption zfail; zfail.Type = OT_ZFail; zfail.Value.IntValue = $3;
			NodeOption pass; pass.Type = OT_PassOp; pass.Value.IntValue = $5;
			NodeOption cmp; cmp.Type = OT_CompareFunc; cmp.Value.IntValue = $7;

			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &fail);
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &zfail);
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &pass);
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &cmp);
		}
	;

stencil_op_body
	: /* empty */
	| stencil_op_option stencil_op_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

stencil_op_option
	: TOKEN_FAIL '=' TOKEN_OPVALUE ';'					{ $$.Type = OT_Fail; $$.Value.IntValue = $3; }
	| TOKEN_ZFAIL '=' TOKEN_OPVALUE ';'					{ $$.Type = OT_ZFail; $$.Value.IntValue = $3; }
	| TOKEN_PASS '=' TOKEN_OPVALUE ';'					{ $$.Type = OT_PassOp; $$.Value.IntValue = $3; }
	| TOKEN_COMPAREFUNC '=' TOKEN_COMPFUNCVALUE ';'		{ $$.Type = OT_CompareFunc; $$.Value.IntValue = $3; }
	; 

	/* Target */
target
	: target_header '{' target_body '}' ';' { NodePop(parse_state); $$ = $1; }
	;

target_header
	: TOKEN_TARGET
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_Target); 
			NodePush(parse_state, $$);
		}
	;

target_body
	: /* empty */
	| target_option target_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

target_option
	: TOKEN_INDEX '=' TOKEN_INTEGER ';'					{ $$.Type = OT_Index; $$.Value.IntValue = $3; }
	| TOKEN_ENABLED '=' TOKEN_BOOLEAN ';'				{ $$.Type = OT_Enabled; $$.Value.IntValue = $3; }
	| blend_color_header '{' blenddef_body '}' ';'		{ NodePop(parse_state); $$.Type = OT_Color; $$.Value.NodePtr = $1; }
	| blend_alpha_header '{' blenddef_body '}' ';'		{ NodePop(parse_state); $$.Type = OT_Alpha; $$.Value.NodePtr = $1; }
	| blend_color_header '{' blenddef_body_init '}' ';'	{ NodePop(parse_state); $$.Type = OT_Color; $$.Value.NodePtr = $1; }
	| blend_alpha_header '{' blenddef_body_init '}' ';'	{ NodePop(parse_state); $$.Type = OT_Alpha; $$.Value.NodePtr = $1; }
	| TOKEN_WRITEMASK '=' TOKEN_COLORMASK ';'			{ $$.Type = OT_WriteMask; $$.Value.IntValue = $3; }
	;

	/* Blend definition */
blend_color_header
	: TOKEN_COLOR '='
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_BlendDef); 
			NodePush(parse_state, $$);
		}
	;

blend_alpha_header
	: TOKEN_ALPHA '='
		{ 
			$$ = NodeCreate(parse_state->MemContext, NT_BlendDef); 
			NodePush(parse_state, $$);
		}
	;

blenddef_body
	: /* empty */
	| blenddef_option blenddef_body		{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;

blenddef_body_init
	: TOKEN_OPVALUE ',' TOKEN_OPVALUE ',' TOKEN_BLENDOPVALUE
		{
			NodeOption src; src.Type = OT_Source; src.Value.IntValue = $1;
			NodeOption dst; dst.Type = OT_Dest; dst.Value.IntValue = $3;
			NodeOption op; op.Type = OT_Op; op.Value.IntValue = $5;

			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &src);
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &dst);
			NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &op);
		}
	;

blenddef_option
	: TOKEN_SOURCE '=' TOKEN_OPVALUE ';'					{ $$.Type = OT_Source; $$.Value.IntValue = $3; }
	| TOKEN_DEST '=' TOKEN_OPVALUE ';'						{ $$.Type = OT_Dest; $$.Value.IntValue = $3; }
	| TOKEN_OP '=' TOKEN_BLENDOPVALUE ';'					{ $$.Type = OT_Op; $$.Value.IntValue = $3; }
	;
	
	/* Attribute */
attributes
	: attributes_header attributes_body ']' { NodePop(parse_state); $$ = $1; }
	;
	
attributes_header
	: '['
		{
			$$ = NodeCreate(parse_state->MemContext, NT_Attributes); 
			NodePush(parse_state, $$);
		}
	;
	
attributes_body
	: /* empty */
	| attribute								{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	| attribute ','	attributes_body			{ NodeOptionsAdd(parse_state->MemContext, parse_state->TopNode->Options, &$1); }
	;
	
attribute
	: TOKEN_NAME '(' TOKEN_STRING ')'		{ $$.Type = OT_AttrName; $$.Value.StrValue = $3; }
	| TOKEN_SHOW 							{ $$.Type = OT_AttrShow; $$.Value.IntValue = 0; }
	;

%%

void yyerror(YYLTYPE *locp, ParseState* parse_state, yyscan_t scanner, const char *msg) 
{ 
	parse_state->HasError = 1;
	parse_state->ErrorLine = locp->first_line;
	parse_state->ErrorColumn = locp->first_column;
	parse_state->ErrorMessage = MmallocStrdup(parse_state->MemContext, msg);
	parse_state->ErrorFile = locp->filename;
}
