//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCorePrerequisites.h"
#include "BsHString.h"
#include "BsStringTableManager.h"

namespace bs
{
	HString::HString(UINT32 stringTableId)
		:mParameters(nullptr), mIsDirty(true), mStringPtr(nullptr)
	{
		mStringData = StringTableManager::instance().getTable(stringTableId)->getStringData(L"");

		if(mStringData->numParameters > 0)
			mParameters = bs_newN<WString>(mStringData->numParameters);
	}

	HString::HString(const WString& identifierString, UINT32 stringTableId)
		:mParameters(nullptr), mIsDirty(true), mStringPtr(nullptr)
	{
		mStringData = StringTableManager::instance().getTable(stringTableId)->getStringData(identifierString);

		if(mStringData->numParameters > 0)
			mParameters = bs_newN<WString>(mStringData->numParameters);
	}

	HString::HString(const WString& identifierString, const WString& defaultString, UINT32 stringTableId)
		:mParameters(nullptr), mIsDirty(true), mStringPtr(nullptr)
	{
		HStringTable table = StringTableManager::instance().getTable(stringTableId);
		table->setString(identifierString, StringTable::DEFAULT_LANGUAGE, defaultString);

		mStringData = table->getStringData(identifierString);

		if (mStringData->numParameters > 0)
			mParameters = bs_newN<WString>(mStringData->numParameters);
	}

	HString::HString(const HString& copy)
	{
		*this = copy;
	}

	HString::~HString()
	{
		if (mParameters != nullptr)
			bs_deleteN(mParameters, mStringData->numParameters);
	}

	HString::operator const WString& () const 
	{ 
		return getValue(); 
	}

	HString& HString::operator=(const HString& rhs)
	{
		mStringData = rhs.mStringData;
		mIsDirty = rhs.mIsDirty;
		mCachedString = rhs.mCachedString;

		if (rhs.mStringData->numParameters > 0)
		{
			mParameters = bs_newN<WString>(mStringData->numParameters);
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

	const WString& HString::getValue() const
	{
		if(mIsDirty)
		{
			if(mParameters != nullptr)
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

	void HString::setParameter(UINT32 idx, const WString& value)
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