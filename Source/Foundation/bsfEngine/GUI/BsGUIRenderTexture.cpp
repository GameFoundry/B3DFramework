//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIRenderTexture.h"
#include "GUI/BsGUIManager.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Image/BsSpriteTexture.h"

namespace bs
{
	const String& GUIRenderTexture::GetGuiTypeName()
	{
		static String name = "RenderTexture";
		return name;
	}

	GUIRenderTexture::GUIRenderTexture(const String& styleName, const SPtr<RenderTexture>& texture, bool transparent, const GUIDimensions& dimensions)
		: GUITexture(styleName, HSpriteTexture(), TextureScaleMode::StretchToFit, false, dimensions), mTransparent(transparent)
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
		return new(bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, transparent, GUIDimensions::Create());
	}

	GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, bool transparent, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, transparent, GUIDimensions::Create(options));
	}

	GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, const String& styleName)
	{
		return new(bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, false, GUIDimensions::Create());
	}

	GUIRenderTexture* GUIRenderTexture::Create(const SPtr<RenderTexture>& texture, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, false, GUIDimensions::Create(options));
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

			SetTexture(SpriteTexture::Create(texture->GetColorTexture(0)));

			GUIManager::Instance().SetInputBridge(mSourceTexture, this);
		}
		else
		{
			SetTexture(SpriteTexture::Create(HTexture()));
		}

		MarkLayoutAsDirtyInternal();
	}

	void GUIRenderTexture::UpdateRenderElementsInternal()
	{
		if(mActiveTexture != nullptr && mActiveTexture.IsLoaded())
			mDesc.Texture = mActiveTexture;

		mDesc.Width = mLayoutData.Area.Width;
		mDesc.Height = mLayoutData.Area.Height;
		mDesc.Transparent = mTransparent;
		mDesc.Color = GetTint();

		mImageSprite->Update(mDesc, (u64)GetParentWidgetInternal());

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::Populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
		}

		GUIElement::UpdateRenderElementsInternal();
	}
} // namespace bs
