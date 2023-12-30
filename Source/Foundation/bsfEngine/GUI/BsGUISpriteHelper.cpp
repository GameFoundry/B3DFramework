//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISpriteHelper.h"

#include "BsGUIHelper.h"
#include "BsGUIVectorPaths.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsTextSprite.h"
#include "Image/BsSpriteVectorPath.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

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

	Size2UI vectorShapeSize = createInformation.Size;

	// Note: If I support gradients, this also needs to check if using gradients for the center, as we cannot use the approach in that case.
	const bool canUseScale9Grid = (scale9GridBorderLeft + scale9GridBorderRight) < createInformation.Size.Width && (scale9GridBorderBottom + scale9GridBorderTop) < createInformation.Size.Height && mBackgroundPathBuilder == GUIBackgroundVectorPathBuilder::Get();
	if(canUseScale9Grid)
	{
		mBackgroundSpriteInformation.BorderLeft = scale9GridBorderLeft;
		mBackgroundSpriteInformation.BorderRight = scale9GridBorderRight;
		mBackgroundSpriteInformation.BorderTop = scale9GridBorderTop;
		mBackgroundSpriteInformation.BorderBottom = scale9GridBorderBottom;

		vectorShapeSize.Width = scale9GridBorderLeft + scale9GridBorderRight + 1;
		vectorShapeSize.Height = scale9GridBorderTop + scale9GridBorderBottom + 1;
	}

	mBackgroundSpriteInformation.Width = createInformation.Size.Width;
	mBackgroundSpriteInformation.Height = createInformation.Size.Height;

	if(mBackgroundPathBuilder)
	{
		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.Size = vectorShapeSize;
		spriteVectorPathCreateInformation.VectorPath = mBackgroundPathBuilder->BuildPath(vectorShapeSize, createInformation.Rules);

		mBackgroundSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
	}
	else
		mBackgroundSpriteInformation.Image = nullptr;

	mBackgroundSpriteInformation.Color = createInformation.Tint;
	mBackgroundSpriteInformation.Color.A *= createInformation.Rules.Opacity;

	mBackgroundSprite.Update(mBackgroundSpriteInformation, createInformation.BatchId);

	// Calculate content bounds
	const Rect2 backgroundImageBounds(
		(float)createInformation.Offset.X, (float)createInformation.Offset.Y,
		(float)createInformation.Size.Width, (float)createInformation.Size.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(&mBackgroundSprite, createInformation.Depth, backgroundImageBounds) }, outRenderElements );
	}
}

void GUIBackgroundSprite::BuildRenderElements(const Size2UI& size, const GUIElementStyle& style, GUIElementState state, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements, const Vector2I& offset, u32 depth)
{
	mBackgroundSpriteInformation.Width = size.Width;
	mBackgroundSpriteInformation.Height = size.Height;

	const HSpriteImage& activeImage = style.GetImageForState(state);
	if(SpriteImage::CheckIsLoaded(activeImage))
		mBackgroundSpriteInformation.Image = activeImage;
	else
		mBackgroundSpriteInformation.Image = nullptr;

	mBackgroundSpriteInformation.BorderLeft = style.Border.Left;
	mBackgroundSpriteInformation.BorderRight = style.Border.Right;
	mBackgroundSpriteInformation.BorderTop = style.Border.Top;
	mBackgroundSpriteInformation.BorderBottom = style.Border.Bottom;
	mBackgroundSpriteInformation.Color = tint;

	mBackgroundSprite.Update(mBackgroundSpriteInformation, batchId);

	// Calculate content bounds
	const Rect2 backgroundImageBounds(
		(float)offset.X, (float)offset.Y,
		(float)size.Width, (float)size.Height);

	// Populate GUI render elements from the sprite
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(&mBackgroundSprite, depth, backgroundImageBounds) }, outRenderElements );
	}
}

void GUIBackgroundSprite::SetAnimationStartTime(float time)
{
	mBackgroundSpriteInformation.AnimationStartTime = time;
}

