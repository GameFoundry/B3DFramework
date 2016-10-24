//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIRenderTexture.h"
#include "BsGUIManager.h"
#include "BsRenderTexture.h"
#include "BsSpriteTexture.h"

namespace BansheeEngine
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
			GUIManager::instance().setInputBridge(mSourceTexture.get(), nullptr);
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
			GUIManager::instance().setInputBridge(mSourceTexture.get(), nullptr);

		mSourceTexture = texture;

		if (mSourceTexture != nullptr)
		{
			if (mSourceTexture->getProperties().requiresTextureFlipping())
			{
				mDesc.uvOffset = Vector2(0.0f, 1.0f);
				mDesc.uvScale = Vector2(1.0f, -1.0f);
			}

			setTexture(SpriteTexture::create(texture->getColorTexture(0)));

			GUIManager::instance().setInputBridge(mSourceTexture.get(), this);
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
		{
			mDesc.texture = mActiveTexture.getInternalPtr();
		}

		mDesc.width = mLayoutData.area.width;
		mDesc.height = mLayoutData.area.height;
		mDesc.transparent = mTransparent;
		mDesc.color = getTint();

		mImageSprite->update(mDesc, (UINT64)_getParentWidget());

		GUIElement::updateRenderElementsInternal();
	}
}