//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUITexture.h"
#include "B3DGUIUtility.h"
#include "2D/B3DImageSprite.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "Image/B3DSpriteVectorPath.h"
#include "StyleSheet/B3DGUIStyleSheet.h"
#include "Utility/B3DTime.h"

using namespace b3d;

const String& GUITexture::GetGuiTypeName()
{
	static String name = "Texture";
	return name;
}

GUITexture::GUITexture(PrivatelyConstruct, const GUITextureContents& contents, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIInteractable(styleName, dimensions), mScaleMode(contents.ScaleMode), mTransparent(contents.IsTransparent), mUsingStyleTexture(contents.Image == nullptr)
{
	mImageSprite = B3DNew<ImageSprite>();
	mImageSpriteInformation.AnimationStartTime = GetTime().GetRealTimeInSeconds();
	mActiveImage = contents.Image;

	if(mActiveImage.IsLoaded())
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
	GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());

	mActiveImage = image;

	if(mActiveImage.IsLoaded())
	{
		const Size2UI& animationFrameSize = mActiveImage->GetAnimationFrameSize();
		mActiveImageSize = GUILogicalSize(animationFrameSize.Width, animationFrameSize.Height);
	}
	else
		mActiveImageSize = GUILogicalSize::kZero;

	mUsingStyleTexture = false;
	mImageSpriteInformation.AnimationStartTime = GetTime().GetRealTimeInSeconds();

	GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());
	if(originalSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUITexture::UpdateRenderElements()
{
	mRenderElements.clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, GUIElementState::Normal, mBackgroundSprite);

	Size2I textureSize(kZeroTag);
	if(mActiveImage.IsLoaded())
	{
		mImageSpriteInformation.Image = mActiveImage;
		textureSize = mImageSpriteInformation.Image->GetAnimationFrameSize().To<i32>();
	}

	Size2I destinationSize = mAbsoluteSize.To<i32>();

	// ScaleToFit is the only scaling mode that might result in the GUITexture area not being completely covered by
	// the sprite. We need the actual sprite size and offsets to center it.
	Vector2I imageSpriteOffset;
	if(mScaleMode == TextureScaleMode::ScaleToFit)
	{
		if(destinationSize.Width != 0 && destinationSize.Height != 0)
		{
			float aspectX = (float)textureSize.Width / (float)destinationSize.Width;
			float aspectY = (float)textureSize.Height / (float)destinationSize.Height;

			if(aspectY > aspectX)
			{
				destinationSize.Width = Math::RoundToI32((float)textureSize.Width / aspectY);
				destinationSize.Height = Math::RoundToI32((float)textureSize.Height / aspectY);
			}
			else
			{
				destinationSize.Width = Math::RoundToI32((float)textureSize.Width / aspectX);
				destinationSize.Height = Math::RoundToI32((float)textureSize.Height / aspectX);
			}
		}

		imageSpriteOffset = Vector2I(
			((i32)mAbsoluteSize.Width - destinationSize.Width) / 2,
			((i32)mAbsoluteSize.Height - destinationSize.Height) / 2);
	}
	else
		imageSpriteOffset = Vector2I(kZeroTag);

	mImageSpriteInformation.Size = destinationSize;
	mImageSpriteInformation.Transparent = mTransparent;
	mImageSpriteInformation.Color = GetTint();

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		mImageSpriteInformation.Color.A *= styleSheetRules.Opacity;
	}

	if(mScaleMode != TextureScaleMode::ScaleToFit)
		mImageSpriteInformation.UvScale = ImageSprite::GetTextureUvScale(textureSize, destinationSize, mScaleMode);
	else
		mImageSpriteInformation.UvScale = Vector2::kOne;

	mImageSprite->Update(mImageSpriteInformation, (u64)GetParentWidget());

	const Area2 imageSpriteBounds(
		(float)imageSpriteOffset.X, (float)imageSpriteOffset.Y,
		(float)mImageSpriteInformation.Size.Width, (float)mImageSpriteInformation.Size.Height);

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

		mImageSpriteInformation.AnimationStartTime = GetTime().GetRealTimeInSeconds();

		if(mActiveImage.IsLoaded())
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
		if(mActiveImage.IsLoaded())
			optimalSize.Width = mActiveImageSize.Width;
		else
			optimalSize.Width = GetSizeConstraints().MaximumWidth;
	}

	if(GetSizeConstraints().IsHeightFixed())
		optimalSize.Height = GetSizeConstraints().MinimumHeight;
	else
	{
		if(mActiveImage.IsLoaded())
			optimalSize.Height = mActiveImageSize.Height;
		else
			optimalSize.Height = GetSizeConstraints().MaximumHeight;
	}

	return optimalSize;
}
