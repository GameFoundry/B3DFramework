//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Localization/BsHString.h"
#include "Localization/BsStringTableManager.h"

using namespace bs;

HString::HString()
{
	mStringData = StringTableManager::Instance().GetTable(0)->GetStringData(u8"");

	if(mStringData->NumParameters > 0)
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
}

HString::HString(u32 stringTableId)
{
	mStringData = StringTableManager::Instance().GetTable(stringTableId)->GetStringData(u8"");

	if(mStringData->NumParameters > 0)
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
}

HString::HString(const String& identifierString, u32 stringTableId)
{
	mStringData = StringTableManager::Instance().GetTable(stringTableId)->GetStringData(identifierString);

	if(mStringData->NumParameters > 0)
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
}

HString::HString(const String& identifierString, const String& defaultString, u32 stringTableId)
{
	HStringTable table = StringTableManager::Instance().GetTable(stringTableId);
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

	if(copy.mStringData->NumParameters > 0)
	{
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
		if(copy.mParameters != nullptr)
		{
			for(u32 i = 0; i < mStringData->NumParameters; i++)
				mParameters[i] = copy.mParameters[i];
		}

		mStringPtr = &mCachedString;
	}
	else
	{
		mParameters = nullptr;
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

	if(rhs.mStringData->NumParameters > 0)
	{
		mParameters = B3DNewMultiple<String>(mStringData->NumParameters);
		if(rhs.mParameters != nullptr)
		{
			for(u32 i = 0; i < mStringData->NumParameters; i++)
				mParameters[i] = rhs.mParameters[i];
		}

		mStringPtr = &mCachedString;
	}
	else
	{
		mParameters = nullptr;
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
		else
		{
			mStringPtr = &mStringData->String;
		}

		mIsDirty = false;
	}

	return *mStringPtr;
}

void HString::SetParameter(u32 idx, const String& value)
{
	if(idx >= mStringData->NumParameters)
		return;

	mParameters[idx] = value;
	mIsDirty = true;
}

const HString& HString::Dummy()
{
	static HString dummyVal;

	return dummyVal;
}
