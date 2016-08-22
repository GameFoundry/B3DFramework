//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#ifndef __ASTFX_H__
#define __ASTFX_H__

#include <stdlib.h>
#include <string.h>

enum tagNodeType
{
	NT_Shader,
	NT_Technique,
	NT_Parameters,
	NT_Blocks,
	NT_Pass,
	NT_StencilOp,
	NT_Target,
	NT_BlendDef,
	NT_SamplerState,
	NT_AddrMode,
	NT_Parameter,
	NT_Block,
	NT_Tags,
	NT_CodeVertex,
	NT_CodeFragment,
	NT_CodeGeometry,
	NT_CodeHull,
	NT_CodeDomain,
	NT_CodeCompute,
	NT_CodeCommon,
};

enum tagOptionType
{
	OT_None = 0,
	OT_Separable,
	OT_Priority,
	OT_Sort,
	OT_Transparent,
	OT_Technique,
	OT_Renderer,
	OT_Language,
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
	OT_Stencil,
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
	OT_Blend,
	OT_Color,
	OT_Alpha,
	OT_WriteMask,
	OT_Source,
	OT_Dest,
	OT_Op,
	OT_AddrMode,
	OT_MinFilter,
	OT_MagFilter,
	OT_MipFilter,
	OT_MaxAniso,
	OT_MipBias,
	OT_MipMin,
	OT_MipMax,
	OT_BorderColor,
	OT_U,
	OT_V,
	OT_W,
	OT_Alias,
	OT_Auto,
	OT_Shared,
	OT_Usage,
	OT_ParamType,
	OT_Identifier,
	OT_ParamValue,
	OT_ParamStrValue,
	OT_Parameters,
	OT_Blocks,
	OT_Parameter,
	OT_Block,
	OT_SamplerState,
	OT_Code,
	OT_StencilRef,
	OT_Tags,
	OT_TagValue,
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

enum tagParamType
{
	PT_Float, PT_Float2, PT_Float3, PT_Float4, 
	PT_Int, PT_Int2, PT_Int3, PT_Int4, PT_Color,
	PT_Mat2x2, PT_Mat2x3, PT_Mat2x4,
	PT_Mat3x2, PT_Mat3x3, PT_Mat3x4,
	PT_Mat4x2, PT_Mat4x3, PT_Mat4x4,
	PT_Sampler1D, PT_Sampler2D, PT_Sampler3D, PT_SamplerCUBE, PT_Sampler2DMS,
	PT_Texture1D, PT_Texture2D, PT_Texture3D, PT_TextureCUBE, PT_Texture2DMS,
	PT_RWTexture1D, PT_RWTexture2D, PT_RWTexture3D, PT_RWTexture2DMS,
	PT_ByteBuffer, PT_StructBuffer, PT_ByteBufferRW, PT_StructBufferRW,
	PT_TypedBufferRW, PT_AppendBuffer, PT_ConsumeBuffer,
	PT_Count // Keep at end
};

enum tagFillModeValue 
{ 
	FMV_Wire, FMV_Solid 
};

enum tagCullModeValue 
{ 
	CMV_None, CMV_CW, CMV_CCW 
};

enum tagCompFuncValue
{ 
	CFV_Fail, CFV_Pass, CFV_LT, CFV_LTE, 
	CFV_EQ, CFV_NEQ, CFV_GTE, CFV_GT
};

enum tagOpValue 
{ 
	OV_Keep, OV_Zero, OV_Replace, OV_Incr, OV_Decr, 
	OV_IncrWrap, OV_DecrWrap, OV_Invert, OV_One, OV_DestColor, 
	OV_SrcColor, OV_InvDestColor, OV_InvSrcColor, OV_DestAlpha, 
	OV_SrcAlpha, OV_InvDestAlpha, OV_InvSrcAlpha
};

enum tagBlendOpValue 
{ 
	BOV_Add, BOV_Subtract, BOV_RevSubtract, 
	BOV_Min, BOV_Max 
};

enum tagAddrModeValue
{
	AMV_Wrap, AMV_Mirror, AMV_Clamp, AMV_Border
};

enum tagFilterValue 
{ 
	FV_None, FV_Point, FV_Linear, FV_Anisotropic, 
	FV_PointCmp, FV_LinearCmp, FV_AnisotropicCmp 
};

enum tagBufferUsageValue 
{ 
	BUV_Static, BUV_Dynamic 
};

enum tagQueueSortTypeValue
{
	QST_FrontToBack, QST_BackToFront, QST_None
};

typedef enum tagNodeType NodeType;
typedef enum tagOptionType OptionType;
typedef enum tagOptionDataType OptionDataType;
typedef enum tagParamType ParamType;
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
typedef struct tagCodeString CodeString;
typedef struct tagDefineEntry DefineEntry;
typedef enum tagFillModeValue FillModeValue;
typedef enum tagCullModeValue CullModeValue;
typedef enum tagCompFuncValue CompFuncValue;
typedef enum tagOpValue OpValue;
typedef enum tagBlendOpValue BlendOpValue;
typedef enum tagAddrModeValue AddrModeValue;
typedef enum tagFilterValue FilterValue;
typedef enum tagBufferUsageValue BufferUsageValue;
typedef enum tagQueueSortTypeValue QueueSortTypeValue;

struct tagNodeLink
{
	ASTFXNode* node;
	NodeLink* next;
};

struct tagIncludeData
{
	char* filename;
	char* buffer;
};

struct tagIncludeLink
{
	IncludeData* data;
	IncludeLink* next;
};

struct tagConditionalData
{
	int selfEnabled;
	int enabled;

