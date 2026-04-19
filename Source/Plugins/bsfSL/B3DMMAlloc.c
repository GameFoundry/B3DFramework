//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMMAlloc.h"
#include <stdlib.h>
#include <string.h>

typedef struct tagMMAllocHeader MMAllocHeader;

struct tagMMAllocHeader
{
	MMAllocHeader* Next;
	MMAllocHeader* Prev;
};

void* MmallocNewContext()
{
	MMAllocHeader* header = (MMAllocHeader*)malloc(sizeof(MMAllocHeader));
	header->Next = 0;
	header->Prev = 0;

	return header;
}

void MmallocFreeContext(void* context)
{
	MMAllocHeader* header = (MMAllocHeader*)context;
	while (header->Next != 0)
		Mmfree((char*)header->Next + sizeof(MMAllocHeader));

	free(header);
}

void* Mmalloc(void* context, int size)
{
	void* buffer = malloc(size + sizeof(MMAllocHeader));

	MMAllocHeader* header = (MMAllocHeader*)buffer;
	MMAllocHeader* parent = (MMAllocHeader*)context;

	header->Next = parent->Next;
	if (parent->Next)
		parent->Next->Prev = header;

	header->Prev = parent;
	parent->Next = header;

	return (char*)buffer + sizeof(MMAllocHeader);
}

void Mmfree(void* ptr)
{
	void* buffer = (char*)ptr - sizeof(MMAllocHeader);
	MMAllocHeader* header = (MMAllocHeader*)buffer;

	if (header->Prev)
		header->Prev->Next = header->Next;

	if (header->Next)
		header->Next->Prev = header->Prev;

	free(buffer);
}

char* MmallocStrdup(void* context, const char* input)
{
	size_t length = strlen(input);
	char* output = (char*)Mmalloc(context, (int)(sizeof(char) * (length + 1)));

	memcpy(output, input, length);
	output[length] = '\0';

	return output;
}
