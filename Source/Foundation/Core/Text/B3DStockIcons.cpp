//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/B3DStockIcons.h"
#include "Image/B3DSpriteGlyph.h"
#include "Resources/B3DBuiltinResources.h"

using namespace b3d;

void StockIcons::OnStartUp()
{
	#define B3D_STOCK_ICON(Id, Identifier, Unicode, Label, Font) mNameLookup[Id] = StockIcon::Identifier;
	#include "Text/B3DStockIcons.generated.inc"
	#undef B3D_STOCK_ICON
}

void StockIcons::OnShutDown()
{
	mCache.clear();
}

HSpriteImage StockIcons::GetIcon(StockIcon icon, float size) const
{
	StockIconKey key(icon, size);

	auto found = mCache.find(key);
	if(found != mCache.end())
	{
		HSpriteImage image = found->second.Lock();
		if(image.IsLoaded(false))
			return image;
	}

	HFont font = GetFont(icon);
	if(!B3D_ENSURE(font.IsLoaded(false)))
		return nullptr;

	const u32 unicode = GetUnicode(icon);
	HSpriteGlyph glyph = SpriteGlyph::Create(font, unicode, size);
	mCache[key] = glyph.GetWeak();

	return glyph;
}

HFont StockIcons::GetFont(StockIcon icon) const
{
	switch(icon)
	{
	default:
	case StockIcon::None: return nullptr;
#define B3D_STOCK_ICON(Id, Identifier, Unicode, Label, Font) case StockIcon::Identifier: return GetBuiltinResources().GetFont(Font);
#include "Text/B3DStockIcons.generated.inc"
#undef B3D_STOCK_ICON
	}
}

u32 StockIcons::GetUnicode(StockIcon icon) const
{
	switch(icon)
	{
	default:
	case StockIcon::None: return 0;
#define B3D_STOCK_ICON(Id, Identifier, Unicode, Label, Font) case StockIcon::Identifier: return (Unicode);
#include "Text/B3DStockIcons.generated.inc"
#undef B3D_STOCK_ICON
	}
}

StockIcon StockIcons::ParseIconName(const String& name)
{
	auto found = mNameLookup.find(name);
	if(found != mNameLookup.end())
		return found->second;

	return StockIcon::None;
}



