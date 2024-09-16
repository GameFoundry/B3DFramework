//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITexture.h"
#include "BsGUIHelper.h"
#include "2D/BsImageSprite.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "Image/BsSpriteVectorPath.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

const String& GUITexture::GetGuiTypeName()
{
	static String name = "Texture";
	return name;
}

GUITexture::GUITexture(PrivatelyConstruct, const GUITextureContents& contents, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIInteractable(styleName, dimensions), mScaleMode(contents.ScaleMode), mTransparent(contents.IsTransparent), mUsingStyleTexture(contents.Image == nullptr)
{
	mImageSprite = B3DNew<ImageSprite>();
	mDesc.AnimationStartTime = GetTime().GetRealTimeInSeconds();
	mActiveImage = contents.Image;

	if(SpriteImage::CheckIsLoaded(mActiveImage))
	{
		const Size2UI& animationFrameSize = mActiveImage->GetAnimationFrameSize();
		mActiveImageWidth = animationFrameSize.Width;
		mActiveImageHeight = animationFrameSize.Height;
	}
	else
	{
		mActiveImageWidth = 0;
		mActiveImageHeight = 0;
	}
}

GUITexture::~GUITexture()
{
	B3DDelete(mImageSprite);
}

void GUITexture::SetImage(const HSpriteImage& image)
{
	Vector2I origSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	mActiveImage = image;

	if(SpriteImage::CheckIsLoaded(mActiveImage))
	{
		const Size2UI& animationFrameSize = mActiveImage->GetAnimationFrameSize();
		mActiveImageWidth = animationFrameSize.Width;
		mActiveImageHeight = animationFrameSize.Height;
	}
	else
	{
		mActiveImageWidth = 0;
		mActiveImageHeight = 0;
	}

	mUsingStyleTexture = false;
	mDesc.AnimationStartTime = GetTime().GetRealTimeInSeconds();

	Vector2I newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUITexture::UpdateRenderElements()
{
	mRenderElements.clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, GUIElementState::Normal, mBackgroundSprite);

	Size2UI textureSize(BsZero);
	if(SpriteImage::CheckIsLoaded(mActiveImage))
	{
		mDesc.Image = mActiveImage;
		textureSize = mDesc.Image->GetAnimationFrameSize();
	}
	Vector2I destSize(mLayoutData.Area.Width, mLayoutData.Area.Height);

	// ScaleToFit is the only scaling mode that might result in the GUITexture area not being completely covered by
	// the sprite. We need the actual sprite size and offsets to center it.
	Vector2I imageSpriteOffset;
	if(mScaleMode == TextureScaleMode::ScaleToFit)
	{
		if(destSize.X != 0 && destSize.Y != 0)
		{
			float aspectX = textureSize.Width / (float)destSize.X;
			float aspectY = textureSize.Height / (float)destSize.Y;

			if(aspectY > aspectX)
			{
				destSize.X = Math::RoundToU32(textureSize.Width / aspectY);
				destSize.Y = Math::RoundToU32(textureSize.Height / aspectY);
			}
			else
			{
				destSize.X = Math::RoundToU32(textureSize.Width / aspectX);
				destSize.Y = Math::RoundToU32(textureSize.Height / aspectX);
			}
		}

		imageSpriteOffset = Vector2I(
			((i32)mLayoutData.Area.Width - destSize.X) / 2,
			((i32)mLayoutData.Area.Height - destSize.Y) / 2);
	}
	else
		imageSpriteOffset = Vector2I();

	mDesc.Width = (u32)destSize.X;
	mDesc.Height = (u32)destSize.Y;
	mDesc.Transparent = mTransparent;
	mDesc.Color = GetTint();

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		mDesc.Color.A *= styleSheetRules.Opacity;
	}

	if(mScaleMode != TextureScaleMode::ScaleToFit)
		mDesc.UvScale = ImageSprite::GetTextureUvScale(textureSize, destSize, mScaleMode);
	else
		mDesc.UvScale = Vector2::kOne;

	mImageSprite->Update(mDesc, (u64)GetParentWidget());

	const Rect2 imageSpriteBounds(
		(float)imageSpriteOffset.X, (float)imageSpriteOffset.Y,
		(float)mDesc.Width, (float)mDesc.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(mImageSprite, 0, imageSpriteBounds) }, mRenderElements);
	}

	GUIInteractable::UpdateRenderElements();
}

void GUITexture::NotifyStyleChanged()
{
	if(mUsingStyleTexture)
	{
		if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
			mActiveImage = mStyleSheetRuleInformation.CurrentStateRuleset->Rules.BackgroundImage;

		mDesc.AnimationStartTime = GetTime().GetRealTimeInSeconds();

		if(SpriteImage::CheckIsLoaded(mActiveImage))
		{
			const Size2UI& animationFrameSize = mActiveImage->GetAnimationFrameSize();
			mActiveImageWidth = animationFrameSize.Width;
			mActiveImageHeight = animationFrameSize.Height;
		}
		else
		{
			mActiveImageWidth = 0;
			mActiveImageHeight = 0;
		}
	}
}

Vector2I GUITexture::CalculateUnconstrainedOptimalSize() const
{
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		const Size2UI contentSize = GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(GUIContent(mActiveImage), styleSheetRules, GetSizeConstraints().MaxWidth);
		
		const i32 contentWidth = std::max(0, (i32)contentSize.Width);
		const i32 contentHeight = std::max(0, (i32)contentSize.Height);

		return Vector2I(contentWidth, contentHeight);
	}

	// TODO - Accounting for style dimensions might be redundant here, I'm pretty sure we do that on higher level anyway
	Vector2I optimalSize;

	// Note: We use cached texture size here. This is because we use this method for checking we a layout update is
	// needed (size change is detected). Sprite texture could change without us knowing and by storing the size we can
	// safely detect this. (In short, don't do mActiveTexture->getFrameWidth/Height() here)

	if(GetSizeConstraints().IsWidthFixed())
		optimalSize.X = GetSizeConstraints().MinWidth;
	else
	{
		if(SpriteImage::CheckIsLoaded(mActiveImage))
			optimalSize.X = mActiveImageWidth;
		else
			optimalSize.X = GetSizeConstraints().MaxWidth;
	}

	if(GetSizeConstraints().IsHeightFixed())
		optimalSize.Y = GetSizeConstraints().MinHeight;
	else
	{
		if(SpriteImage::CheckIsLoaded(mActiveImage))
			optimalSize.Y = mActiveImageHeight;
		else
			optimalSize.Y = GetSizeConstraints().MaxHeight;
	}

	return optimalSize;
}
