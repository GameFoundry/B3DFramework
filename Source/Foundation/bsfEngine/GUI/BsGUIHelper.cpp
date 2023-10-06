//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIHelper.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUIDimensions.h"
#include "Image/BsTexture.h"
#include "String/BsUnicode.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

Vector2I GUIHelper::CalculateOptimalContentSize(const Vector2I& contentSize, const GUIElementStyle& style, const GUIDimensions& dimensions)
{
	u32 contentWidth = style.Margins.Left + style.Margins.Right + style.ContentOffset.Left + style.ContentOffset.Right;
	u32 contentHeight = style.Margins.Top + style.Margins.Bottom + style.ContentOffset.Top + style.ContentOffset.Bottom;

	return Vector2I(std::max((u32)contentSize.X, contentWidth), std::max((u32)contentSize.Y, contentHeight));
}

Vector2I GUIHelper::CalculateOptimalContentSize(const GUIContent& content, const GUIElementStyle& style, const GUIDimensions& dimensions, GUIElementState state)
{
	Vector2I contentBounds = CalculateOptimalContentSize((const String&)content.Text, style, dimensions);

	const HSpriteTexture& image = content.GetImage(state);
	if(SpriteTexture::CheckIsLoaded(image))
	{
		contentBounds.X += image->GetWidth() + GUIContent::kImageTextSpacing;
		contentBounds.Y = std::max(image->GetHeight(), (u32)contentBounds.Y);
	}

	return contentBounds;
}

Vector2I GUIHelper::CalculateOptimalContentSize(const String& text, const GUIElementStyle& style, const GUIDimensions& dimensions)
{
	u32 wordWrapWidth = 0;

	if(style.WordWrap)
		wordWrapWidth = dimensions.MaxWidth;

	u32 contentWidth = style.Margins.Left + style.Margins.Right + style.ContentOffset.Left + style.ContentOffset.Right;
	u32 contentHeight = style.Margins.Top + style.Margins.Bottom + style.ContentOffset.Top + style.ContentOffset.Bottom;

	if(style.Font != nullptr && !text.empty())
	{
		B3DMarkAllocatorFrame();

		const U32String utf32text = UTF8::ToUtF32(text);
		TextData<FrameAllocatorTag> textData(utf32text, style.Font, style.FontSize, wordWrapWidth, 0, style.WordWrap);

		contentWidth += textData.GetWidth();
		contentHeight += textData.GetNumLines() * textData.GetLineHeight();

		B3DClearAllocatorFrame();
	}

	return Vector2I(contentWidth, contentHeight);
}

Size2UI GUIHelper::CalculateSizeWithPadding(const Size2UI& contentSize, const GUIStyleSheetStateRule& style)
{
	const u32 paddingWidth = style.Padding.Left + style.Padding.Right;
	const u32 paddingHeight = style.Padding.Top + style.Padding.Bottom;

	return Size2UI(contentSize.Width + paddingWidth, contentSize.Height + paddingHeight);
}

Size2UI GUIHelper::CalculateOptimalContentSizeWithPadding(const GUIContent& content, const GUIStyleSheetStateRule& style, const GUIDimensions& dimensions)
{
	Size2UI contentBounds = CalculateOptimalContentSizeWithPadding((const String&)content.Text, style, dimensions);

	const HSpriteTexture& image = content.GetImage(GUIElementState::Normal);
	if(SpriteTexture::CheckIsLoaded(image))
	{
		const u32 paddingHeight = style.Padding.Top + style.Padding.Bottom;

		contentBounds.Width += image->GetWidth() + GUIContent::kImageTextSpacing;
		contentBounds.Height = std::max(image->GetHeight() + paddingHeight, (u32)contentBounds.Height);
	}

	return contentBounds;
}

Size2UI GUIHelper::CalculateOptimalContentSizeWithPadding(const String& text, const GUIStyleSheetStateRule& style, const GUIDimensions& dimensions)
{
	u32 wordWrapWidth = 0;

	if(style.WordWrap == GUIWordWrapMode::WrapWord)
		wordWrapWidth = dimensions.MaxWidth;

	u32 contentWidth = style.Padding.Left + style.Padding.Right;
	u32 contentHeight = style.Padding.Top + style.Padding.Bottom;

	const HFont font = style.GetOrLoadFont();
	if(font != nullptr && !text.empty())
	{
		B3DMarkAllocatorFrame();

		const U32String utf32text = UTF8::ToUtF32(text);
		TextData<FrameAllocatorTag> textData(utf32text, font, style.FontSize, wordWrapWidth, 0, style.WordWrap == GUIWordWrapMode::WrapWord);

		contentWidth += textData.GetWidth();
		contentHeight += textData.GetNumLines() * textData.GetLineHeight();

		B3DClearAllocatorFrame();
	}

	return Size2UI(contentWidth, contentHeight);
}

Vector2I GUIHelper::CalculateTextBounds(const String& text, const HFont& font, u32 fontSize)
{
	Vector2I size;
	if(font != nullptr)
	{
		B3DMarkAllocatorFrame();

		const U32String utf32text = UTF8::ToUtF32(text);
		TextData<FrameAllocatorTag> textData(utf32text, font, fontSize, 0, 0, false);

		size.X = textData.GetWidth();
		size.Y = textData.GetNumLines() * textData.GetLineHeight();

		B3DClearAllocatorFrame();
	}

	return size;
}
