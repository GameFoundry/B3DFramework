//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFontImportOptions.h"
#include "BsFontImportOptionsRTTI.h"

namespace bs
{
	FontImportOptions::FontImportOptions()
		:mDPI(96), mRenderMode(FontRenderMode::HintedSmooth), mBold(false), mItalic(false)
	{
		mFontSizes.push_back(10);
		mCharIndexRanges.push_back(std::make_pair(33, 166)); // Most used ASCII characters
	}

	void FontImportOptions::addCharIndexRange(UINT32 from, UINT32 to)
	{
		mCharIndexRanges.push_back(std::make_pair(from, to));
	}

	void FontImportOptions::clearCharIndexRanges()
	{
		mCharIndexRanges.clear();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* FontImportOptions::getRTTIStatic()
	{
		return FontImportOptionsRTTI::instance();
	}

	RTTITypeBase* FontImportOptions::getRTTI() const
	{
		return FontImportOptions::getRTTIStatic();
	}
}