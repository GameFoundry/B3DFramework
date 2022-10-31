//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFont.h"
#include "Private/RTTI/BsFontRTTI.h"
#include "Resources/BsResources.h"

using namespace bs;

const CharDesc& FontBitmap::GetCharDesc(u32 charId) const
{
	auto iterFind = Characters.find(charId);
	if(iterFind != Characters.end())
	{
		return Characters.at(charId);
	}

	return MissingGlyph;
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
	: Resource(false)
{}

void Font::Initialize(const Vector<SPtr<FontBitmap>>& fontData)
{
	for(auto iter = fontData.begin(); iter != fontData.end(); ++iter)
	{
		mFontDataPerSize[(*iter)->Size] = *iter;

		for(auto& texture : (*iter)->TexturePages)
		{
			if(texture != nullptr)
				AddResourceDependency(texture);
		}
	}

	Resource::Initialize();
}

SPtr<FontBitmap> Font::GetBitmap(u32 size) const
{
	auto iterFind = mFontDataPerSize.find(size);

	if(iterFind == mFontDataPerSize.end())
		return nullptr;

	return iterFind->second;
}

i32 Font::GetClosestSize(u32 size) const
{
	u32 minDiff = std::numeric_limits<u32>::max();
	u32 bestSize = size;

	for(auto iter = mFontDataPerSize.begin(); iter != mFontDataPerSize.end(); ++iter)
	{
		if(iter->first == size)
			return size;
		else if(iter->first > size)
		{
			u32 diff = iter->first - size;
			if(diff < minDiff)
			{
				minDiff = diff;
				bestSize = iter->first;
			}
		}
		else
		{
			u32 diff = size - iter->first;
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
	for(auto& fontDataEntry : mFontDataPerSize)
	{
		for(auto& texture : fontDataEntry.second->TexturePages)
		{
			if(texture.IsLoaded())
				dependencies.push_back(texture.Get());
		}
	}
}

HFont Font::Create(const Vector<SPtr<FontBitmap>>& fontData)
{
	SPtr<Font> newFont = CreatePtrInternal(fontData);

	return static_resource_cast<Font>(GetResources().CreateResourceHandleInternal(newFont));
}

SPtr<Font> Font::CreatePtrInternal(const Vector<SPtr<FontBitmap>>& fontData)
{
	SPtr<Font> newFont = B3DMakeCoreFromExisting<Font>(new(B3DAllocate<Font>()) Font());
	newFont->SetThisPtrInternal(newFont);
	newFont->Initialize(fontData);

	return newFont;
}

SPtr<Font> Font::CreateEmptyInternal()
{
	SPtr<Font> newFont = B3DMakeCoreFromExisting<Font>(new(B3DAllocate<Font>()) Font());
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
