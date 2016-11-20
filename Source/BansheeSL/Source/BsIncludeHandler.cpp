#include "BsSLPrerequisites.h"
#include "BsShaderManager.h"
#include "BsShaderInclude.h"

extern "C" {
#include "BsIncludeHandler.h"
#include "BsMMAlloc.h"
}

using namespace bs;

char* includePush(ParseState* state, const char* filename, int line, int column, int* size)
{
	int filenameQuotesLen = (int)strlen(filename);
	char* filenameNoQuote = (char*)mmalloc(state->memContext, filenameQuotesLen - 1);
	memcpy(filenameNoQuote, filename + 1, filenameQuotesLen - 2);
	filenameNoQuote[filenameQuotesLen - 2] = '\0';

	HShaderInclude include = ShaderManager::instance().findInclude(filenameNoQuote);

	if (include != nullptr)
		include.blockUntilLoaded();

	int filenameLen = (int)strlen(filenameNoQuote);
	if (include.isLoaded())
	{
		String includeSource = include->getString();

		*size = (int)includeSource.size() + 2;
		char* output = (char*)mmalloc(state->memContext, *size);

		memcpy(output, includeSource.data(), *size - 2);
		output[*size - 2] = 0;
		output[*size - 1] = 0;

		int linkSize =  sizeof(IncludeLink) + sizeof(IncludeData) + filenameLen + 1;
		char* linkData = (char*)mmalloc(state->memContext, linkSize);

		IncludeLink* newLink = (IncludeLink*)linkData;
		linkData += sizeof(IncludeLink);

		IncludeData* includeData = (IncludeData*)linkData;
		linkData += sizeof(IncludeData);

		memcpy(linkData, filenameNoQuote, filenameLen);
		linkData[filenameLen] = '\0';

		includeData->filename = linkData;
		includeData->buffer = output;

		newLink->data = includeData;
		newLink->next = state->includeStack;

		state->includeStack = newLink;

		mmfree(filenameNoQuote);
		return output;
	}

	const char* errorLabel = "Error opening include file: ";
	int labelLen = (int)strlen(errorLabel);

	int messageLen = filenameLen + labelLen + 1;
	char* message = (char*)mmalloc(state->memContext, messageLen);

	memcpy(message, errorLabel, labelLen);
	memcpy(message + labelLen, filenameNoQuote, filenameLen);
	message[messageLen - 1] = '\0';

	state->hasError = 1;
	state->errorLine = line;
	state->errorColumn = column;
	state->errorMessage = message;
	state->errorFile = getCurrentFilename(state);

	mmfree(filenameNoQuote);
	return nullptr;
}

void includePop(ParseState* state)
{
	IncludeLink* current = state->includeStack;

	if (!current)
		return;

	state->includeStack = current->next;
	current->next = state->includes;
	state->includes = current;

	mmfree(current->data->buffer);
	current->data->buffer = nullptr;
}