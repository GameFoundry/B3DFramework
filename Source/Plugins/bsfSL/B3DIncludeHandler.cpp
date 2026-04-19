//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSLPrerequisites.h"
#include "Material/B3DShaderManager.h"
#include "Material/B3DShaderInclude.h"

extern "C" {
#include "B3DIncludeHandler.h"
#include "B3DMMAlloc.h"
}

using namespace b3d;

char* IncludePush(ParseState* state, const char* filename, int line, int column, int* size)
{
	int filenameQuotesLen = (int)strlen(filename);
	char* filenameNoQuote = (char*)Mmalloc(state->MemContext, filenameQuotesLen - 1);
	memcpy(filenameNoQuote, filename + 1, filenameQuotesLen - 2);
	filenameNoQuote[filenameQuotesLen - 2] = '\0';

	TOptional<String> include = ShaderManager::Instance().FindIncludeSource(filenameNoQuote);

	int filenameLen = (int)strlen(filenameNoQuote);
	if(include.has_value())
	{
		const String includeSource = include.value();

		*size = (int)includeSource.size() + 2;
		char* output = (char*)Mmalloc(state->MemContext, *size);

		memcpy(output, includeSource.data(), *size - 2);
		output[*size - 2] = 0;
		output[*size - 1] = 0;

		int linkSize = sizeof(IncludeLink) + sizeof(IncludeData) + filenameLen + 1;
		char* linkData = (char*)Mmalloc(state->MemContext, linkSize);

		IncludeLink* newLink = (IncludeLink*)linkData;
		linkData += sizeof(IncludeLink);

		IncludeData* includeData = (IncludeData*)linkData;
		linkData += sizeof(IncludeData);

		memcpy(linkData, filenameNoQuote, filenameLen);
		linkData[filenameLen] = '\0';

		includeData->Filename = linkData;
		includeData->Buffer = output;

		newLink->Data = includeData;
		newLink->Next = state->IncludeStack;

		state->IncludeStack = newLink;

		Mmfree(filenameNoQuote);
		return output;
	}

	const char* errorLabel = "Error opening include file: ";
	int labelLen = (int)strlen(errorLabel);

	int messageLen = filenameLen + labelLen + 1;
	char* message = (char*)Mmalloc(state->MemContext, messageLen);

	memcpy(message, errorLabel, labelLen);
	memcpy(message + labelLen, filenameNoQuote, filenameLen);
	message[messageLen - 1] = '\0';

	state->HasError = 1;
	state->ErrorLine = line;
	state->ErrorColumn = column;
	state->ErrorMessage = message;
	state->ErrorFile = GetCurrentFilename(state);

	Mmfree(filenameNoQuote);
	return nullptr;
}

void IncludePop(ParseState* state)
{
	IncludeLink* current = state->IncludeStack;

	if(!current)
		return;

	state->IncludeStack = current->Next;
	current->Next = state->Includes;
	state->Includes = current;

	Mmfree(current->Data->Buffer);
	current->Data->Buffer = nullptr;
}
