//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIRenderTexture.h"

#include "BsGUITexture.h"
#include "GUI/BsGUIManager.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Image/BsSpriteTexture.h"

using namespace bs;

const String& GUIRenderTexture::GetGuiTypeName()
{
	static String name = "RenderTexture";
	return name;
}

GUIRenderTexture::GUIRenderTexture(const String& styleName, const SPtr<RenderTexture>& texture, bool transparent, const GUISizeConstraints& dimensions)
	: GUITexture(PrivatelyConstruct(), GUITextureContents(nullptr, TextureScaleMode::StretchToFit, false), styleName, dimensions), mTransparent(transparent)
{
	SetRenderTexture(texture);
}

GUIRenderTexture::~GUIRenderTexture()
{
	if(mSourceTexture != nullptr)
		GUIManager::Instance().SetInputBridge(mSourceTexture, nullptr);
}

GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, bool transparent, const String& styleName)
{
	return new(B3DAllocate<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, transparent, GUISizeConstraints::Create());
}

GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, bool transparent, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, transparent, GUISizeConstraints::Create(options));
}

GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, const String& styleName)
{
	return new(B3DAllocate<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, false, GUISizeConstraints::Create());
}

GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, false, GUISizeConstraints::Create(options));
}

void GUIRenderTexture::SetRenderTexture(const SPtr<RenderTexture>& texture)
{
	if(mSourceTexture != nullptr)
		GUIManager::Instance().SetInputBridge(mSourceTexture, nullptr);

	mSourceTexture = texture;

	if(mSourceTexture != nullptr)
	{
		if(mSourceTexture->GetProperties().RequiresTextureFlipping)
		{
			mDesc.UvOffset = Vector2(0.0f, 1.0f);
			mDesc.UvScale = Vector2(1.0f, -1.0f);
		}

		SetImage(SpriteTexture::Create(texture->GetColorTexture(0)));

		GUIManager::Instance().SetInputBridge(mSourceTexture, this);
	}
	else
	{
		SetImage(SpriteTexture::Create(HTexture()));
	}

	MarkLayoutAsDirty();
}

void GUIRenderTexture::UpdateRenderElements()
{
	if(mActiveImage != nullptr && mActiveImage.IsLoaded())
		mDesc.Image = mActiveImage;

	mDesc.Width = mLayoutData.Area.Width;
	mDesc.Height = mLayoutData.Area.Height;
	mDesc.Transparent = mTransparent;
	mDesc.Color = GetTint();

	mImageSprite->Update(mDesc, (u64)GetParentWidget());

	const Rect2 imageBounds(
		0.0f, 0.0f,
		(float)mLayoutData.Area.Width, (float)mLayoutData.Area.Height);

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mImageSprite, 0, imageBounds) }, mRenderElements);
	}

	GUIInteractable::UpdateRenderElements();
}
