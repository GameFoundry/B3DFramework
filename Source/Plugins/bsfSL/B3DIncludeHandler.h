//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef __INCLUDEHANDLER_H__
#define __INCLUDEHANDLER_H__

#include "B3DASTFX.h"

char* IncludePush(ParseState* state, const char* filename, int line, int column, int* size);
void IncludePop(ParseState* state);

#endif
