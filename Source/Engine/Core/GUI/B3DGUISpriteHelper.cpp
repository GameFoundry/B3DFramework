//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUISpriteHelper.h"

#include "B3DGUIUtility.h"
#include "B3DGUIVectorPaths.h"
#include "Image/B3DSpriteTexture.h"
#include "2D/B3DTextSprite.h"
#include "Image/B3DSpriteVectorPath.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

GUIBackgroundSprite::GUIBackgroundSprite()
	:mBackgroundPathBuilder(GUIBackgroundVectorPathBuilder::Get())
{
	
}

void GUIBackgroundSprite::BuildRenderElements(const GUIBackgroundSpriteCreateInformation& createInformation, TInlineArray<GUIRenderElement, 4>& outRenderElements)
{
	// Skip building the sprite if invisible
	if((createInformation.Tint.A * createInformation.Rules.Opacity) <= 0.0f)
		return;

	// If possible, create a smaller vector shape and then use scale-9-grid scaling to scale it up to size. This prevents needing to have large
	// rasterized vector shapes often used for widget or window backgrounds.
	const float width = (float)createInformation.Size.Width;
	const float height = (float)createInformation.Size.Height;
	const float minimumExtent = Math::Min(width, height) * 0.5f;

	const float borderTopLeftRadius = Math::Min(minimumExtent, (float)createInformation.Rules.BorderTopLeftRadius);
	const float borderTopRightRadius = Math::Min(minimumExtent, (float)createInformation.Rules.BorderTopRightRadius);
	const float borderBottomLeftRadius = Math::Min(minimumExtent, (float)createInformation.Rules.BorderBottomLeftRadius);
	const float borderBottomRightRadius = Math::Min(minimumExtent, (float)createInformation.Rules.BorderBottomRightRadius);

	const u32 scale9GridBorderLeft = (u32)Math::Max(borderTopLeftRadius, borderBottomLeftRadius) + createInformation.Rules.BorderLeft.GetVisibleWidth();
	const u32 scale9GridBorderRight = (u32)Math::Max(borderTopRightRadius, borderBottomRightRadius) + createInformation.Rules.BorderRight.GetVisibleWidth();
	const u32 scale9GridBorderTop = (u32)Math::Max(borderTopLeftRadius, borderTopRightRadius) + createInformation.Rules.BorderTop.GetVisibleWidth();
	const u32 scale9GridBorderBottom = (u32)Math::Max(borderBottomLeftRadius, borderBottomRightRadius) + createInformation.Rules.BorderBottom.GetVisibleWidth();

	Size2I vectorShapeSize = createInformation.Size.To<i32>();

	// Note: If I support gradients, this also needs to check if using gradients for the center, as we cannot use the approach in that case.
	const bool canUseScale9Grid = (scale9GridBorderLeft + scale9GridBorderRight) < createInformation.Size.Width && (scale9GridBorderBottom + scale9GridBorderTop) < createInformation.Size.Height && mBackgroundPathBuilder == GUIBackgroundVectorPathBuilder::Get();
	if(canUseScale9Grid)
	{
		mBackgroundSpriteInformation.BorderLeft = scale9GridBorderLeft;
		mBackgroundSpriteInformation.BorderRight = scale9GridBorderRight;
		mBackgroundSpriteInformation.BorderTop = scale9GridBorderTop;
		mBackgroundSpriteInformation.BorderBottom = scale9GridBorderBottom;

		vectorShapeSize.Width = (i32)scale9GridBorderLeft + (i32)scale9GridBorderRight + 1;
		vectorShapeSize.Height = (i32)scale9GridBorderTop + (i32)scale9GridBorderBottom + 1;
	}

	mBackgroundSpriteInformation.Size = createInformation.Size.To<i32>();

	if(mBackgroundPathBuilder)
	{
		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.DefaultSize = vectorShapeSize;
		spriteVectorPathCreateInformation.VectorPath = mBackgroundPathBuilder->BuildPath(vectorShapeSize, createInformation.Rules);

		mBackgroundSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
	}
	else
		mBackgroundSpriteInformation.Image = nullptr;

	mBackgroundSpriteInformation.Color = createInformation.Tint;
	mBackgroundSpriteInformation.Color.A *= createInformation.Rules.Opacity;

	mBackgroundSprite.Update(mBackgroundSpriteInformation, createInformation.BatchId);

	// Calculate content bounds
	const Area2 backgroundImageBounds(
		(float)createInformation.Offset.X, (float)createInformation.Offset.Y,
		(float)createInformation.Size.Width, (float)createInformation.Size.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(&mBackgroundSprite, createInformation.Depth, backgroundImageBounds) }, outRenderElements );
	}
}

void GUIBackgroundSprite::SetAnimationStartTime(float time)
{
	mBackgroundSpriteInformation.AnimationStartTime = time;
}

