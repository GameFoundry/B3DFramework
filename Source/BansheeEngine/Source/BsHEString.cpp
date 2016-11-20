//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisites.h"
#include "BsHEString.h"

namespace bs
{
	const UINT32 HEString::ENGINE_STRING_TABLE_ID = 30000; // Arbitrary

	HEString::HEString(const WString& identifier)
		:mInternal(identifier, ENGINE_STRING_TABLE_ID)
	{
		
	}

	HEString::HEString(const WString& identifier, const WString& defaultString)
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