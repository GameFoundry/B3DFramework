//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsASTFX.h"
#include "BsMMAlloc.h"
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
};

NodeOptions* nodeOptionsCreate(void* context)
{
	static const int BUFFER_SIZE = 5;

	NodeOptions* options = (NodeOptions*)mmalloc(context, sizeof(NodeOptions));
	options->count = 0;
	options->bufferSize = BUFFER_SIZE;

	options->entries = (NodeOption*)mmalloc(context, sizeof(NodeOption) * options->bufferSize);
	memset(options->entries, 0, sizeof(NodeOption) * options->bufferSize);

	return options;
}

void nodeOptionDelete(NodeOption* option)
{
	if (OPTION_LOOKUP[(int)option->type].dataType == ODT_Complex)
	{
		nodeDelete(option->value.nodePtr);
		option->value.nodePtr = 0;
	}
	else if (OPTION_LOOKUP[(int)option->type].dataType == ODT_String)
	{
		mmfree((void*)option->value.strValue);
		option->value.strValue = 0;
	}
}

void nodeOptionsDelete(NodeOptions* options)
{
	int i = 0;

	for (i = 0; i < options->count; i++)
		nodeOptionDelete(&options->entries[i]);

	mmfree(options->entries);
	mmfree(options);
}

void nodeOptionsResize(void* context, NodeOptions* options, int size)
{
	NodeOption* originalEntries = options->entries;
	int originalSize = options->bufferSize;
	int elementsToCopy = originalSize;
	int sizeToCopy = 0;

	options->bufferSize = size;
	if (options->count > options->bufferSize)
		options->count = options->bufferSize;

	if (elementsToCopy > size)
		elementsToCopy = size;

	sizeToCopy = elementsToCopy * sizeof(NodeOption);

	options->entries = (NodeOption*)mmalloc(context, sizeof(NodeOption) * options->bufferSize);

	memcpy(options->entries, originalEntries, sizeToCopy);
	memset(options->entries + elementsToCopy, 0, sizeof(NodeOption) * options->bufferSize - sizeToCopy);

	mmfree(originalEntries);
}

void nodeOptionsGrowIfNeeded(void* context, NodeOptions* options)
{
	static const int BUFFER_GROW = 10;

	if (options->count == options->bufferSize)
		nodeOptionsResize(context, options, options->bufferSize + BUFFER_GROW);
}

void nodeOptionsAdd(void* context, NodeOptions* options, const NodeOption* option)
{
	nodeOptionsGrowIfNeeded(context, options);

	options->entries[options->count] = *option;
	options->count++;
}

ASTFXNode* nodeCreate(void* context, NodeType type)
{
	ASTFXNode* node = (ASTFXNode*)mmalloc(context, sizeof(ASTFXNode));
	node->options = nodeOptionsCreate(context);
	node->type = type;

	return node;
}

void nodeDelete(ASTFXNode* node)
{
	nodeOptionsDelete(node->options);

	mmfree(node);
}

void nodePush(ParseState* parseState, ASTFXNode* node)
{
	NodeLink* linkNode = (NodeLink*)mmalloc(parseState->memContext, sizeof(NodeLink));
	linkNode->next = parseState->nodeStack;
	linkNode->node = node;

	parseState->nodeStack = linkNode;
	parseState->topNode = node;
}

void nodePop(ParseState* parseState)
{
	if (!parseState->nodeStack)
		return;

	NodeLink* toRemove = parseState->nodeStack;
	parseState->nodeStack = toRemove->next;

	if (parseState->nodeStack)
		parseState->topNode = parseState->nodeStack->node;
	else
		parseState->topNode = 0;

	mmfree(toRemove);
}

void beginCodeBlock(ParseState* parseState, RawCodeType type)
{
	RawCode* rawCodeBlock = (RawCode*)mmalloc(parseState->memContext, sizeof(RawCode));
	rawCodeBlock->index = parseState->numRawCodeBlocks[type];
	rawCodeBlock->size = 0;
	rawCodeBlock->capacity = 4096;
	rawCodeBlock->code = mmalloc(parseState->memContext, rawCodeBlock->capacity);
	rawCodeBlock->next = parseState->rawCodeBlock[type];

	parseState->numRawCodeBlocks[type]++;
	parseState->rawCodeBlock[type] = rawCodeBlock;

	// Insert defines for code-blocks as we don't perform pre-processing within code blocks but we still want outer defines
	// to be recognized by them (Performing pre-processing for code blocks is problematic because it would require parsing
	// of all the language syntax in order to properly handle macro replacement).
	for (int i = 0; i < parseState->numDefines; i++)
	{
		const char* define = "#define ";

		appendCodeBlock(parseState, type, define, (int)strlen(define));
		appendCodeBlock(parseState, type, parseState->defines[i].name, (int)strlen(parseState->defines[i].name));

		if (parseState->defines[i].expr != 0)
		{
			appendCodeBlock(parseState, type, " ", 1);
			appendCodeBlock(parseState, type, parseState->defines[i].expr, (int)strlen(parseState->defines[i].expr));
		}

		appendCodeBlock(parseState, type, "\n", 1);
	}
}