void GUIContentSprites::BuildRenderElements(const GUIContentSpriteCreateInformation& createInformation, TInlineArray<GUIRenderElement, 4>& outRenderElements)
{
	const Area2I contentArea = createInformation.ContentArea;
	const Size2I contentAreaSize = contentArea.GetSize().To<i32>();

	const bool isContentTextAvailable = !createInformation.Content.Text.GetValue().empty();
	if(isContentTextAvailable)
	{
		mContentTextSpriteInformation = BuildTextSpriteInformation(contentArea, createInformation.Content.Text.GetValue(), createInformation.Rules, createInformation.Tint, createInformation.FontScale, createInformation.WordWrap);

		mContentTextSprite.Update(mContentTextSpriteInformation, createInformation.BatchId);
	}

	HSpriteImage contentImage = createInformation.Content.GetImage(GUIElementState::Normal);
	const bool isContentImageAvailable = contentImage.IsLoaded(false);
	if(isContentImageAvailable)
	{
		// Note: We always scale content to fill the available area, without stretching or cropping. We might want to give the user an option on which approach to use,
		// e.g. scale by scale/DPI, stretch to fit, crop to fit
		const Size2I scaledImageSize = CalculateScaledImageSize(contentImage, contentAreaSize);

		mContentImageSpriteInformation.Image = contentImage;
		mContentImageSpriteInformation.Size = scaledImageSize;
		mContentImageSpriteInformation.Color = createInformation.Tint;

		mContentImageSpriteInformation.Color *= createInformation.Rules.Color;
		mContentImageSpriteInformation.Color.A *= createInformation.Rules.Opacity;

		mContentImageSprite.Update(mContentImageSpriteInformation, createInformation.BatchId);
	}

	// Calculate content bounds
	Area2 textBounds;
	Area2 imageBounds;

	Area2I textSpriteBounds = isContentTextAvailable ? mContentTextSprite.GetBounds(Vector2I(BsZero), Area2I()) : Area2I();
	Area2I contentImageSpriteBounds = isContentImageAvailable ? mContentImageSprite.GetBounds(Vector2I(BsZero), Area2I()) : Area2I();

	CalculateContentBounds(contentArea, Size2UI(contentImageSpriteBounds.Width, contentImageSpriteBounds.Height), Size2UI(textSpriteBounds.Width, textSpriteBounds.Height), GUIImagePosition::Left, textBounds, imageBounds);

	const Vector2 textOffset = Vector2(textBounds.X, textBounds.Y) + createInformation.Offset.To<float>();
	const Vector2 imageOffset = Vector2(imageBounds.X, imageBounds.Y) + createInformation.Offset.To<float>();

	if(isContentImageAvailable)
		GUIRenderElementHelper::Append({ GUIRenderElementHelper::SpriteInfo(&mContentImageSprite, createInformation.Depth, imageOffset, imageBounds) }, outRenderElements );

	if(isContentTextAvailable)
		GUIRenderElementHelper::Append({ GUIRenderElementHelper::SpriteInfo(&mContentTextSprite, createInformation.Depth, textOffset, textBounds) }, outRenderElements );
}

TextSpriteInformation GUIContentSprites::BuildTextSpriteInformation(const Area2I& contentArea, const String& text, const GUIStyleSheetRules& rules, const Color& tint, float fontScale, bool wordWrap)
{
	TextSpriteInformation textSpriteInformation;

	textSpriteInformation.Text = text;
	textSpriteInformation.Size = Size2I((i32)contentArea.Width, (i32)contentArea.Height);
	textSpriteInformation.WordWrap = wordWrap;

	textSpriteInformation.InitializeFromStyleSheetRules(rules);
	textSpriteInformation.Color *= tint;
	textSpriteInformation.FontSize *= fontScale;
	
	return textSpriteInformation;
}

void GUIContentSprites::SetAnimationStartTime(float time)
{
	mContentImageSpriteInformation.AnimationStartTime = time;
}

Size2I GUIContentSprites::CalculateScaledImageSize(const HSpriteImage& image, const Size2I& size)
{
	const Size2I& imageSize = image->GetDefaultAllocatedImage().GetSize();
	i32 contentWidth = imageSize.Width;
	i32 contentHeight = imageSize.Height;

	const i32 contentMaxWidth = size.Width;
	const i32 contentMaxHeight = size.Height;

	float horizontalRatio = (float)contentMaxWidth / (float)contentWidth;
	float verticalRatio = (float)contentMaxHeight / (float)contentHeight;

	if(horizontalRatio < verticalRatio)
	{
		contentWidth = Math::RoundToI32(contentWidth * horizontalRatio);
		contentHeight = Math::RoundToI32(contentHeight * horizontalRatio);
	}
	else
	{
		contentWidth = Math::RoundToI32(contentWidth * verticalRatio);
		contentHeight = Math::RoundToI32(contentHeight * verticalRatio);
	}

	return Size2I(contentWidth, contentHeight);
}

