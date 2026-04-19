//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef __ASTFX_H__
#define __ASTFX_H__

#include <stdlib.h>
#include <string.h>

enum tagNodeType
{
	NT_Root,
	NT_Options,
	NT_Shader,
	NT_SubShader,
	NT_Mixin,
	NT_Pass,
	NT_Blend,
	NT_Raster,
	NT_Depth,
	NT_Stencil,
	NT_Target,
	NT_StencilOp,
	NT_BlendDef,
	NT_Tags,
	NT_Code,
	NT_Variations,
	NT_Variation,
	NT_VariationOption,
	NT_Attributes
};

enum tagOptionType
{
	OT_None = 0,
	OT_Options,
	OT_Separable,
	OT_Priority,
	OT_Sort,
	OT_Transparent,
	OT_Shader,
	OT_SubShader,
	OT_Mixin,
	OT_Raster,
	OT_Depth,
	OT_Stencil,
	OT_Blend,
	OT_FeatureSet,
	OT_Pass,
	OT_FillMode,
	OT_CullMode,
	OT_DepthBias,
	OT_SDepthBias,
	OT_DepthClip,
	OT_Scissor,
	OT_Multisample,
	OT_AALine,
	OT_DepthRead,
	OT_DepthWrite,
	OT_CompareFunc,
	OT_StencilReadMask,
	OT_StencilWriteMask,
	OT_StencilOpFront,
	OT_StencilOpBack,
	OT_PassOp,
	OT_Fail,
	OT_ZFail,
	OT_AlphaToCoverage,
	OT_IndependantBlend,
	OT_Target,
	OT_Index,
	OT_Enabled,
	OT_Color,
	OT_Alpha,
	OT_WriteMask,
	OT_Source,
	OT_Dest,
	OT_Op,
	OT_Identifier,
	OT_Code,
	OT_StencilRef,
	OT_Tags,
	OT_TagValue,
	OT_Variations,
	OT_Variation,
	OT_VariationValue,
	OT_Forward,
	OT_Attributes,
	OT_AttrName,
	OT_VariationOption,
	OT_AttrShow,
	OT_Count
};

enum tagOptionDataType
{
	ODT_Int,
	ODT_Float,
	ODT_Bool,
	ODT_String,
	ODT_Complex,
	ODT_Matrix
};

enum tagFillModeValue
{
	FMV_Wire,
	FMV_Solid
};

enum tagCullAndSortModeValue
{
	CASV_None,
	CASV_CW,
	CASV_CCW,
	CASV_FrontToBack,
	CASV_BackToFront
};

enum tagCompFuncValue
{
	CFV_Fail,
	CFV_Pass,
	CFV_LT,
	CFV_LTE,
	CFV_EQ,
	CFV_NEQ,
	CFV_GTE,
	CFV_GT
};

enum tagOpValue
{
	OV_Keep,
	OV_Zero,
	OV_Replace,
	OV_Incr,
	OV_Decr,
	OV_IncrWrap,
	OV_DecrWrap,
	OV_Invert,
	OV_One,
	OV_DestColor,
	OV_SrcColor,
	OV_InvDestColor,
	OV_InvSrcColor,
	OV_DestAlpha,
	OV_SrcAlpha,
	OV_InvDestAlpha,
	OV_InvSrcAlpha
};

enum tagBlendOpValue
{
	BOV_Add,
	BOV_Subtract,
	BOV_RevSubtract,
	BOV_Min,
	BOV_Max
};

enum tagRawCodeType
{
	RCT_CodeBlock,
	RCT_SubShaderBlock,
	RCT_Count
};

enum tagConditionalOp
{
	CO_None,
	CO_Equals,
	CO_NotEquals,
	CO_Lesser,
	CO_Greater,
	CO_LesserEqual,
	CO_GreaterEqual
};

