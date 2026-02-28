//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIUtility.h"
#include "GUI/B3DGUIElement.h"
#include "GUI/B3DGUILayout.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "Image/B3DTexture.h"
#include "String/B3DUnicode.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

GUILogicalSize GUIUtility::CalculateSizeWithPaddingAndBorder(const GUILogicalSize& contentSize, const GUIStyleSheetRules& styleSheetRule)
{
	const GUILogicalUnit paddingWidth = styleSheetRule.Padding.Left + styleSheetRule.Padding.Right;
	const GUILogicalUnit paddingHeight = styleSheetRule.Padding.Top + styleSheetRule.Padding.Bottom;

	const GUILogicalUnit borderWidth = (i32)styleSheetRule.BorderLeft.GetVisibleWidth() + (i32)styleSheetRule.BorderRight.GetVisibleWidth();
	const GUILogicalUnit borderHeight = (i32)styleSheetRule.BorderTop.GetVisibleWidth() + (i32)styleSheetRule.BorderBottom.GetVisibleWidth();

	return GUILogicalSize(contentSize.Width + paddingWidth + borderWidth, contentSize.Height + paddingHeight + borderHeight);
}

GUILogicalSize GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetRules& styleSheetRule, GUILogicalUnit wordWrapWidth)
{
	GUILogicalSize contentBounds = CalculateOptimalContentSizeWithPaddingAndBorder((const String&)content.Text, styleSheetRule, wordWrapWidth);

	const HSpriteImage& image = content.GetImage(GUIElementState::Normal);
	if(image.IsLoaded())
	{
		const GUILogicalUnit paddingHeight = styleSheetRule.Padding.Top + styleSheetRule.Padding.Bottom;
		const GUILogicalUnit borderHeight = (i32)styleSheetRule.BorderTop.GetVisibleWidth() + (i32)styleSheetRule.BorderBottom.GetVisibleWidth();

		const GUILogicalSize& imageSize = image->GetDefaultAllocatedImage().GetSize().To<GUILogicalUnit>();
		contentBounds.Width += imageSize.Width + GUIContent::kImageTextSpacing;
		contentBounds.Height = std::max(imageSize.Height + paddingHeight + borderHeight, contentBounds.Height);
	}

	return contentBounds;
}

GUILogicalSize GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetRules& styleSheetRule, GUILogicalUnit wordWrapWidth)
{
	GUILogicalSize contentSize(kZeroTag);

	if(styleSheetRule.WordWrap != GUIWordWrapMode::WrapWord)
		wordWrapWidth = 0;

	const HFont font = styleSheetRule.Font;
	if(font != nullptr && !text.empty())
	{
		FrameAllocatorScope frameScope;

		const U32String utf32text = UTF8::ToUtF32(text);
		TTextGeometry<FrameAllocatorTag> textData(utf32text, font, styleSheetRule.FontSize, (u32)wordWrapWidth, 0, styleSheetRule.WordWrap == GUIWordWrapMode::WrapWord);

		contentSize.Width += Math::RoundToI32(textData.GetWidth());
		contentSize.Height += Math::RoundToI32((float)textData.GetLineCount() * textData.GetLineHeight());
	}

	return CalculateSizeWithPaddingAndBorder(contentSize, styleSheetRule);
}

GUILogicalArea GUIUtility::RemovePaddingAndBorder(const GUILogicalSize& layoutSize, const GUIStyleSheetRules& styleSheetRules)
{
	const RectOffset& padding = styleSheetRules.Padding;
	const GUILogicalUnit paddingWidth = padding.Left + padding.Right;
	const GUILogicalUnit paddingHeight = padding.Top + padding.Bottom;

	const GUILogicalUnit borderWidth = (i32)styleSheetRules.BorderLeft.GetVisibleWidth() + (i32)styleSheetRules.BorderRight.GetVisibleWidth();
	const GUILogicalUnit borderHeight = (i32)styleSheetRules.BorderTop.GetVisibleWidth() + (i32)styleSheetRules.BorderBottom.GetVisibleWidth();

	GUILogicalArea bounds(0, 0, layoutSize.Width, layoutSize.Height);
	const GUILogicalUnit nonContentWidth = Math::Min(bounds.Width, paddingWidth + borderWidth);
	const GUILogicalUnit nonContentHeight = Math::Min(bounds.Height, paddingHeight + borderHeight);

	bounds.X += Math::Min(bounds.Width, padding.Left + (i32)styleSheetRules.BorderLeft.GetVisibleWidth());
	bounds.Y += Math::Min(bounds.Height, padding.Top + (i32)styleSheetRules.BorderTop.GetVisibleWidth());
	bounds.Width -= nonContentWidth;
	bounds.Height -= nonContentHeight;

	return bounds;
}

Size2I GUIUtility::CalculateTextBounds(const String& text, const HFont& font, float fontSize)
{
	Size2I size{kZeroTag};
	if(font != nullptr)
	{
		FrameAllocatorScope frameScope;

		const U32String utf32text = UTF8::ToUtF32(text);
		TTextGeometry<FrameAllocatorTag> textData(utf32text, font, fontSize, 0, 0, false);

		size.Width = Math::RoundToI32(textData.GetWidth());
		size.Height = Math::RoundToI32((float)textData.GetLineCount() * textData.GetLineHeight());
	}

	return size;
}
