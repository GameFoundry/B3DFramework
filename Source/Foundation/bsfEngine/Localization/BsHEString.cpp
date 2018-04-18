//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPrerequisites.h"
#include "Localization/BsHEString.h"

namespace bs
{
	const UINT32 HEString::ENGINE_STRING_TABLE_ID = 30000; // Arbitrary

	HEString::HEString(const String& identifier)
		:mInternal(identifier, ENGINE_STRING_TABLE_ID)
	{
		
	}

	HEString::HEString(const String& identifier, const String& defaultString)
		: mInternal(identifier, defaultString, ENGINE_STRING_TABLE_ID)
	{
		
	}

	HEString::HEString()
		: mInternal(ENGINE_STRING_TABLE_ID)
	{
		
	}

	HEString::operator HString() const
	{
		return mInternal;
	}
}