	ConditionalData* next;
};

struct tagCodeString
{
	char* code;
	int index;
	int size;
	int capacity;

	CodeString* next;
};

struct tagDefineEntry
{
	char* name;
	char* expr;
};

struct tagParseState
{
	ASTFXNode* rootNode;
	ASTFXNode* topNode;
	void* memContext;

	int hasError;
	int errorLine;
	int errorColumn;
	const char* errorMessage;
	char* errorFile;

	NodeLink* nodeStack;
	IncludeLink* includeStack;
	IncludeLink* includes;
	CodeString* codeStrings;
	int numCodeStrings;
	int numOpenBrackets;

	DefineEntry* defines;
	int numDefines;
	int defineCapacity;
	ConditionalData* conditionalStack;
};

struct tagOptionInfo
{
	OptionType type;
	OptionDataType dataType;
};

union tagOptionData
{
	int intValue;
	float floatValue;
	const char* strValue;
	float matrixValue[16];
	int intVectorValue[4];
	ASTFXNode* nodePtr;
};

struct tagNodeOption
{
	OptionType type;
	OptionData value;
};

struct tagNodeOptions
{
	NodeOption* entries;

	int count;
	int bufferSize;
};

struct tagASTFXNode
{
	NodeType type;
	NodeOptions* options;
};

extern OptionInfo OPTION_LOOKUP[OT_Count];

NodeOptions* nodeOptionsCreate(void* context);
void nodeOptionDelete(NodeOption* option);
void nodeOptionsDelete(NodeOptions* options);
void nodeOptionsResize(void* context, NodeOptions* options, int size);
void nodeOptionsGrowIfNeeded(void* context, NodeOptions* options);
void nodeOptionsAdd(void* context, NodeOptions* options, const NodeOption* option);

ASTFXNode* nodeCreate(void* context, NodeType type);
void nodeDelete(ASTFXNode* node);

void nodePush(ParseState* parseState, ASTFXNode* node);
void nodePop(ParseState* parseState);

void beginCodeBlock(ParseState* parseState);
void appendCodeBlock(ParseState* parseState, const char* value, int size);
int getCodeBlockIndex(ParseState* parseState);

void addDefine(ParseState* parseState, const char* value);
void addDefineExpr(ParseState* parseState, const char* value);
int hasDefine(ParseState* parseState, const char* value);
void removeDefine(ParseState* parseState, const char* value);

int pushConditional(ParseState* parseState, int state);
int switchConditional(ParseState* parseState);
int setConditional(ParseState* parseState, int state);
int popConditional(ParseState* parseState);

char* getCurrentFilename(ParseState* parseState);

ParseState* parseStateCreate();
void parseStateDelete(ParseState* parseState);

#endif