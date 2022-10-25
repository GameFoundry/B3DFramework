//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITexture.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"

using namespace bs;

const String& GUITexture::GetGuiTypeName()
{
	static String name = "Texture";
	return name;
}

GUITexture::GUITexture(const String& styleName, const HSpriteTexture& texture, TextureScaleMode scale, bool transparent, const GUIDimensions& dimensions)
	: GUIElement(styleName, dimensions), mScaleMode(scale), mTransparent(transparent), mUsingStyleTexture(false)
{
	mImageSprite = bs_new<ImageSprite>();
	mDesc.AnimationStartTime = gTime().GetTime();

	if(texture != nullptr)
	{
		mActiveTexture = texture;
		mUsingStyleTexture = false;
	}
	else
	{
		mActiveTexture = GetStyleInternal()->Normal.Texture;
		mUsingStyleTexture = true;
	}

	bool isTexLoaded = SpriteTexture::CheckIsLoaded(mActiveTexture);
	mActiveTextureWidth = isTexLoaded ? mActiveTexture->GetFrameWidth() : 0;
	mActiveTextureHeight = isTexLoaded ? mActiveTexture->GetFrameHeight() : 0;
}

GUITexture::~GUITexture()
{
	bs_delete(mImageSprite);
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent, const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, transparent, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, transparent, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, true, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, TextureScaleMode::StretchToFit, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, TextureScaleMode::StretchToFit, true, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(TextureScaleMode scale, const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), scale, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(TextureScaleMode scale, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), scale, true, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), TextureScaleMode::StretchToFit, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const String& styleName)
{
	return new(bs_alloc<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), TextureScaleMode::StretchToFit, true, GUIDimensions::Create());
}

void GUITexture::SetTexture(const HSpriteTexture& texture)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

	mActiveTexture = texture;

	bool isTexLoaded = SpriteTexture::CheckIsLoaded(mActiveTexture);
	mActiveTextureWidth = isTexLoaded ? mActiveTexture->GetFrameWidth() : 0;
	mActiveTextureHeight = isTexLoaded ? mActiveTexture->GetFrameHeight() : 0;

	mUsingStyleTexture = false;
	mDesc.AnimationStartTime = gTime().GetTime();

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirtyInternal();
	else
		MarkContentAsDirtyInternal();
}

void GUITexture::UpdateRenderElementsInternal()
{
	Vector2I textureSize;
	if(SpriteTexture::CheckIsLoaded(mActiveTexture))
	{
		mDesc.Texture = mActiveTexture;
		textureSize.X = mDesc.Texture->GetFrameWidth();
		textureSize.Y = mDesc.Texture->GetFrameHeight();
	}
	Vector2I destSize(mLayoutData.Area.Width, mLayoutData.Area.Height);

	// ScaleToFit is the only scaling mode that might result in the GUITexture area not being completely covered by
	// the sprite. We need the actual sprite size and offsets to center it.
	if(mScaleMode == TextureScaleMode::ScaleToFit)
	{
		if(destSize.X != 0 && destSize.Y != 0)
		{
			float aspectX = textureSize.X / (float)destSize.X;
			float aspectY = textureSize.Y / (float)destSize.Y;

			if(aspectY > aspectX)
			{
				destSize.X = Math::RoundToPosInt(textureSize.X / aspectY);
				destSize.Y = Math::RoundToPosInt(textureSize.Y / aspectY);
			}
			else
			{
				destSize.X = Math::RoundToPosInt(textureSize.X / aspectX);
				destSize.Y = Math::RoundToPosInt(textureSize.Y / aspectX);
			}
		}

		mImageSpriteOffset = Vector2I(
			((i32)mLayoutData.Area.Width - destSize.X) / 2,
			((i32)mLayoutData.Area.Height - destSize.Y) / 2);
	}
	else
		mImageSpriteOffset = Vector2I();

	mDesc.Width = (u32)destSize.X;
	mDesc.Height = (u32)destSize.Y;

	mDesc.BorderLeft = GetStyleInternal()->Border.Left;
	mDesc.BorderRight = GetStyleInternal()->Border.Right;
	mDesc.BorderTop = GetStyleInternal()->Border.Top;
	mDesc.BorderBottom = GetStyleInternal()->Border.Bottom;
	mDesc.Transparent = mTransparent;
	mDesc.Color = GetTint();

	if(mScaleMode != TextureScaleMode::ScaleToFit)
		mDesc.UvScale = ImageSprite::GetTextureUvScale(textureSize, destSize, mScaleMode);
	else
		mDesc.UvScale = Vector2::ONE;

	mImageSprite->Update(mDesc, (u64)GetParentWidgetInternal());

	// Populate GUI render elements from the sprites
	{
		using T = impl::GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
	}

	GUIElement::UpdateRenderElementsInternal();
}

void GUITexture::StyleUpdated()
{
	if(mUsingStyleTexture)
	{
		mActiveTexture = GetStyleInternal()->Normal.Texture;
		mDesc.AnimationStartTime = gTime().GetTime();

		bool isTexLoaded = SpriteTexture::CheckIsLoaded(mActiveTexture);
		mActiveTextureWidth = isTexLoaded ? mActiveTexture->GetFrameWidth() : 0;
		mActiveTextureHeight = isTexLoaded ? mActiveTexture->GetFrameHeight() : 0;
	}
}

Vector2I GUITexture::GetOptimalSizeInternal() const
{
	// TODO - Accounting for style dimensions might be redundant here, I'm pretty sure we do that on higher level anyway
	Vector2I optimalSize;

	// Note: We use cached texture size here. This is because we use this method for checking we a layout update is
	// needed (size change is detected). Sprite texture could change without us knowing and by storing the size we can
	// safely detect this. (In short, don't do mActiveTexture->getFrameWidth/Height() here)

	if(GetDimensionsInternal().FixedWidth())
		optimalSize.X = GetDimensionsInternal().MinWidth;
	else
	{
		if(SpriteTexture::CheckIsLoaded(mActiveTexture))
			optimalSize.X = mActiveTextureWidth;
		else
			optimalSize.X = GetDimensionsInternal().MaxWidth;
	}

	if(GetDimensionsInternal().FixedHeight())
		optimalSize.Y = GetDimensionsInternal().MinHeight;
	else
	{
		if(SpriteTexture::CheckIsLoaded(mActiveTexture))
			optimalSize.Y = mActiveTextureHeight;
		else
			optimalSize.Y = GetDimensionsInternal().MaxHeight;
	}

	return optimalSize;
}

void GUITexture::FillBuffer(
	u8* vertices,
	u32* indices,
	u32 vertexOffset,
	u32 indexOffset,
	const Vector2I& offset,
	u32 maxNumVerts,
	u32 maxNumIndices,
	u32 renderElementIdx) const
{
	u8* uvs = vertices + sizeof(Vector2);
	u32 vertexStride = sizeof(Vector2) * 2;
	u32 indexStride = sizeof(u32);

	Vector2I layoutOffset = Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y) + mImageSpriteOffset + offset;
	mImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, layoutOffset, mLayoutData.GetLocalClipRect());
}