void appendCodeBlock(ParseState* parseState, RawCodeType type, const char* value, int size)
{
	RawCode* rawCode = parseState->rawCodeBlock[type];

	if ((rawCode->size + size) > rawCode->capacity)
	{
		int newCapacity = rawCode->capacity;
		do 
		{
			newCapacity *= 2;
		} while ((rawCode->size + size) > newCapacity);

		char* newBuffer = mmalloc(parseState->memContext, newCapacity);
		memcpy(newBuffer, rawCode->code, rawCode->size);
		mmfree(rawCode->code);

		rawCode->code = newBuffer;
		rawCode->capacity = newCapacity;
	}

	memcpy(&rawCode->code[rawCode->size], value, size);
	rawCode->size += size;
}

int getCodeBlockIndex(ParseState* parseState, RawCodeType type)
{
	return parseState->rawCodeBlock[type]->index;
}

char* getCurrentFilename(ParseState* parseState)
{
	if (!parseState->includeStack)
		return NULL;

	return parseState->includeStack->data->filename;
}

void addDefine(ParseState* parseState, const char* value)
{
	int defineIdx = parseState->numDefines;
	parseState->numDefines++;

	if(parseState->numDefines > parseState->defineCapacity)
	{
		int newCapacity = parseState->defineCapacity * 2;
		DefineEntry* newDefines = mmalloc(parseState->memContext, newCapacity * sizeof(DefineEntry));

		memcpy(newDefines, parseState->defines, parseState->defineCapacity * sizeof(DefineEntry));

		mmfree(parseState->defines);
		parseState->defines = newDefines;
		parseState->defineCapacity = newCapacity;
	}

	parseState->defines[defineIdx].name = mmalloc_strdup(parseState->memContext, value);
	parseState->defines[defineIdx].expr = 0;
}

void addDefineExpr(ParseState* parseState, const char* value)
{
	int defineIdx = parseState->numDefines - 1;
	if(defineIdx < 0)
	{
		assert(0);
		return;
	}

	parseState->defines[defineIdx].expr = mmalloc_strdup(parseState->memContext, value);
}

int hasDefine(ParseState* parseState, const char* value)
{
	for (int i = 0; i < parseState->numDefines; i++)
	{
		if (strcmp(parseState->defines[i].name, value) == 0)
			return 1;
	}

	return 0;
}

int isDefineEnabled(ParseState* parseState, const char* value)
{
	for (int i = 0; i < parseState->numDefines; i++)
	{
		if (strcmp(parseState->defines[i].name, value) == 0)
		{
			if(parseState->defines[i].expr == 0)
				return 0;

			int val = atoi(parseState->defines[i].expr);
			return val != 0;
		}
	}

	return 0;
}

void removeDefine(ParseState* parseState, const char* value)
{
	for (int i = 0; i < parseState->numDefines; i++)
	{
		if (strcmp(parseState->defines[i].name, value) == 0)
		{
			int remaining = parseState->numDefines - (i + 1);

			if(remaining > 0)
				memcpy(&parseState->defines[i], &parseState->defines[i + 1], remaining * sizeof(DefineEntry));

			parseState->numDefines--;
		}
	}
}

int pushConditionalDef(ParseState* parseState, int state)
{
	ConditionalData* conditional = mmalloc(parseState->memContext, sizeof(ConditionalData));
	conditional->enabled = state && (parseState->conditionalStack == 0 || parseState->conditionalStack->enabled);
	conditional->selfEnabled = state;
	conditional->name = NULL;
	conditional->op = CO_None;
	conditional->value = NULL;
	conditional->next = parseState->conditionalStack;

	parseState->conditionalStack = conditional;

	return conditional->enabled;
}

void pushConditional(ParseState* parseState, const char* name)
{
	ConditionalData* conditional = mmalloc(parseState->memContext, sizeof(ConditionalData));
	conditional->enabled = (parseState->conditionalStack == 0 || parseState->conditionalStack->enabled);
	conditional->selfEnabled = 0;
	conditional->op = CO_None;
	conditional->value = NULL;
	conditional->name = NULL;
	conditional->next = parseState->conditionalStack;

	if(name)
		conditional->name = mmalloc_strdup(parseState->memContext, name);

	parseState->conditionalStack = conditional;
}

