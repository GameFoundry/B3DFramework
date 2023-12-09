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

void GUIBackgroundSprite::BuildRenderElements(const Size2UI& size, const GUIStyleSheetRules& rules, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements, const Vector2I& offset)
{
	// Skip building the sprite if invisible
	if((tint.A * rules.BackgroundColor.A * rules.Opacity) <= 0.0f)
		return;

	mBackgroundSpriteInformation.Width = size.Width;
	mBackgroundSpriteInformation.Height = size.Height;

	if(mBackgroundPathBuilder)
	{
		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.Size = size;
		spriteVectorPathCreateInformation.VectorPath = mBackgroundPathBuilder->BuildPath(spriteVectorPathCreateInformation.Size, rules);

		mBackgroundSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
	}
	else
		mBackgroundSpriteInformation.Image = nullptr;

	mBackgroundSpriteInformation.Color = tint;
	mBackgroundSpriteInformation.Color.A *= rules.Opacity;

	mBackgroundSprite.Update(mBackgroundSpriteInformation, batchId);

	// Calculate content bounds
	const Rect2 backgroundImageBounds(
		(float)offset.X, (float)offset.Y,
		(float)size.Width, (float)size.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(&mBackgroundSprite, 1, backgroundImageBounds) }, outRenderElements );
	}
}

void GUIBackgroundSprite::BuildRenderElements(const Size2UI& size, const GUIElementStyle& style, GUIElementState state, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements, const Vector2I& offset)
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
		T::Append({ T::SpriteInfo(&mBackgroundSprite, 1, backgroundImageBounds) }, outRenderElements );
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
		mContentTextSpriteInformation.Text = content.Text;

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
		mContentTextSpriteInformation.Text = content.Text;
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

void GUISpriteHelper::BuildSpriteRenderElements(GUIElement& element, GUIElementState state, GUIBackgroundSprite& sprite)
{
	const Size2UI size(element.GetLayoutData().Area.Width, element.GetLayoutData().Area.Height);
	const u64 batchId = (u64)element.GetParentWidget();
	const Color& tint = element.GetTint();

	const bool isUsingStyleSheets = element.IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = element.mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		sprite.BuildRenderElements(size, styleSheetRules, tint, batchId, element.mRenderElements);
	}
	else
	{
		sprite.BuildRenderElements(size, *element.GetStyle(), state, tint, batchId, element.mRenderElements);
	}
}

void GUISpriteHelper::BuildSpriteRenderElements(GUIElement& element, GUIElementState state, const GUIContent& content, GUIContentSprites& sprites)
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
