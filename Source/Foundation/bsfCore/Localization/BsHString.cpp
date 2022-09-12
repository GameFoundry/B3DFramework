//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Localization/BsHString.h"
#include "Localization/BsStringTableManager.h"

namespace bs
{
	HString::HString()
	{
		mStringData = StringTableManager::instance().GetTable(0)->GetStringData(u8"");

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<String>(mStringData->numParameters);
	}

	HString::HString(UINT32 stringTableId)
	{
		mStringData = StringTableManager::instance().GetTable(stringTableId)->GetStringData(u8"");

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<String>(mStringData->numParameters);
	}

	HString::HString(const String& identifierString, UINT32 stringTableId)
	{
		mStringData = StringTableManager::instance().GetTable(stringTableId)->GetStringData(identifierString);

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<String>(mStringData->numParameters);
	}

	HString::HString(const String& identifierString, const String& defaultString, UINT32 stringTableId)
	{
		HStringTable table = StringTableManager::instance().GetTable(stringTableId);
		table->SetString(identifierString, StringTable::DEFAULT_LANGUAGE, defaultString);

		mStringData = table->GetStringData(identifierString);

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<String>(mStringData->numParameters);
	}

	HString::HString(const HString& copy)
	{
		mStringData = copy.mStringData;
		mIsDirty = copy.mIsDirty;
		mCachedString = copy.mCachedString;

		if (copy.mStringData->numParameters > 0)
		{
			mParameters = bs_newN<String>(mStringData->numParameters);
			if (copy.mParameters != nullptr)
			{
				for (UINT32 i = 0; i < mStringData->numParameters; i++)
					mParameters[i] = copy.mParameters[i];
			}

			mStringPtr = &mCachedString;
		}
		else
		{
			mParameters = nullptr;
			mStringPtr = &mStringData->string;
		}
	}

	HString::~HString()
	{
		if (mParameters != nullptr)
			bs_deleteN(mParameters, mStringData->numParameters);
	}

	HString::operator const String& () const
	{
		return GetValue();
	}

	HString& HString::operator=(const HString& rhs)
	{
		if (mParameters != nullptr)
		{
			bs_deleteN(mParameters, mStringData->numParameters);
			mParameters = nullptr;
		}

		mStringData = rhs.mStringData;
		mIsDirty = rhs.mIsDirty;
		mCachedString = rhs.mCachedString;

		if (rhs.mStringData->numParameters > 0)
		{
			mParameters = bs_newN<String>(mStringData->numParameters);
			if (rhs.mParameters != nullptr)
			{
				for (UINT32 i = 0; i < mStringData->numParameters; i++)
					mParameters[i] = rhs.mParameters[i];
			}

			mStringPtr = &mCachedString;
		}
		else
		{
			mParameters = nullptr;
			mStringPtr = &mStringData->string;
		}

		return *this;
	}

	const String& HString::GetValue() const
	{
		if (mIsDirty)
		{
			if (mParameters != nullptr)
			{
				mStringData->ConcatenateString(mCachedString, mParameters, mStringData->numParameters);
				mStringPtr = &mCachedString;
			}
			else
			{
				mStringPtr = &mStringData->string;
			}

			mIsDirty = false;
		}

		return *mStringPtr;
	}

	void HString::SetParameter(UINT32 idx, const String& value)
	{
		if (idx >= mStringData->numParameters)
			return;

		mParameters[idx] = value;
		mIsDirty = true;
	}

	const HString& HString::Dummy()
	{
		static HString dummyVal;

		return dummyVal;
	}
}
