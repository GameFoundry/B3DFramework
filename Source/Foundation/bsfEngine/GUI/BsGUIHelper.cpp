//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIHelper.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUIDimensions.h"
#include "Image/BsTexture.h"
#include "String/BsUnicode.h"

namespace bs
{
	Vector2I GUIHelper::calcOptimalContentsSize(const Vector2I& contentSize, const GUIElementStyle& style, 
		const GUIDimensions& dimensions)
	{
		UINT32 contentWidth = style.margins.left + style.margins.right + style.contentOffset.left + style.contentOffset.right;
		UINT32 contentHeight = style.margins.top + style.margins.bottom + style.contentOffset.top + style.contentOffset.bottom;

		return Vector2I(std::max((UINT32)contentSize.x, contentWidth), std::max((UINT32)contentSize.y, contentHeight));
	}

	Vector2I GUIHelper::calcOptimalContentsSize(const GUIContent& content, const GUIElementStyle& style, 
		const GUIDimensions& dimensions, GUIElementState state)
	{
		Vector2I contentBounds = calcOptimalContentsSize((const String&)content.getText(), style, dimensions);

		const HSpriteTexture& image = content.getImage(state);
		if (SpriteTexture::checkIsLoaded(image))
		{
			contentBounds.x += image->getWidth() + GUIContent::IMAGE_TEXT_SPACING;
			contentBounds.y = std::max(image->getHeight(), (UINT32)contentBounds.y);
		}

		return contentBounds;
	}

	Vector2I GUIHelper::calcOptimalContentsSize(const String& text, const GUIElementStyle& style, const 
		GUIDimensions& dimensions)
	{
		UINT32 wordWrapWidth = 0;

		if(style.wordWrap)
			wordWrapWidth = dimensions.maxWidth;

		UINT32 contentWidth = style.margins.left + style.margins.right + style.contentOffset.left + style.contentOffset.right;
		UINT32 contentHeight = style.margins.top + style.margins.bottom + style.contentOffset.top + style.contentOffset.bottom;

		if(style.font != nullptr && !text.empty())
		{
			bs_frame_mark();

			const U32String utf32text = UTF8::toUTF32(text);
			TextData<FrameAlloc> textData(utf32text, style.font, style.fontSize, wordWrapWidth, 0, style.wordWrap);

			contentWidth += textData.getWidth();
			contentHeight += textData.getNumLines() * textData.getLineHeight(); 

			bs_frame_clear();
		}

		return Vector2I(contentWidth, contentHeight);
	}

	Vector2I GUIHelper::calcTextSize(const String& text, const HFont& font, UINT32 fontSize)
	{
		Vector2I size;
		if (font != nullptr)
		{
			bs_frame_mark();

			const U32String utf32text = UTF8::toUTF32(text);
			TextData<FrameAlloc> textData(utf32text, font, fontSize, 0, 0, false);

			size.x = textData.getWidth();
			size.y = textData.getNumLines() * textData.getLineHeight();

			bs_frame_clear();
		}

		return size;
	}
}
