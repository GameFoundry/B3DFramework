//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Text/BsFont.h"
#include "Private/RTTI/BsFontRTTI.h"
#include "Text/BsFontManager.h"
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

	Font::~Font()
	{ }

	void Font::initialize(const Vector<SPtr<FontBitmap>>& fontData)
	{
		for(auto iter = fontData.begin(); iter != fontData.end(); ++iter)
			mFontDataPerSize[(*iter)->size] = *iter;

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

	void Font::getResourceDependencies(FrameVector<HResource>& dependencies) const
	{
		for (auto& fontDataEntry : mFontDataPerSize)
		{
			for (auto& texture : fontDataEntry.second->texturePages)
			{
				if (texture != nullptr)
					dependencies.push_back(texture);
			}
		}
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
		SPtr<Font> newFont = _createPtr(fontData);

		return static_resource_cast<Font>(gResources()._createResourceHandle(newFont));
	}

	SPtr<Font> Font::_createPtr(const Vector<SPtr<FontBitmap>>& fontData)
	{
		return FontManager::instance().create(fontData);
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