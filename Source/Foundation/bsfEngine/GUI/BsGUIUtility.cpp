//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIUtility.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUILayout.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "Image/BsTexture.h"
#include "String/BsUnicode.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

Size2UI GUIUtility::CalculateSizeWithPaddingAndBorder(const Size2UI& contentSize, const GUIStyleSheetRules& styleSheetRule)
{
	const u32 paddingWidth = styleSheetRule.Padding.Left + styleSheetRule.Padding.Right;
	const u32 paddingHeight = styleSheetRule.Padding.Top + styleSheetRule.Padding.Bottom;

	const u32 borderWidth = styleSheetRule.BorderLeft.GetVisibleWidth() + styleSheetRule.BorderRight.GetVisibleWidth();
	const u32 borderHeight = styleSheetRule.BorderTop.GetVisibleWidth() + styleSheetRule.BorderBottom.GetVisibleWidth();

	return Size2UI(contentSize.Width + paddingWidth + borderWidth, contentSize.Height + paddingHeight + borderHeight);
}

Size2UI GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth)
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

Size2UI GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth)
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

Rect2I GUIUtility::CalculateContentArea(const Size2UI& layoutSize, const GUIStyleSheetRules& styleSheetRules)
{
	const RectOffset& padding = styleSheetRules.Padding;
	const u32 paddingWidth = padding.Left + padding.Right;
	const u32 paddingHeight = padding.Top + padding.Bottom;

	const u32 borderWidth = styleSheetRules.BorderLeft.GetVisibleWidth() + styleSheetRules.BorderRight.GetVisibleWidth();
	const u32 borderHeight = styleSheetRules.BorderTop.GetVisibleWidth() + styleSheetRules.BorderBottom.GetVisibleWidth();

	Rect2I bounds(0, 0, layoutSize.Width, layoutSize.Height);
	const u32 nonContentWidth = Math::Min(bounds.Width, paddingWidth + borderWidth);
	const u32 nonContentHeight = Math::Min(bounds.Height, paddingHeight + borderHeight);

	bounds.X += (i32)Math::Min(bounds.Width, padding.Left + styleSheetRules.BorderLeft.GetVisibleWidth());
	bounds.Y += (i32)Math::Min(bounds.Height, padding.Top + styleSheetRules.BorderTop.GetVisibleWidth());
	bounds.Width -= nonContentWidth;
	bounds.Height -= nonContentHeight;

	return bounds;
}

Vector2I GUIUtility::CalculateTextBounds(const String& text, const HFont& font, float fontSize)
{
	Vector2I size{BsZero};
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
