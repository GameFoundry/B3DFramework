//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFontManager.h"
#include "BsFont.h"

namespace bs
{
	SPtr<Font> FontManager::create(const Vector<SPtr<FontBitmap>>& fontData) const
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->_setThisPtr(newFont);
		newFont->initialize(fontData);

		return newFont;
	}

	SPtr<Font> FontManager::_createEmpty() const
	{
		SPtr<Font> newFont = bs_core_ptr<Font>(new (bs_alloc<Font>()) Font());
		newFont->_setThisPtr(newFont);

		return newFont;
	}
}