//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIHelper.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUISizeConstraints.h"
#include "Image/BsTexture.h"
#include "String/BsUnicode.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

Vector2I GUIHelper::CalculateOptimalContentSize(const Vector2I& contentSize, const GUIElementStyle& style, const GUISizeConstraints& dimensions)
{
	u32 contentWidth = style.Margins.Left + style.Margins.Right + style.ContentOffset.Left + style.ContentOffset.Right;
	u32 contentHeight = style.Margins.Top + style.Margins.Bottom + style.ContentOffset.Top + style.ContentOffset.Bottom;

	return Vector2I(std::max((u32)contentSize.X, contentWidth), std::max((u32)contentSize.Y, contentHeight));
}

Vector2I GUIHelper::CalculateOptimalContentSize(const GUIContent& content, const GUIElementStyle& style, const GUISizeConstraints& dimensions, GUIElementState state)
{
	Vector2I contentBounds = CalculateOptimalContentSize((const String&)content.Text, style, dimensions);

	const HSpriteImage& image = content.GetImage(state);
	if(SpriteImage::CheckIsLoaded(image))
	{
		const Size2UI& imageSize = image->GetSize();

		contentBounds.X += (i32)(imageSize.Width + GUIContent::kImageTextSpacing);
		contentBounds.Y = (i32)std::max(imageSize.Height, (u32)contentBounds.Y);
	}

	return contentBounds;
}

Vector2I GUIHelper::CalculateOptimalContentSize(const String& text, const GUIElementStyle& style, const GUISizeConstraints& dimensions)
{
	u32 wordWrapWidth = 0;

	if(style.WordWrap)
		wordWrapWidth = dimensions.MaxWidth;

	i32 contentWidth = style.Margins.Left + style.Margins.Right + style.ContentOffset.Left + style.ContentOffset.Right;
	i32 contentHeight = style.Margins.Top + style.Margins.Bottom + style.ContentOffset.Top + style.ContentOffset.Bottom;

	if(style.Font != nullptr && !text.empty())
	{
		B3DMarkAllocatorFrame();

		const U32String utf32text = UTF8::ToUtF32(text);
		TTextGeometry<FrameAllocatorTag> textData(utf32text, style.Font, style.FontSize, wordWrapWidth, 0, style.WordWrap);

		contentWidth += Math::RoundToI32(textData.GetWidth());
		contentHeight += Math::RoundToI32((float)textData.GetLineCount() * textData.GetLineHeight());

		B3DClearAllocatorFrame();
	}

	return Vector2I(contentWidth, contentHeight);
}

Size2UI GUIHelper::CalculateSizeWithPaddingAndBorder(const Size2UI& contentSize, const GUIStyleSheetRules& styleSheetRule)
{
	const u32 paddingWidth = styleSheetRule.Padding.Left + styleSheetRule.Padding.Right;
	const u32 paddingHeight = styleSheetRule.Padding.Top + styleSheetRule.Padding.Bottom;

	const u32 borderWidth = styleSheetRule.BorderLeft.GetVisibleWidth() + styleSheetRule.BorderRight.GetVisibleWidth();
	const u32 borderHeight = styleSheetRule.BorderTop.GetVisibleWidth() + styleSheetRule.BorderBottom.GetVisibleWidth();

	return Size2UI(contentSize.Width + paddingWidth + borderWidth, contentSize.Height + paddingHeight + borderHeight);
}

Size2UI GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth)
{
	Size2UI contentBounds = CalculateOptimalContentSizeWithPaddingAndBorder((const String&)content.Text, styleSheetRule, wordWrapWidth);

	const HSpriteImage& image = content.GetImage(GUIElementState::Normal);
	if(SpriteImage::CheckIsLoaded(image))
	{
		const u32 paddingHeight = styleSheetRule.Padding.Top + styleSheetRule.Padding.Bottom;
		const u32 borderHeight = styleSheetRule.BorderTop.GetVisibleWidth() + styleSheetRule.BorderBottom.GetVisibleWidth();

		const Size2UI& imageSize = image->GetSize();
		contentBounds.Width += imageSize.Width + GUIContent::kImageTextSpacing;
		contentBounds.Height = std::max(imageSize.Height + paddingHeight + borderHeight, contentBounds.Height);
	}

	return contentBounds;
}

Size2UI GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth)
{
	Size2UI contentSize(BsZero);

	if(styleSheetRule.WordWrap != GUIWordWrapMode::WrapWord)
		wordWrapWidth = 0;

	const HFont font = styleSheetRule.Font;
	if(font != nullptr && !text.empty())
	{
		B3DMarkAllocatorFrame();

		const U32String utf32text = UTF8::ToUtF32(text);
		TTextGeometry<FrameAllocatorTag> textData(utf32text, font, styleSheetRule.FontSize, wordWrapWidth, 0, styleSheetRule.WordWrap == GUIWordWrapMode::WrapWord);

		contentSize.Width += Math::RoundToU32(textData.GetWidth());
		contentSize.Height += Math::RoundToU32((float)textData.GetLineCount() * textData.GetLineHeight());

		B3DClearAllocatorFrame();
	}

	return CalculateSizeWithPaddingAndBorder(contentSize, styleSheetRule);
}

Vector2I GUIHelper::CalculateTextBounds(const String& text, const HFont& font, float fontSize)
{
	Vector2I size;
	if(font != nullptr)
	{
		B3DMarkAllocatorFrame();

		const U32String utf32text = UTF8::ToUtF32(text);
		TTextGeometry<FrameAllocatorTag> textData(utf32text, font, fontSize, 0, 0, false);

		size.X = Math::RoundToI32(textData.GetWidth());
		size.Y = Math::RoundToI32((float)textData.GetLineCount() * textData.GetLineHeight());

		B3DClearAllocatorFrame();
	}

	return size;
}
