//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFont.h"
#include "Private/RTTI/BsFontRTTI.h"
#include "Resources/BsResources.h"

namespace bs
{
	const CharDesc& FontBitmap::GetCharDesc(UINT32 charId) const
	{
		auto iterFind = characters.Find(charId);
		if(iterFind != characters.End())
		{
			return characters.At(charId);
		}

		return missingGlyph;
	}

	RTTITypeBase* FontBitmap::getRTTIStatic()
	{
		return FontBitmapRTTI::Instance();
	}

	RTTITypeBase* FontBitmap::getRTTI() const
	{
		return FontBitmap::GetRTTIStatic();
	}

	Font::Font()
		:Resource(false)
	{ }

	void Font::Initialize(const Vector<SPtr<FontBitmap>>& fontData)
	{
		for(auto iter = fontData.Begin(); iter != fontData.end(); ++iter)
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

	SPtr<FontBitmap> Font::GetBitmap(UINT32 size) const
	{
		auto iterFind = mFontDataPerSize.Find(size);

		if(iterFind == mFontDataPerSize.End())
			return nullptr;

		return iterFind->second;
	}

	INT32 Font::GetClosestSize(UINT32 size) const
	{
		UINT32 minDiff = std::numeric_limits<UINT32>::max();
		UINT32 bestSize = size;

		for(auto iter = mFontDataPerSize.Begin(); iter != mFontDataPerSize.end(); ++iter)
		{
			if(iter->first == size)
				return size;
			else If(iter->first > size)
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

	void Font::GetCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		for (auto& fontDataEntry : mFontDataPerSize)
		{
			for (auto& texture : fontDataEntry.second->texturePages)
			{
				if (texture.IsLoaded())
					dependencies.push_back(texture.Get());
			}
		}
	}

	HFont Font::Create(const Vector<SPtr<FontBitmap>>& fontData)
	{
		SPtr<Font> newFont = _createPtr(fontData);

		return static_resource_cast<Font>(gResources()._createResourceHandle(newFont));
	}

	SPtr<Font> Font::_createPtr(const Vector<SPtr<FontBitmap>>& fontData)
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->_setThisPtr(newFont);
		newFont->Initialize(fontData);

		return newFont;
	}

	SPtr<Font> Font::_createEmpty()
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->_setThisPtr(newFont);

		return newFont;
	}

	RTTITypeBase* Font::getRTTIStatic()
	{
		return FontRTTI::Instance();
	}

	RTTITypeBase* Font::getRTTI() const
	{
		return Font::GetRTTIStatic();
	}
}
