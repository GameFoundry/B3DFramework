//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPrerequisites.h"
#include "Localization/B3DHEString.h"

using namespace b3d;

const u32 HEString::kEngineStringTableId = 30000; // Arbitrary

HEString::HEString(const String& identifier)
	: mInternal(identifier, kEngineStringTableId)
{
}

HEString::HEString(const String& identifier, const String& defaultString)
	: mInternal(identifier, defaultString, kEngineStringTableId)
{
}

HEString::HEString()
	: mInternal(kEngineStringTableId)
{
}

HEString::operator HString() const
{
	return mInternal;
}