void GUIContentSprites::BuildRenderElements(const Size2UI& size, const GUIContent& content, const GUIStyleSheetRules& rules, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements)
{
	const Rect2I contentArea = GUIHelper::CalculateContentArea(size, rules);

	const bool isContentTextAvailable = !content.Text.GetValue().empty();
	if(isContentTextAvailable)
	{
		mContentTextSpriteInformation.Text = (String)content.Text;

		mContentTextSpriteInformation.Width = contentArea.Width;
		mContentTextSpriteInformation.Height = contentArea.Height;

		mContentTextSpriteInformation.InitializeFromStyleSheetRules(rules);
		mContentTextSpriteInformation.Color *= tint;

		mContentTextSprite.Update(mContentTextSpriteInformation, batchId);
	}

	HSpriteImage contentImage = content.GetImage(GUIElementState::Normal);
	const bool isContentImageAvailable = contentImage.IsLoaded(false);
	if(isContentImageAvailable)
	{
		const Size2UI scaledImageSize = CalculateScaledImageSize(contentImage, size);

		mContentImageSpriteInformation.Image = contentImage;
		mContentImageSpriteInformation.Width = scaledImageSize.Width;
		mContentImageSpriteInformation.Height = scaledImageSize.Height;
		mContentImageSpriteInformation.Color = tint;

		mContentImageSpriteInformation.Color *= rules.Color;
		mContentImageSpriteInformation.Color.A *= rules.Opacity;

		mContentImageSprite.Update(mContentImageSpriteInformation, batchId);
	}

	// Calculate content bounds
	Rect2 textBounds;
	Rect2 imageBounds;

	Rect2I textSpriteBounds = isContentTextAvailable ? mContentTextSprite.GetBounds(Vector2I(), Rect2I()) : Rect2I();
	Rect2I contentImageSpriteBounds = isContentImageAvailable ? mContentImageSprite.GetBounds(Vector2I(), Rect2I()) : Rect2I();

	CalculateContentBounds(contentArea, Size2UI(contentImageSpriteBounds.Width, contentImageSpriteBounds.Height), Size2UI(textSpriteBounds.Width, textSpriteBounds.Height), GUIImagePosition::Left, textBounds, imageBounds);

	if(isContentImageAvailable)
		GUIRenderElementHelper::Append({ GUIRenderElementHelper::SpriteInfo(&mContentImageSprite, 0, imageBounds) }, outRenderElements );

	if(isContentTextAvailable)
		GUIRenderElementHelper::Append({ GUIRenderElementHelper::SpriteInfo(&mContentTextSprite, 0, textBounds) }, outRenderElements );
}

void GUIContentSprites::BuildRenderElements(const Size2UI& size, const GUIContent& content, const GUIElementStyle& style, GUIElementState state, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements)
{
	const Rect2I contentArea = GUIHelper::CalculateContentArea(size, style);

	const bool isContentTextAvailable = !content.Text.GetValue().empty();
	if(isContentTextAvailable)
	{
		mContentTextSpriteInformation.Text = (String)content.Text;
		mContentTextSpriteInformation.Font = style.Font;
		mContentTextSpriteInformation.FontSize = style.FontSize;
		mContentTextSpriteInformation.Color = tint * style.GetTextColorForState(state);

		mContentTextSpriteInformation.Width = contentArea.Width;
		mContentTextSpriteInformation.Height = contentArea.Height;
		mContentTextSpriteInformation.HorzAlign = style.TextHorzAlign;
		mContentTextSpriteInformation.VertAlign = style.TextVertAlign;

		mContentTextSprite.Update(mContentTextSpriteInformation, batchId);
	}

	HSpriteImage contentImage = content.GetImage(state);
	const bool isContentImageAvailable = contentImage.IsLoaded(false);
	if(isContentImageAvailable)
	{
		const Size2UI scaledImageSize = CalculateScaledImageSize(contentImage, size);

		mContentImageSpriteInformation.Image = contentImage;
		mContentImageSpriteInformation.Width = scaledImageSize.Width;
		mContentImageSpriteInformation.Height = scaledImageSize.Height;
		mContentImageSpriteInformation.Color = tint;

		mContentImageSprite.Update(mContentImageSpriteInformation, batchId);
	}

	// Calculate content bounds
	Rect2 textBounds;
	Rect2 imageBounds;

	Rect2I textSpriteBounds = isContentTextAvailable ? mContentTextSprite.GetBounds(Vector2I(), Rect2I()) : Rect2I();
	Rect2I contentImageSpriteBounds = isContentImageAvailable ? mContentImageSprite.GetBounds(Vector2I(), Rect2I()) : Rect2I();

	CalculateContentBounds(contentArea, Size2UI(contentImageSpriteBounds.Width, contentImageSpriteBounds.Height), Size2UI(textSpriteBounds.Width, textSpriteBounds.Height), style.ImagePosition, textBounds, imageBounds);

	if(isContentImageAvailable)
		GUIRenderElementHelper::Append({ GUIRenderElementHelper::SpriteInfo(&mContentImageSprite, 0, imageBounds) }, outRenderElements );

	if(isContentTextAvailable)
		GUIRenderElementHelper::Append({ GUIRenderElementHelper::SpriteInfo(&mContentTextSprite, 0, textBounds) }, outRenderElements );
}

