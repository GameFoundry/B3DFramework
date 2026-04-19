//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DASTFX.h"
#include "B3DMMAlloc.h"
#include <assert.h>

OptionInfo OPTION_LOOKUP[] =
{
	{ OT_None, ODT_Int },
	{ OT_Options, ODT_Complex },
	{ OT_Separable, ODT_Bool },
	{ OT_Priority, ODT_Int },
	{ OT_Sort, ODT_Int },
	{ OT_Transparent, ODT_Bool },
	{ OT_Shader, ODT_Complex },
	{ OT_SubShader, ODT_Complex },
	{ OT_Mixin, ODT_String },
	{ OT_Raster, ODT_Complex },
	{ OT_Depth, ODT_Complex },
	{ OT_Stencil, ODT_Complex },
	{ OT_Blend, ODT_Complex },
	{ OT_FeatureSet, ODT_String },
	{ OT_Pass, ODT_Complex },
	{ OT_FillMode, ODT_Int },
	{ OT_CullMode, ODT_Int },
	{ OT_DepthBias, ODT_Float },
	{ OT_SDepthBias, ODT_Float },
	{ OT_DepthClip, ODT_Bool },
	{ OT_Scissor, ODT_Bool },
	{ OT_Multisample, ODT_Bool },
	{ OT_AALine, ODT_Bool },
	{ OT_DepthRead, ODT_Bool },
	{ OT_DepthWrite, ODT_Bool },
	{ OT_CompareFunc, ODT_Int },
	{ OT_StencilReadMask, ODT_Int },
	{ OT_StencilWriteMask, ODT_Int },
	{ OT_StencilOpFront, ODT_Complex },
	{ OT_StencilOpBack, ODT_Complex },
	{ OT_PassOp, ODT_Int },
	{ OT_Fail, ODT_Int },
	{ OT_ZFail, ODT_Int },
	{ OT_AlphaToCoverage, ODT_Bool },
	{ OT_IndependantBlend, ODT_Bool },
	{ OT_Target, ODT_Complex },
	{ OT_Index, ODT_Int },
	{ OT_Enabled, ODT_Bool },
	{ OT_Color, ODT_Complex },
	{ OT_Alpha, ODT_Complex },
	{ OT_WriteMask, ODT_Int },
	{ OT_Source, ODT_Int },
	{ OT_Dest, ODT_Int },
	{ OT_Op, ODT_Int },
	{ OT_Identifier, ODT_String },
	{ OT_Code, ODT_Complex },
	{ OT_StencilRef, ODT_Int },
	{ OT_Tags, ODT_Complex },
	{ OT_TagValue, ODT_String },
	{ OT_Variations, ODT_Complex },
	{ OT_Variation, ODT_Complex },
	{ OT_VariationValue, ODT_Int },
	{ OT_Forward, ODT_Bool },
	{ OT_Attributes, ODT_Complex },
	{ OT_AttrName, ODT_String },
	{ OT_VariationOption, ODT_Complex },
	{ OT_AttrShow, ODT_Int },
};

NodeOptions* NodeOptionsCreate(void* context)
{
	static const int kBufferSize = 5;

	NodeOptions* options = (NodeOptions*)Mmalloc(context, sizeof(NodeOptions));
	options->Count = 0;
	options->BufferSize = kBufferSize;

	options->Entries = (NodeOption*)Mmalloc(context, sizeof(NodeOption) * options->BufferSize);
	memset(options->Entries, 0, sizeof(NodeOption) * options->BufferSize);

	return options;
}

void NodeOptionDelete(NodeOption* option)
{
	if (OPTION_LOOKUP[(int)option->Type].DataType == ODT_Complex)
	{
		NodeDelete(option->Value.NodePtr);
		option->Value.NodePtr = 0;
	}
	else if (OPTION_LOOKUP[(int)option->Type].DataType == ODT_String)
	{
		Mmfree((void*)option->Value.StrValue);
		option->Value.StrValue = 0;
	}
}

void NodeOptionsDelete(NodeOptions* options)
{
	int i = 0;

	for (i = 0; i < options->Count; i++)
		NodeOptionDelete(&options->Entries[i]);

	Mmfree(options->Entries);
	Mmfree(options);
}

