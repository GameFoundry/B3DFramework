//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPrerequisites.h"
#include "Localization/B3DHString.h"
#include "Localization/B3DStringTableManager.h"

using namespace b3d;

HString::HString()
{ }

HString::HString(u32 stringTableId)
{
	mStringData = GetStringTableManager().GetTable(stringTableId)->GetStringData(u8"");

	if(mStringData->NumParameters > 0)
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
}

HString::HString(const String& identifierString, u32 stringTableId)
{
	mStringData = GetStringTableManager().GetTable(stringTableId)->GetStringData(identifierString);

	if(mStringData->NumParameters > 0)
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
}

HString::HString(const String& identifierString, const String& defaultString, u32 stringTableId)
{
	HStringTable table = GetStringTableManager().GetTable(stringTableId);
	table->SetString(identifierString, StringTable::kDefaultLanguage, defaultString);

	mStringData = table->GetStringData(identifierString);

	if(mStringData->NumParameters > 0)
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
}

HString::HString(const HString& copy)
{
	mStringData = copy.mStringData;
	mIsDirty = copy.mIsDirty;
	mCachedString = copy.mCachedString;

	if(copy.mStringData != nullptr && copy.mStringData->NumParameters > 0)
	{
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
		if(copy.mParameters != nullptr)
		{
			for(u32 parameterIndex = 0; parameterIndex < mStringData->NumParameters; parameterIndex++)
				mParameters[parameterIndex] = copy.mParameters[parameterIndex];
		}

		mStringPtr = &mCachedString;
	}
	else
	{
		mParameters = nullptr;

		if(mStringData != nullptr)
			mStringPtr = &mStringData->String;
	}
}

HString::~HString()
{
	if(mParameters != nullptr)
		B3DDeleteMultiple(mParameters, mStringData->NumParameters);
}

HString::operator const String&() const
{
	return GetValue();
}

HString& HString::operator=(const HString& rhs)
{
	if(mParameters != nullptr)
	{
		B3DDeleteMultiple(mParameters, mStringData->NumParameters);
		mParameters = nullptr;
	}

	mStringData = rhs.mStringData;
	mIsDirty = rhs.mIsDirty;
	mCachedString = rhs.mCachedString;

	if(rhs.mStringData != nullptr && rhs.mStringData->NumParameters > 0)
	{
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
		if(rhs.mParameters != nullptr)
		{
			for(u32 parameterIndex = 0; parameterIndex < mStringData->NumParameters; parameterIndex++)
				mParameters[parameterIndex] = rhs.mParameters[parameterIndex];
		}

		mStringPtr = &mCachedString;
	}
	else
	{
		mParameters = nullptr;

		if(mStringData != nullptr)
			mStringPtr = &mStringData->String;
	}

	return *this;
}

const String& HString::GetValue() const
{
	if(mIsDirty)
	{
		if(mParameters != nullptr)
		{
			mStringData->ConcatenateString(mCachedString, mParameters, mStringData->NumParameters);
			mStringPtr = &mCachedString;
		}
		else if(mStringData != nullptr)
		{
			mStringPtr = &mStringData->String;
		}
		else
		{
			mStringPtr = nullptr;
		}

		mIsDirty = false;
	}

	static const String kEmptyString;
	if(mStringPtr != nullptr)
		return *mStringPtr;

	return kEmptyString;
}

void HString::SetParameter(u32 index, const String& value)
{
	if(mStringData == nullptr || index >= mStringData->NumParameters)
		return;

	mParameters[index] = value;
	mIsDirty = true;
}

const HString& HString::Dummy()
{
	static HString dummyVal;

	return dummyVal;
}
