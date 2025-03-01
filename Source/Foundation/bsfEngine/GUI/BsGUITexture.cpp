//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITexture.h"
#include "BsGUIUtility.h"
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
		mActiveImageSize = animationFrameSize.To<GUILogicalUnit>();
	}
	else
		mActiveImageSize = GUILogicalSize::kZero;
}

GUITexture::~GUITexture()
{
	B3DDelete(mImageSprite);
}

void GUITexture::SetImage(const HSpriteImage& image)
{
	GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	mActiveImage = image;

	if(SpriteImage::CheckIsLoaded(mActiveImage))
	{
		const Size2UI& animationFrameSize = mActiveImage->GetAnimationFrameSize();
		mActiveImageSize = GUILogicalSize(animationFrameSize.Width, animationFrameSize.Height);
	}
	else
		mActiveImageSize = GUILogicalSize::kZero;

	mUsingStyleTexture = false;
	mDesc.AnimationStartTime = GetTime().GetRealTimeInSeconds();

	GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
	if(originalSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUITexture::UpdateRenderElements()
{
	mRenderElements.clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, GUIElementState::Normal, mBackgroundSprite);

	Size2I textureSize(BsZero);
	if(SpriteImage::CheckIsLoaded(mActiveImage))
	{
		mDesc.Image = mActiveImage;
		textureSize = mDesc.Image->GetAnimationFrameSize().To<i32>();
	}

	Size2I destSize = mAbsoluteSize.To<i32>();

	// ScaleToFit is the only scaling mode that might result in the GUITexture area not being completely covered by
	// the sprite. We need the actual sprite size and offsets to center it.
	Vector2I imageSpriteOffset;
	if(mScaleMode == TextureScaleMode::ScaleToFit)
	{
		if(destSize.Width != 0 && destSize.Height != 0)
		{
			float aspectX = (float)textureSize.Width / (float)destSize.Width;
			float aspectY = (float)textureSize.Height / (float)destSize.Height;

			if(aspectY > aspectX)
			{
				destSize.Width = Math::RoundToI32((float)textureSize.Width / aspectY);
				destSize.Height = Math::RoundToI32((float)textureSize.Height / aspectY);
			}
			else
			{
				destSize.Width = Math::RoundToI32((float)textureSize.Width / aspectX);
				destSize.Height = Math::RoundToI32((float)textureSize.Height / aspectX);
			}
		}

		imageSpriteOffset = Vector2I(
			((i32)mAbsoluteSize.Width - destSize.Width) / 2,
			((i32)mAbsoluteSize.Height - destSize.Height) / 2);
	}
	else
		imageSpriteOffset = Vector2I(BsZero);

	mDesc.Width = (u32)destSize.Width;
	mDesc.Height = (u32)destSize.Height;
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

	const Area2 imageSpriteBounds(
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
			mActiveImageSize = GUILogicalSize(animationFrameSize.Width, animationFrameSize.Height);
		}
		else
			mActiveImageSize = GUILogicalSize::kZero;
	}
}

GUILogicalSize GUITexture::CalculateUnconstrainedOptimalSize() const
{
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		const GUILogicalSize contentSize = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(GUIContent(mActiveImage), styleSheetRules, GetSizeConstraints().MaximumWidth);
		
		const GUILogicalUnit contentWidth = Math::Max(contentSize.Width, 0);
		const GUILogicalUnit contentHeight = Math::Max(contentSize.Height, 0);

		return GUILogicalSize(contentWidth, contentHeight);
	}

	// TODO - Accounting for style dimensions might be redundant here, I'm pretty sure we do that on higher level anyway
	GUILogicalSize optimalSize;

	// Note: We use cached texture size here. This is because we use this method for checking we a layout update is
	// needed (size change is detected). Sprite texture could change without us knowing and by storing the size we can
	// safely detect this. (In short, don't do mActiveTexture->getFrameWidth/Height() here)

	if(GetSizeConstraints().IsWidthFixed())
		optimalSize.Width = GetSizeConstraints().MinimumWidth;
	else
	{
		if(SpriteImage::CheckIsLoaded(mActiveImage))
			optimalSize.Width = mActiveImageSize.Width;
		else
			optimalSize.Width = GetSizeConstraints().MaximumWidth;
	}

	if(GetSizeConstraints().IsHeightFixed())
		optimalSize.Height = GetSizeConstraints().MinimumHeight;
	else
	{
		if(SpriteImage::CheckIsLoaded(mActiveImage))
			optimalSize.Height = mActiveImageSize.Height;
		else
			optimalSize.Height = GetSizeConstraints().MaximumHeight;
	}

	return optimalSize;
}