void NodeOptionsResize(void* context, NodeOptions* options, int size)
{
	NodeOption* originalEntries = options->Entries;
	int originalSize = options->BufferSize;
	int elementsToCopy = originalSize;
	int sizeToCopy = 0;

	options->BufferSize = size;
	if (options->Count > options->BufferSize)
		options->Count = options->BufferSize;

	if (elementsToCopy > size)
		elementsToCopy = size;

	sizeToCopy = elementsToCopy * sizeof(NodeOption);

	options->Entries = (NodeOption*)Mmalloc(context, sizeof(NodeOption) * options->BufferSize);

	memcpy(options->Entries, originalEntries, sizeToCopy);
	memset(options->Entries + elementsToCopy, 0, sizeof(NodeOption) * options->BufferSize - sizeToCopy);

	Mmfree(originalEntries);
}

void NodeOptionsGrowIfNeeded(void* context, NodeOptions* options)
{
	static const int kBufferGrow = 10;

	if (options->Count == options->BufferSize)
		NodeOptionsResize(context, options, options->BufferSize + kBufferGrow);
}

void NodeOptionsAdd(void* context, NodeOptions* options, const NodeOption* option)
{
	NodeOptionsGrowIfNeeded(context, options);

	options->Entries[options->Count] = *option;
	options->Count++;
}

ASTFXNode* NodeCreate(void* context, NodeType type)
{
	ASTFXNode* node = (ASTFXNode*)Mmalloc(context, sizeof(ASTFXNode));
	node->Options = NodeOptionsCreate(context);
	node->Type = type;

	return node;
}

void NodeDelete(ASTFXNode* node)
{
	NodeOptionsDelete(node->Options);

	Mmfree(node);
}

void NodePush(ParseState* parseState, ASTFXNode* node)
{
	NodeLink* linkNode = (NodeLink*)Mmalloc(parseState->MemContext, sizeof(NodeLink));
	linkNode->Next = parseState->NodeStack;
	linkNode->Node = node;

	parseState->NodeStack = linkNode;
	parseState->TopNode = node;
}

void NodePop(ParseState* parseState)
{
	if (!parseState->NodeStack)
		return;

	NodeLink* toRemove = parseState->NodeStack;
	parseState->NodeStack = toRemove->Next;

	if (parseState->NodeStack)
		parseState->TopNode = parseState->NodeStack->Node;
	else
		parseState->TopNode = 0;

	Mmfree(toRemove);
}

void BeginCodeBlock(ParseState* parseState, RawCodeType type)
{
	RawCode* rawCodeBlock = (RawCode*)Mmalloc(parseState->MemContext, sizeof(RawCode));
	rawCodeBlock->Index = parseState->NumRawCodeBlocks[type];
	rawCodeBlock->Size = 0;
	rawCodeBlock->Capacity = 4096;
	rawCodeBlock->Code = Mmalloc(parseState->MemContext, rawCodeBlock->Capacity);
	rawCodeBlock->Next = parseState->RawCodeBlock[type];

	parseState->NumRawCodeBlocks[type]++;
	parseState->RawCodeBlock[type] = rawCodeBlock;

	// Insert defines for code-blocks as we don't perform pre-processing within code blocks but we still want outer defines
	// to be recognized by them (Performing pre-processing for code blocks is problematic because it would require parsing
	// of all the language syntax in order to properly handle macro replacement).
	for (int i = 0; i < parseState->NumDefines; i++)
	{
		const char* define = "#define ";

		AppendCodeBlock(parseState, type, define, (int)strlen(define));
		AppendCodeBlock(parseState, type, parseState->Defines[i].Name, (int)strlen(parseState->Defines[i].Name));

		if (parseState->Defines[i].Expr != 0)
		{
			AppendCodeBlock(parseState, type, " ", 1);
			AppendCodeBlock(parseState, type, parseState->Defines[i].Expr, (int)strlen(parseState->Defines[i].Expr));
		}

		AppendCodeBlock(parseState, type, "\n", 1);
	}
}

void AppendCodeBlock(ParseState* parseState, RawCodeType type, const char* value, int size)
{
	RawCode* rawCode = parseState->RawCodeBlock[type];

	if ((rawCode->Size + size) > rawCode->Capacity)
	{
		int newCapacity = rawCode->Capacity;
		do
		{
			newCapacity *= 2;
		} while ((rawCode->Size + size) > newCapacity);

		char* newBuffer = Mmalloc(parseState->MemContext, newCapacity);
		memcpy(newBuffer, rawCode->Code, rawCode->Size);
		Mmfree(rawCode->Code);

		rawCode->Code = newBuffer;
		rawCode->Capacity = newCapacity;
	}

	memcpy(&rawCode->Code[rawCode->Size], value, size);
	rawCode->Size += size;
}

