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
	mImageSprite = B3DNew<ImageSprite>();
	mDesc.AnimationStartTime = GetTime().GetTime();

	if(texture != nullptr)
	{
		mActiveTexture = texture;
		mUsingStyleTexture = false;
	}
	else
	{
		mActiveTexture = GetStyle()->Normal.Texture;
		mUsingStyleTexture = true;
	}

	bool isTexLoaded = SpriteTexture::CheckIsLoaded(mActiveTexture);
	mActiveTextureWidth = isTexLoaded ? mActiveTexture->GetFrameWidth() : 0;
	mActiveTextureHeight = isTexLoaded ? mActiveTexture->GetFrameHeight() : 0;
}

GUITexture::~GUITexture()
{
	B3DDelete(mImageSprite);
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, transparent, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, transparent, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, TextureScaleMode scale, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, scale, true, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, TextureScaleMode::StretchToFit, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const HSpriteTexture& texture, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), texture, TextureScaleMode::StretchToFit, true, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(TextureScaleMode scale, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), scale, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(TextureScaleMode scale, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), scale, true, GUIDimensions::Create());
}

GUITexture* GUITexture::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), TextureScaleMode::StretchToFit, true, GUIDimensions::Create(options));
}

GUITexture* GUITexture::Create(const String& styleName)
{
	return new(B3DAllocate<GUITexture>()) GUITexture(GetStyleName<GUITexture>(styleName), HSpriteTexture(), TextureScaleMode::StretchToFit, true, GUIDimensions::Create());
}

void GUITexture::SetTexture(const HSpriteTexture& texture)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	mActiveTexture = texture;

	bool isTexLoaded = SpriteTexture::CheckIsLoaded(mActiveTexture);
	mActiveTextureWidth = isTexLoaded ? mActiveTexture->GetFrameWidth() : 0;
	mActiveTextureHeight = isTexLoaded ? mActiveTexture->GetFrameHeight() : 0;

	mUsingStyleTexture = false;
	mDesc.AnimationStartTime = GetTime().GetTime();

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUITexture::UpdateRenderElements()
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
				destSize.X = Math::RoundToU32(textureSize.X / aspectY);
				destSize.Y = Math::RoundToU32(textureSize.Y / aspectY);
			}
			else
			{
				destSize.X = Math::RoundToU32(textureSize.X / aspectX);
				destSize.Y = Math::RoundToU32(textureSize.Y / aspectX);
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

	mDesc.BorderLeft = GetStyle()->Border.Left;
	mDesc.BorderRight = GetStyle()->Border.Right;
	mDesc.BorderTop = GetStyle()->Border.Top;
	mDesc.BorderBottom = GetStyle()->Border.Bottom;
	mDesc.Transparent = mTransparent;
	mDesc.Color = GetTint();

	if(mScaleMode != TextureScaleMode::ScaleToFit)
		mDesc.UvScale = ImageSprite::GetTextureUvScale(textureSize, destSize, mScaleMode);
	else
		mDesc.UvScale = Vector2::kOne;

	mImageSprite->Update(mDesc, (u64)GetParentWidget());

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
	}

	GUIElement::UpdateRenderElements();
}

void GUITexture::StyleUpdated()
{
	if(mUsingStyleTexture)
	{
		mActiveTexture = GetStyle()->Normal.Texture;
		mDesc.AnimationStartTime = GetTime().GetTime();

		bool isTexLoaded = SpriteTexture::CheckIsLoaded(mActiveTexture);
		mActiveTextureWidth = isTexLoaded ? mActiveTexture->GetFrameWidth() : 0;
		mActiveTextureHeight = isTexLoaded ? mActiveTexture->GetFrameHeight() : 0;
	}
}

Vector2I GUITexture::GetOptimalSize() const
{
	// TODO - Accounting for style dimensions might be redundant here, I'm pretty sure we do that on higher level anyway
	Vector2I optimalSize;

	// Note: We use cached texture size here. This is because we use this method for checking we a layout update is
	// needed (size change is detected). Sprite texture could change without us knowing and by storing the size we can
	// safely detect this. (In short, don't do mActiveTexture->getFrameWidth/Height() here)

	if(GetDimensions().FixedWidth())
		optimalSize.X = GetDimensions().MinWidth;
	else
	{
		if(SpriteTexture::CheckIsLoaded(mActiveTexture))
			optimalSize.X = mActiveTextureWidth;
		else
			optimalSize.X = GetDimensions().MaxWidth;
	}

	if(GetDimensions().FixedHeight())
		optimalSize.Y = GetDimensions().MinHeight;
	else
	{
		if(SpriteTexture::CheckIsLoaded(mActiveTexture))
			optimalSize.Y = mActiveTextureHeight;
		else
			optimalSize.Y = GetDimensions().MaxHeight;
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