void GUIContentSprites::SetAnimationStartTime(float time)
{
	mContentImageSpriteInformation.AnimationStartTime = time;
}

Size2UI GUIContentSprites::CalculateScaledImageSize(const HSpriteImage& image, const Size2UI& size)
{
	const Size2UI& imageSize = image->GetSize();
	u32 contentWidth = imageSize.Width;
	u32 contentHeight = imageSize.Height;

	const u32 contentMaxWidth = Math::Min(size.Width, contentWidth);
	const u32 contentMaxHeight = Math::Min(size.Height, contentHeight);

	float horzRatio = contentMaxWidth / (float)contentWidth;
	float vertRatio = contentMaxHeight / (float)contentHeight;

	if(horzRatio < vertRatio)
	{
		contentWidth = Math::RoundToI32(contentWidth * horzRatio);
		contentHeight = Math::RoundToI32(contentHeight * horzRatio);
	}
	else
	{
		contentWidth = Math::RoundToI32(contentWidth * vertRatio);
		contentHeight = Math::RoundToI32(contentHeight * vertRatio);
	}

	return Size2UI(contentWidth, contentHeight);
}

void GUIContentSprites::CalculateContentBounds(const Rect2I& contentArea, const Size2UI& imageSize, const Size2UI& textSize, GUIImagePosition imagePosition, Rect2& outTextBounds, Rect2& outImageBounds)
{
	if(imageSize.Width > 0 && imageSize.Height > 0)
	{
		i32 imageXOffset = 0;
		i32 textImageSpacing = 0;

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

			outImageBounds.X = (float)(contentArea.X + textSize.Width + imageXOffset + textImageSpacing);
			outImageBounds.Width = (float)Math::Max(0, imageReservedWidth - imageXOffset);
		}
		else
		{
			const i32 imageReservedWidth = (i32)imageSize.Width + imageXOffset;

			outImageBounds.X = (float)(contentArea.X + imageXOffset);
			outImageBounds.Width = (float)Math::Min(imageReservedWidth - imageXOffset, (i32)contentArea.Width);

			outTextBounds.X = (float)(contentArea.X + imageReservedWidth + textImageSpacing);
			outTextBounds.Width = (float)Math::Max(0, (i32)contentArea.Width - imageReservedWidth);
		}

		outTextBounds.Y = (float)contentArea.Y;
		outTextBounds.Height = (float)contentArea.Height;

		const float imageYOffset = Math::Max(0, (float)contentArea.Height - (float)imageSize.Height) / 2.0f;
		outImageBounds.Y = (float)contentArea.Y + (float)imageYOffset;
		outImageBounds.Height = (float)contentArea.Height - imageYOffset;
	}
	else
	{
		outImageBounds = Rect2();

		outTextBounds.X = (float)contentArea.X;
		outTextBounds.Y = (float)contentArea.Y;
		outTextBounds.Width = (float)contentArea.Width;
		outTextBounds.Height = (float)contentArea.Height;
	}
}

void GUISpriteHelper::BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, GUIBackgroundSprite& sprite, u32 depth)
{
	const Size2UI size(element.GetLayoutData().Area.Width, element.GetLayoutData().Area.Height);
	const u64 batchId = (u64)element.GetParentWidget();
	const Color& tint = element.GetTint();

	const bool isUsingStyleSheets = element.IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = element.mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		GUIBackgroundSpriteCreateInformation backgroundSpriteCreateInformation(size, styleSheetRules, tint, batchId);
		backgroundSpriteCreateInformation.Depth = depth;

		sprite.BuildRenderElements(backgroundSpriteCreateInformation, element.mRenderElements);
	}
	else
	{
		sprite.BuildRenderElements(size, *element.GetStyle(), state, tint, batchId, element.mRenderElements, Vector2I::kZero, depth);
	}
}

void GUISpriteHelper::BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, const GUIContent& content, GUIContentSprites& sprites)
{
	const Size2UI size(element.GetLayoutData().Area.Width, element.GetLayoutData().Area.Height);
	const u64 batchId = (u64)element.GetParentWidget();
	const Color& tint = element.GetTint();

	const bool isUsingStyleSheets = element.IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = element.mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		sprites.BuildRenderElements(size, content, styleSheetRules, tint, batchId, element.mRenderElements);
	}
	else
	{
		sprites.BuildRenderElements(size, content, *element.GetStyle(), state, tint, batchId, element.mRenderElements);
	}
}
