//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Localization/BsHString.h"
#include "Localization/BsStringTableManager.h"

namespace bs
{
	HString::HString(UINT32 stringTableId)
		:mParameters(nullptr), mIsDirty(true), mStringPtr(nullptr)
	{
		mStringData = StringTableManager::instance().getTable(stringTableId)->getStringData(u8"");

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<String>(mStringData->numParameters);
	}

	HString::HString(const String& identifierString, UINT32 stringTableId)
		:mParameters(nullptr), mIsDirty(true), mStringPtr(nullptr)
	{
		mStringData = StringTableManager::instance().getTable(stringTableId)->getStringData(identifierString);

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<String>(mStringData->numParameters);
	}

	HString::HString(const String& identifierString, const String& defaultString, UINT32 stringTableId)
		:mParameters(nullptr), mIsDirty(true), mStringPtr(nullptr)
	{
		HStringTable table = StringTableManager::instance().getTable(stringTableId);
		table->setString(identifierString, StringTable::DEFAULT_LANGUAGE, defaultString);

		mStringData = table->getStringData(identifierString);

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
		return getValue();
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

	const String& HString::getValue() const
	{
		if (mIsDirty)
		{
			if (mParameters != nullptr)
			{
				mStringData->concatenateString(mCachedString, mParameters, mStringData->numParameters);
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

	void HString::setParameter(UINT32 idx, const String& value)
	{
		if (idx >= mStringData->numParameters)
			return;

		mParameters[idx] = value;
		mIsDirty = true;
	}

	const HString& HString::dummy()
	{
		static HString dummyVal;

		return dummyVal;
	}
}