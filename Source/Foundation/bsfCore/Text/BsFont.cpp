//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFont.h"
#include "Private/RTTI/BsFontRTTI.h"
#include "Resources/BsResources.h"

namespace bs
{
	const CharDesc& FontBitmap::GetCharDesc(UINT32 charId) const
	{
		auto iterFind = characters.find(charId);
		if(iterFind != characters.end())
		{
			return characters.at(charId);
		}

		return missingGlyph;
	}

	RTTITypeBase* FontBitmap::GetRttiStatic()
	{
		return FontBitmapRTTI::Instance();
	}

	RTTITypeBase* FontBitmap::GetRtti() const
	{
		return FontBitmap::GetRttiStatic();
	}

	Font::Font()
		:Resource(false)
	{ }

	void Font::Initialize(const Vector<SPtr<FontBitmap>>& fontData)
	{
		for(auto iter = fontData.begin(); iter != fontData.end(); ++iter)
		{
			mFontDataPerSize[(*iter)->size] = *iter;

			for (auto& texture : (*iter)->texturePages)
			{
				if (texture != nullptr)
					AddResourceDependency(texture);
			}
		}

		Resource::Initialize();
	}

	SPtr<FontBitmap> Font::GetBitmap(UINT32 size) const
	{
		auto iterFind = mFontDataPerSize.find(size);

		if(iterFind == mFontDataPerSize.end())
			return nullptr;

		return iterFind->second;
	}

	INT32 Font::GetClosestSize(UINT32 size) const
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
		SPtr<Font> newFont = CreatePtrInternal(fontData);

		return static_resource_cast<Font>(gResources().CreateResourceHandleInternal(newFont));
	}

	SPtr<Font> Font::CreatePtrInternal(const Vector<SPtr<FontBitmap>>& fontData)
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->SetThisPtrInternal(newFont);
		newFont->Initialize(fontData);

		return newFont;
	}

	SPtr<Font> Font::CreateEmptyInternal()
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->SetThisPtrInternal(newFont);

		return newFont;
	}

	RTTITypeBase* Font::GetRttiStatic()
	{
		return FontRTTI::Instance();
	}

	RTTITypeBase* Font::GetRtti() const
	{
		return Font::GetRttiStatic();
	}
}
