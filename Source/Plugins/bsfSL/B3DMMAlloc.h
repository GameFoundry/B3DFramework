//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef __MMALLOC_H__
#define __MMALLOC_H__

void* MmallocNewContext();
void MmallocFreeContext(void* context);
void* Mmalloc(void* context, int size);
void Mmfree(void* ptr);
char* MmallocStrdup(void* context, const char* input);

#endif
