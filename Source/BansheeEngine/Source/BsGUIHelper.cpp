//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIHelper.h"
#include "BsSpriteTexture.h"
#include "BsGUIElementStyle.h"
#include "BsGUIDimensions.h"
#include "BsTexture.h"

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
		Vector2I contentBounds = calcOptimalContentsSize((const WString&)content.getText(), style, dimensions);

		HSpriteTexture image = content.getImage(state);
		if (image.isLoaded())
		{
			contentBounds.x += image->getWidth() + GUIContent::IMAGE_TEXT_SPACING;
			contentBounds.y = std::max(image->getHeight(), (UINT32)contentBounds.y);
		}

		return contentBounds;
	}

	Vector2I GUIHelper::calcOptimalContentsSize(const WString& text, const GUIElementStyle& style, const 
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

			TextData<FrameAlloc> textData(text, style.font, style.fontSize, wordWrapWidth, 0, style.wordWrap);

			contentWidth += textData.getWidth();
			contentHeight += textData.getNumLines() * textData.getLineHeight(); 

			bs_frame_clear();
		}

		return Vector2I(contentWidth, contentHeight);
	}

	Vector2I GUIHelper::calcTextSize(const WString& text, const HFont& font, UINT32 fontSize)
	{
		Vector2I size;
		if (font != nullptr)
		{
			bs_frame_mark();

			TextData<FrameAlloc> textData(text, font, fontSize, 0, 0, false);

			size.x = textData.getWidth();
			size.y = textData.getNumLines() * textData.getLineHeight();

			bs_frame_clear();
		}

		return size;
	}
}