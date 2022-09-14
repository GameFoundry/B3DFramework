//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFont.h"
#include "Private/RTTI/BsFontRTTI.h"
#include "Resources/BsResources.h"

namespace bs
{
	const CharDesc& FontBitmap::getCharDesc(UINT32 charId) const
	{
		auto iterFind = characters.find(charId);
		if(iterFind != characters.end())
		{
			return characters.at(charId);
		}

		return missingGlyph;
	}

	RTTITypeBase* FontBitmap::getRTTIStatic()
	{
		return FontBitmapRTTI::instance();
	}

	RTTITypeBase* FontBitmap::getRTTI() const
	{
		return FontBitmap::getRTTIStatic();
	}

	Font::Font()
		:Resource(false)
	{ }

	void Font::initialize(const Vector<SPtr<FontBitmap>>& fontData)
	{
		for(auto iter = fontData.begin(); iter != fontData.end(); ++iter)
		{
			mFontDataPerSize[(*iter)->size] = *iter;

			for (auto& texture : (*iter)->texturePages)
			{
				if (texture != nullptr)
					addResourceDependency(texture);
			}
		}

		Resource::initialize();
	}

	SPtr<FontBitmap> Font::getBitmap(UINT32 size) const
	{
		auto iterFind = mFontDataPerSize.find(size);

		if(iterFind == mFontDataPerSize.end())
			return nullptr;

		return iterFind->second;
	}

	INT32 Font::getClosestSize(UINT32 size) const
	{
		UINT32 minDiff = std::numeric_limits<UINT32>::max();
		UINT32 bestSize = size;

		for(auto iter = mFontDataPerSize.begin(); iter != mFontDataPerSize.end(); ++iter)
		{
			if(iter->first == size)
				return size;
			else if(iter->first > size)
			{
				UINT32 diff = iter->first - size;
				if(diff < minDiff)
				{
					minDiff = diff;
					bestSize = iter->first;
				}
			}
			else
			{
				UINT32 diff = size - iter->first;
				if(diff < minDiff)
				{
					minDiff = diff;
					bestSize = iter->first;
				}
			}
		}

		return bestSize;
	}

	void Font::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		for (auto& fontDataEntry : mFontDataPerSize)
		{
			for (auto& texture : fontDataEntry.second->texturePages)
			{
				if (texture.isLoaded())
					dependencies.push_back(texture.get());
			}
		}
	}

	HFont Font::create(const Vector<SPtr<FontBitmap>>& fontData)
	{
		SPtr<Font> newFont = CreatePtrInternal(fontData);

		return static_resource_cast<Font>(gResources().CreateResourceHandleInternal(newFont));
	}

	SPtr<Font> Font::CreatePtrInternal(const Vector<SPtr<FontBitmap>>& fontData)
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->SetThisPtrInternal(newFont);
		newFont->initialize(fontData);

		return newFont;
	}

	SPtr<Font> Font::CreateEmptyInternal()
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->SetThisPtrInternal(newFont);

		return newFont;
	}

	RTTITypeBase* Font::getRTTIStatic()
	{
		return FontRTTI::instance();
	}

	RTTITypeBase* Font::getRTTI() const
	{
		return Font::getRTTIStatic();
	}
}