int GetCodeBlockIndex(ParseState* parseState, RawCodeType type)
{
	return parseState->RawCodeBlock[type]->Index;
}

char* GetCurrentFilename(ParseState* parseState)
{
	if (!parseState->IncludeStack)
		return NULL;

	return parseState->IncludeStack->Data->Filename;
}

void AddDefine(ParseState* parseState, const char* value)
{
	int defineIdx = parseState->NumDefines;
	parseState->NumDefines++;

	if(parseState->NumDefines > parseState->DefineCapacity)
	{
		int newCapacity = parseState->DefineCapacity * 2;
		DefineEntry* newDefines = Mmalloc(parseState->MemContext, newCapacity * sizeof(DefineEntry));

		memcpy(newDefines, parseState->Defines, parseState->DefineCapacity * sizeof(DefineEntry));

		Mmfree(parseState->Defines);
		parseState->Defines = newDefines;
		parseState->DefineCapacity = newCapacity;
	}

	parseState->Defines[defineIdx].Name = MmallocStrdup(parseState->MemContext, value);
	parseState->Defines[defineIdx].Expr = 0;
}

void AddDefineExpr(ParseState* parseState, const char* value)
{
	int defineIdx = parseState->NumDefines - 1;
	if(defineIdx < 0)
	{
		assert(0);
		return;
	}

	parseState->Defines[defineIdx].Expr = MmallocStrdup(parseState->MemContext, value);
}

int HasDefine(ParseState* parseState, const char* value)
{
	for (int i = 0; i < parseState->NumDefines; i++)
	{
		if (strcmp(parseState->Defines[i].Name, value) == 0)
			return 1;
	}

	return 0;
}

int IsDefineEnabled(ParseState* parseState, const char* value)
{
	for (int i = 0; i < parseState->NumDefines; i++)
	{
		if (strcmp(parseState->Defines[i].Name, value) == 0)
		{
			if(parseState->Defines[i].Expr == 0)
				return 0;

			int val = atoi(parseState->Defines[i].Expr);
			return val != 0;
		}
	}

	return 0;
}

void RemoveDefine(ParseState* parseState, const char* value)
{
	for (int i = 0; i < parseState->NumDefines; i++)
	{
		if (strcmp(parseState->Defines[i].Name, value) == 0)
		{
			int remaining = parseState->NumDefines - (i + 1);

			if(remaining > 0)
				memcpy(&parseState->Defines[i], &parseState->Defines[i + 1], remaining * sizeof(DefineEntry));

			parseState->NumDefines--;
		}
	}
}

int PushConditionalDef(ParseState* parseState, int state)
{
	ConditionalData* conditional = Mmalloc(parseState->MemContext, sizeof(ConditionalData));
	conditional->Enabled = state && (parseState->ConditionalStack == 0 || parseState->ConditionalStack->Enabled);
	conditional->SelfEnabled = state;
	conditional->Name = NULL;
	conditional->Op = CO_None;
	conditional->Value = NULL;
	conditional->Next = parseState->ConditionalStack;

	parseState->ConditionalStack = conditional;

	return conditional->Enabled;
}

void PushConditional(ParseState* parseState, const char* name)
{
	ConditionalData* conditional = Mmalloc(parseState->MemContext, sizeof(ConditionalData));
	conditional->Enabled = (parseState->ConditionalStack == 0 || parseState->ConditionalStack->Enabled);
	conditional->SelfEnabled = 0;
	conditional->Op = CO_None;
	conditional->Value = NULL;
	conditional->Name = NULL;
	conditional->Next = parseState->ConditionalStack;

	if(name)
		conditional->Name = MmallocStrdup(parseState->MemContext, name);

	parseState->ConditionalStack = conditional;
}

void SetConditional(ParseState* parseState, const char* name)
{
	assert(parseState->ConditionalStack > 0);

	ConditionalData* conditional = parseState->ConditionalStack;
	ConditionalData* parent = conditional->Next;

	conditional->Name = MmallocStrdup(parseState->MemContext, name);
	conditional->Enabled = (parent == 0 || parent->Enabled);
	conditional->SelfEnabled = 0;
}