void setConditional(ParseState* parseState, const char* name)
{
	assert(parseState->conditionalStack > 0);

	ConditionalData* conditional = parseState->conditionalStack;
	ConditionalData* parent = conditional->next;

	conditional->name = mmalloc_strdup(parseState->memContext, name);
	conditional->enabled = (parent == 0 || parent->enabled);
	conditional->selfEnabled = 0;
}

void setConditionalOp(ParseState* parseState, ConditionalOp op)
{
	assert(parseState->conditionalStack > 0);

	ConditionalData* conditional = parseState->conditionalStack;
	conditional->op = op;
}

void setConditionalVal(ParseState* parseState, const char* value)
{
	assert(parseState->conditionalStack > 0);

	ConditionalData* conditional = parseState->conditionalStack;
	conditional->value = mmalloc_strdup(parseState->memContext, value);
}

int evalConditional(ParseState* parseState)
{
	assert(parseState->conditionalStack > 0);

	ConditionalData* conditional = parseState->conditionalStack;
	if(!conditional->name)
	{
		conditional->enabled = 0;
		return 0;
	}

	int myVal = 1;
	if(conditional->value)
		myVal = atoi(conditional->value);

	for (int i = 0; i < parseState->numDefines; i++)
	{
		if (strcmp(parseState->defines[i].name, conditional->name) == 0)
		{
			int val = 0;
			if(parseState->defines[i].expr)
				val = atoi(parseState->defines[i].expr);

			switch(conditional->op)
			{
			default:
			case CO_None: conditional->selfEnabled = val != 0; break;
			case CO_Equals: conditional->selfEnabled = myVal == val; break;
			case CO_NotEquals: conditional->selfEnabled = myVal != val; break;
			case CO_Lesser: conditional->selfEnabled = val < myVal; break;
			case CO_Greater: conditional->selfEnabled = val > myVal; break;
			case CO_LesserEqual: conditional->selfEnabled = val <= myVal; break;
			case CO_GreaterEqual: conditional->selfEnabled = val >= myVal; break;
			}
		}
	}

	conditional->enabled &= conditional->selfEnabled;
	return conditional->enabled;
}

int setConditionalState(ParseState* parseState, int state)
{
	if (parseState->conditionalStack == 0)
		return 1;

	ConditionalData* conditional = parseState->conditionalStack;
	ConditionalData* parent = conditional->next;

	conditional->enabled = state && (parent == 0 || parent->enabled);
	conditional->selfEnabled = state;

	return conditional->enabled;
}

int switchConditional(ParseState* parseState)
{
	if (parseState->conditionalStack == 0)
		return 1;

	ConditionalData* conditional = parseState->conditionalStack;
	return setConditionalState(parseState, !conditional->selfEnabled);
}

int popConditional(ParseState* parseState)
{
	if (parseState->conditionalStack == 0)
		return 1;

	ConditionalData* conditional = parseState->conditionalStack;
	parseState->conditionalStack = conditional->next;

	if(conditional->value)
		mmfree(conditional->value);

	if(conditional->name)
		mmfree(conditional->name);

	mmfree(conditional);

	return parseState->conditionalStack == 0 || parseState->conditionalStack->enabled;
}

ParseState* parseStateCreate()
{
	ParseState* parseState = (ParseState*)malloc(sizeof(ParseState));
	parseState->memContext = mmalloc_new_context();
	parseState->rootNode = nodeCreate(parseState->memContext, NT_Root);
	parseState->topNode = 0;
	parseState->nodeStack = 0;
	parseState->includeStack = 0;
	parseState->includes = 0;
	parseState->rawCodeBlock[0] = 0;
	parseState->rawCodeBlock[1] = 0;
	parseState->numRawCodeBlocks[0] = 0;
	parseState->numRawCodeBlocks[1] = 0;
	parseState->numOpenBrackets = 0;

	parseState->hasError = 0;
	parseState->errorLine = 0;
	parseState->errorColumn = 0;
	parseState->errorMessage = 0;
	parseState->errorFile = 0;

	parseState->conditionalStack = 0;
	parseState->defineCapacity = 10;
	parseState->numDefines = 0;
	parseState->defines = mmalloc(parseState->memContext, parseState->defineCapacity * sizeof(DefineEntry));

	nodePush(parseState, parseState->rootNode);

	return parseState;
}

void parseStateDelete(ParseState* parseState)
{
	while (parseState->nodeStack != 0)
		nodePop(parseState);

	nodeDelete(parseState->rootNode);
	mmalloc_free_context(parseState->memContext);

	free(parseState);
}