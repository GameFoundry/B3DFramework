//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSLPrerequisites.h"
#include "Material/BsShaderManager.h"
#include "Material/BsShaderInclude.h"

extern "C" {
#include "BsIncludeHandler.h"
#include "BsMMAlloc.h"
}

using namespace bs;

char* includePush(ParseState* state, const char* filename, int line, int column, int* size)
{
	int filenameQuotesLen = (int)strlen(filename);
	char* filenameNoQuote = (char*)mmalloc(state->MemContext, filenameQuotesLen - 1);
	memcpy(filenameNoQuote, filename + 1, filenameQuotesLen - 2);
	filenameNoQuote[filenameQuotesLen - 2] = '\0';

	HShaderInclude include = ShaderManager::Instance().FindInclude(filenameNoQuote);

	if(include != nullptr)
		include.BlockUntilLoaded();

	int filenameLen = (int)strlen(filenameNoQuote);
	if(include.IsLoaded())
	{
		String includeSource = include->GetString();

		*size = (int)includeSource.size() + 2;
		char* output = (char*)mmalloc(state->MemContext, *size);

		memcpy(output, includeSource.data(), *size - 2);
		output[*size - 2] = 0;
		output[*size - 1] = 0;

		int linkSize = sizeof(IncludeLink) + sizeof(IncludeData) + filenameLen + 1;
		char* linkData = (char*)mmalloc(state->MemContext, linkSize);

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

		mmfree(filenameNoQuote);
		return output;
	}

	const char* errorLabel = "Error opening include file: ";
	int labelLen = (int)strlen(errorLabel);

	int messageLen = filenameLen + labelLen + 1;
	char* message = (char*)mmalloc(state->MemContext, messageLen);

	memcpy(message, errorLabel, labelLen);
	memcpy(message + labelLen, filenameNoQuote, filenameLen);
	message[messageLen - 1] = '\0';

	state->HasError = 1;
	state->ErrorLine = line;
	state->ErrorColumn = column;
	state->ErrorMessage = message;
	state->ErrorFile = getCurrentFilename(state);

	mmfree(filenameNoQuote);
	return nullptr;
}

void includePop(ParseState* state)
{
	IncludeLink* current = state->IncludeStack;

	if(!current)
		return;

	state->IncludeStack = current->Next;
	current->Next = state->Includes;
	state->Includes = current;

	mmfree(current->Data->Buffer);
	current->Data->Buffer = nullptr;
}
