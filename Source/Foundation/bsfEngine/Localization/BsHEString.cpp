//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPrerequisites.h"
#include "Localization/BsHEString.h"

using namespace bs;

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