void GUIContentSprites::CalculateContentBounds(const Area2I& contentArea, const Size2UI& imageSize, const Size2UI& textSize, GUIImagePosition imagePosition, Area2& outTextBounds, Area2& outImageBounds)
{
	if(imageSize.Width > 0 && imageSize.Height > 0)
	{
		GUILogicalUnit imageXOffset = 0;
		GUILogicalUnit textImageSpacing = 0;

		if(textSize.Width == 0)
		{
			const u32 freeWidth = (u32)std::max(0, (i32)contentArea.Width - (i32)textSize.Width - (i32)imageSize.Width);
			imageXOffset = (i32)(freeWidth / 2);
		}
		else
			textImageSpacing = GUIContent::kImageTextSpacing;

		if(imagePosition == GUIImagePosition::Right)
		{
			const i32 imageReservedWidth = Math::Max(0, (i32)contentArea.Width - (i32)textSize.Width);

			outTextBounds.X = (float)contentArea.X;
			outTextBounds.Width = (float)Math::Max(0, (i32)contentArea.Width - imageReservedWidth);

			outImageBounds.X = (float)(contentArea.X + (i32)textSize.Width + (i32)imageXOffset + (i32)textImageSpacing);
			outImageBounds.Width = (float)Math::Max(0, imageReservedWidth - (i32)imageXOffset);
		}
		else
		{
			const i32 imageReservedWidth = (i32)imageSize.Width + (i32)imageXOffset;

			outImageBounds.X = (float)(contentArea.X + (i32)imageXOffset);
			outImageBounds.Width = (float)Math::Min(imageReservedWidth - (i32)imageXOffset, (i32)contentArea.Width);

			outTextBounds.X = (float)(contentArea.X + imageReservedWidth + (i32)textImageSpacing);
			outTextBounds.Width = (float)Math::Max(0, (i32)contentArea.Width - imageReservedWidth);
		}

		outTextBounds.Y = (float)contentArea.Y;
		outTextBounds.Height = (float)contentArea.Height;

		const float imageYOffset = Math::Max(0, Math::Round(((float)contentArea.Height - (float)imageSize.Height) / 2.0f));
		outImageBounds.Y = (float)contentArea.Y + (float)imageYOffset;
		outImageBounds.Height = (float)contentArea.Height - imageYOffset;
	}
	else
	{
		outImageBounds = Area2();

		outTextBounds.X = (float)contentArea.X;
		outTextBounds.Y = (float)contentArea.Y;
		outTextBounds.Width = (float)contentArea.Width;
		outTextBounds.Height = (float)contentArea.Height;
	}
}

void GUISpriteHelper::BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, GUIBackgroundSprite& sprite, const Vector2I& offset, u32 depth)
{
	const Size2UI size = element.mAbsoluteSize.To<u32>();
	const u64 batchId = (u64)element.GetParentWidget();
	const Color& tint = element.GetTint();

	if(element.mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return;

	const GUIStyleSheetRules& styleSheetRules = element.mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

	GUIBackgroundSpriteCreateInformation backgroundSpriteCreateInformation(size, styleSheetRules, tint, batchId);
	backgroundSpriteCreateInformation.Depth = depth;
	backgroundSpriteCreateInformation.Offset = offset;

	sprite.BuildRenderElements(backgroundSpriteCreateInformation, element.mRenderElements);
}

void GUISpriteHelper::BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, const GUIContent& content, GUIContentSprites& sprites, const Vector2I& offset, u32 depth, bool wordWrap)
{
	const GUILogicalSize& logicalSize = element.GetLayoutData().Size;
	const Size2UI physicalSize = element.mAbsoluteSize.To<u32>();
	const u64 batchId = (u64)element.GetParentWidget();
	const Color& tint = element.GetTint();

	if(element.mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return;

	const GUIStyleSheetRules& styleSheetRules = element.mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const GUILogicalArea logicalContentArea = GUIUtility::RemovePaddingAndBorder(logicalSize, styleSheetRules);
	const GUIPhysicalArea physicalContentArea = GUIUtility::LogicalToPhysical(logicalContentArea, element.GetAbsoluteScale());

	GUIContentSpriteCreateInformation contentSpriteCreateInformation(physicalSize, content, styleSheetRules, tint, element.mAbsoluteScale, batchId);
	contentSpriteCreateInformation.Depth = depth;
	contentSpriteCreateInformation.Offset = offset;
	contentSpriteCreateInformation.ContentArea = physicalContentArea.To<i32, u32>();

	sprites.BuildRenderElements(contentSpriteCreateInformation, element.mRenderElements);
}

TextSpriteInformation GUISpriteHelper::BuildTextSpriteInformation(const GUIInteractable& element, GUIElementState state, const String& text, float fontScale, bool wordWrap)
{
	const GUILogicalSize& logicalSize = element.GetLayoutData().Size;
	const Color& tint = element.GetTint();

	if(element.mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = element.mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		const GUILogicalArea logicalContentArea = GUIUtility::RemovePaddingAndBorder(logicalSize, styleSheetRules);
		const GUIPhysicalArea physicalContentArea = GUIUtility::LogicalToPhysical(logicalContentArea, element.GetAbsoluteScale());

		return GUIContentSprites::BuildTextSpriteInformation(physicalContentArea.To<i32, u32>(), text, styleSheetRules, tint, fontScale, wordWrap);
	}

	return TextSpriteInformation();
}