typedef enum tagNodeType NodeType;
typedef enum tagOptionType OptionType;
typedef enum tagOptionDataType OptionDataType;
typedef struct tagParseState ParseState;
typedef struct tagOptionInfo OptionInfo;
typedef union tagOptionData OptionData;
typedef struct tagNodeOptions NodeOptions;
typedef struct tagNodeOption NodeOption;
typedef struct tagASTFXNode ASTFXNode;
typedef struct tagNodeLink NodeLink;
typedef struct tagIncludeData IncludeData;
typedef struct tagIncludeLink IncludeLink;
typedef struct tagConditionalData ConditionalData;
typedef struct tagRawCode RawCode;
typedef struct tagDefineEntry DefineEntry;
typedef enum tagFillModeValue FillModeValue;
typedef enum tagCullAndSortModeValue CullAndSortModeValue;
typedef enum tagCompFuncValue CompFuncValue;
typedef enum tagOpValue OpValue;
typedef enum tagBlendOpValue BlendOpValue;
typedef enum tagRawCodeType RawCodeType;
typedef enum tagConditionalOp ConditionalOp;

struct tagNodeLink
{
	ASTFXNode* Node;
	NodeLink* Next;
};

struct tagIncludeData
{
	char* Filename;
	char* Buffer;
};

struct tagIncludeLink
{
	IncludeData* Data;
	IncludeLink* Next;
};

struct tagConditionalData
{
	char* Name;
	int SelfEnabled;
	int Enabled;
	ConditionalOp Op;
	char* Value;

	ConditionalData* Next;
};

struct tagRawCode
{
	char* Code;
	int Index;
	int Size;
	int Capacity;

	RawCode* Next;
};

struct tagDefineEntry
{
	char* Name;
	char* Expr;
};

struct tagParseState
{
	ASTFXNode* RootNode;
	ASTFXNode* TopNode;
	void* MemContext;

	int HasError;
	int ErrorLine;
	int ErrorColumn;
	const char* ErrorMessage;
	char* ErrorFile;

	NodeLink* NodeStack;
	IncludeLink* IncludeStack;
	IncludeLink* Includes;
	RawCode* RawCodeBlock[RCT_Count];
	int NumRawCodeBlocks[RCT_Count];
	int NumOpenBrackets;

	DefineEntry* Defines;
	int NumDefines;
	int DefineCapacity;
	ConditionalData* ConditionalStack;
};

struct tagOptionInfo
{
	OptionType Type;
	OptionDataType DataType;
};

union tagOptionData
{
	int IntValue;
	float FloatValue;
	const char* StrValue;
	float MatrixValue[16];
	int IntVectorValue[4];
	ASTFXNode* NodePtr;
};

struct tagNodeOption
{
	OptionType Type;
	OptionData Value;
};

struct tagNodeOptions
{
	NodeOption* Entries;

	int Count;
	int BufferSize;
};

struct tagASTFXNode
{
	NodeType Type;
	NodeOptions* Options;
};

extern OptionInfo OPTION_LOOKUP[OT_Count];

NodeOptions* NodeOptionsCreate(void* context);
void NodeOptionDelete(NodeOption* option);
void NodeOptionsDelete(NodeOptions* options);
void NodeOptionsResize(void* context, NodeOptions* options, int size);
void NodeOptionsGrowIfNeeded(void* context, NodeOptions* options);
void NodeOptionsAdd(void* context, NodeOptions* options, const NodeOption* option);

ASTFXNode* NodeCreate(void* context, NodeType type);
void NodeDelete(ASTFXNode* node);

void NodePush(ParseState* parseState, ASTFXNode* node);
void NodePop(ParseState* parseState);

void BeginCodeBlock(ParseState* parseState, RawCodeType type);
void AppendCodeBlock(ParseState* parseState, RawCodeType type, const char* value, int size);
int GetCodeBlockIndex(ParseState* parseState, RawCodeType type);

void AddDefine(ParseState* parseState, const char* value);
void AddDefineExpr(ParseState* parseState, const char* value);
int HasDefine(ParseState* parseState, const char* value);
void RemoveDefine(ParseState* parseState, const char* value);

int PushConditionalDef(ParseState* parseState, int state);
void PushConditional(ParseState* parseState, const char* name);
void SetConditionalOp(ParseState* parseState, ConditionalOp op);
void SetConditionalVal(ParseState* parseState, const char* value);
int EvalConditional(ParseState* parseState);
int SwitchConditional(ParseState* parseState);
void SetConditional(ParseState* parseState, const char* name);
int PopConditional(ParseState* parseState);

char* GetCurrentFilename(ParseState* parseState);

ParseState* ParseStateCreate();
void ParseStateDelete(ParseState* parseState);

#endif