void SetConditionalOp(ParseState* parseState, ConditionalOp op)
{
	assert(parseState->ConditionalStack > 0);

	ConditionalData* conditional = parseState->ConditionalStack;
	conditional->Op = op;
}

void SetConditionalVal(ParseState* parseState, const char* value)
{
	assert(parseState->ConditionalStack > 0);

	ConditionalData* conditional = parseState->ConditionalStack;
	conditional->Value = MmallocStrdup(parseState->MemContext, value);
}

int EvalConditional(ParseState* parseState)
{
	assert(parseState->ConditionalStack > 0);

	ConditionalData* conditional = parseState->ConditionalStack;
	if(!conditional->Name)
	{
		conditional->Enabled = 0;
		return 0;
	}

	int myVal = 1;
	if(conditional->Value)
		myVal = atoi(conditional->Value);

	for (int i = 0; i < parseState->NumDefines; i++)
	{
		if (strcmp(parseState->Defines[i].Name, conditional->Name) == 0)
		{
			int val = 0;
			if(parseState->Defines[i].Expr)
				val = atoi(parseState->Defines[i].Expr);

			switch(conditional->Op)
			{
			default:
			case CO_None: conditional->SelfEnabled = val != 0; break;
			case CO_Equals: conditional->SelfEnabled = myVal == val; break;
			case CO_NotEquals: conditional->SelfEnabled = myVal != val; break;
			case CO_Lesser: conditional->SelfEnabled = val < myVal; break;
			case CO_Greater: conditional->SelfEnabled = val > myVal; break;
			case CO_LesserEqual: conditional->SelfEnabled = val <= myVal; break;
			case CO_GreaterEqual: conditional->SelfEnabled = val >= myVal; break;
			}
		}
	}

	conditional->Enabled &= conditional->SelfEnabled;
	return conditional->Enabled;
}

int SetConditionalState(ParseState* parseState, int state)
{
	if (parseState->ConditionalStack == 0)
		return 1;

	ConditionalData* conditional = parseState->ConditionalStack;
	ConditionalData* parent = conditional->Next;

	conditional->Enabled = state && (parent == 0 || parent->Enabled);
	conditional->SelfEnabled = state;

	return conditional->Enabled;
}

int SwitchConditional(ParseState* parseState)
{
	if (parseState->ConditionalStack == 0)
		return 1;

	ConditionalData* conditional = parseState->ConditionalStack;
	return SetConditionalState(parseState, !conditional->SelfEnabled);
}

int PopConditional(ParseState* parseState)
{
	if (parseState->ConditionalStack == 0)
		return 1;

	ConditionalData* conditional = parseState->ConditionalStack;
	parseState->ConditionalStack = conditional->Next;

	if(conditional->Value)
		Mmfree(conditional->Value);

	if(conditional->Name)
		Mmfree(conditional->Name);

	Mmfree(conditional);

	return parseState->ConditionalStack == 0 || parseState->ConditionalStack->Enabled;
}

ParseState* ParseStateCreate()
{
	ParseState* parseState = (ParseState*)malloc(sizeof(ParseState));
	parseState->MemContext = MmallocNewContext();
	parseState->RootNode = NodeCreate(parseState->MemContext, NT_Root);
	parseState->TopNode = 0;
	parseState->NodeStack = 0;
	parseState->IncludeStack = 0;
	parseState->Includes = 0;
	parseState->RawCodeBlock[0] = 0;
	parseState->RawCodeBlock[1] = 0;
	parseState->NumRawCodeBlocks[0] = 0;
	parseState->NumRawCodeBlocks[1] = 0;
	parseState->NumOpenBrackets = 0;

	parseState->HasError = 0;
	parseState->ErrorLine = 0;
	parseState->ErrorColumn = 0;
	parseState->ErrorMessage = 0;
	parseState->ErrorFile = 0;

	parseState->ConditionalStack = 0;
	parseState->DefineCapacity = 10;
	parseState->NumDefines = 0;
	parseState->Defines = Mmalloc(parseState->MemContext, parseState->DefineCapacity * sizeof(DefineEntry));

	NodePush(parseState, parseState->RootNode);

	return parseState;
}

void ParseStateDelete(ParseState* parseState)
{
	while (parseState->NodeStack != 0)
		NodePop(parseState);

	NodeDelete(parseState->RootNode);
	MmallocFreeContext(parseState->MemContext);

	free(parseState);
}
