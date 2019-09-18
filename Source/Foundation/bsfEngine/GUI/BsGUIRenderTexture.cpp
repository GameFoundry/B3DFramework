//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIRenderTexture.h"
#include "GUI/BsGUIManager.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Image/BsSpriteTexture.h"

namespace bs
{
	const String& GUIRenderTexture::getGUITypeName()
	{
		static String name = "RenderTexture";
		return name;
	}

	GUIRenderTexture::GUIRenderTexture(const String& styleName, const SPtr<RenderTexture>& texture, bool transparent,
		const GUIDimensions& dimensions)
		:GUITexture(styleName, HSpriteTexture(), TextureScaleMode::StretchToFit, false, dimensions), mTransparent(transparent)
	{
		setRenderTexture(texture);
	}

	GUIRenderTexture::~GUIRenderTexture()
	{
		if (mSourceTexture != nullptr)
			GUIManager::instance().setInputBridge(mSourceTexture, nullptr);
	}

	GUIRenderTexture* GUIRenderTexture::create(const SPtr<RenderTexture>& texture, bool transparent, const String& styleName)
	{
		return new (bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, transparent, GUIDimensions::create());
	}

	GUIRenderTexture* GUIRenderTexture::create(const SPtr<RenderTexture>& texture, bool transparent, const GUIOptions& options,
		const String& styleName)
	{
		return new (bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, transparent, GUIDimensions::create(options));
	}

	GUIRenderTexture* GUIRenderTexture::create(const SPtr<RenderTexture>& texture, const String& styleName)
	{
		return new (bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, false, GUIDimensions::create());
	}

	GUIRenderTexture* GUIRenderTexture::create(const SPtr<RenderTexture>& texture, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIRenderTexture>()) GUIRenderTexture(styleName, texture, false, GUIDimensions::create(options));
	}

	void GUIRenderTexture::setRenderTexture(const SPtr<RenderTexture>& texture)
	{
		if (mSourceTexture != nullptr)
			GUIManager::instance().setInputBridge(mSourceTexture, nullptr);

		mSourceTexture = texture;

		if (mSourceTexture != nullptr)
		{
			if (mSourceTexture->getProperties().requiresTextureFlipping)
			{
				mDesc.uvOffset = Vector2(0.0f, 1.0f);
				mDesc.uvScale = Vector2(1.0f, -1.0f);
			}

			setTexture(SpriteTexture::create(texture->getColorTexture(0)));

			GUIManager::instance().setInputBridge(mSourceTexture, this);
		}
		else
		{
			setTexture(SpriteTexture::create(HTexture()));
		}

		_markLayoutAsDirty();
	}

	void GUIRenderTexture::updateRenderElementsInternal()
	{		
		if(mActiveTexture != nullptr && mActiveTexture.isLoaded())
			mDesc.texture = mActiveTexture;

		mDesc.width = mLayoutData.area.width;
		mDesc.height = mLayoutData.area.height;
		mDesc.transparent = mTransparent;
		mDesc.color = getTint();

		mImageSprite->update(mDesc, (UINT64)_getParentWidget());

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
		}

		GUIElement::updateRenderElementsInternal();
	}
}
