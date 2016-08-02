//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSpriteTexture.h"
#include "BsSpriteTextureRTTI.h"
#include "BsTexture.h"
#include "BsResources.h"
#include "BsBuiltinResources.h"

namespace BansheeEngine
{
	SpriteTexture::SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
		:Resource(false), mAtlasTexture(texture), mUVOffset(uvOffset), mUVScale(uvScale)
	{

	}

	const HTexture& SpriteTexture::getTexture() const 
	{ 
		return mAtlasTexture; 
	}

	void SpriteTexture::setTexture(const HTexture& texture)
	{
		mAtlasTexture = texture;

		markDependenciesDirty();
	}

	Vector2 SpriteTexture::transformUV(const Vector2& uv) const
	{
		return mUVOffset + uv * mUVScale;
	}

	const HSpriteTexture& SpriteTexture::dummy()
	{
		return BuiltinResources::instance().getDummySpriteTexture();
	}

	bool SpriteTexture::checkIsLoaded(const HSpriteTexture& tex)
	{
		return tex != nullptr && tex.isLoaded() && tex->getTexture() != nullptr && tex.isLoaded();
	}

	UINT32 SpriteTexture::getWidth() const
	{
		return Math::roundToInt(mAtlasTexture->getProperties().getWidth() * mUVScale.x);
	}

	UINT32 SpriteTexture::getHeight() const
	{
		return Math::roundToInt(mAtlasTexture->getProperties().getHeight() * mUVScale.y);
	}

	void SpriteTexture::getResourceDependencies(FrameVector<HResource>& dependencies) const
	{
		if (mAtlasTexture != nullptr)
			dependencies.push_back(mAtlasTexture);
	}

	void SpriteTexture::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mAtlasTexture.isLoaded())
			dependencies.push_back(mAtlasTexture.get());
	}

	HSpriteTexture SpriteTexture::create(const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = _createPtr(texture);

		return static_resource_cast<SpriteTexture>(gResources()._createResourceHandle(texturePtr));
	}

	HSpriteTexture SpriteTexture::create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = _createPtr(uvOffset, uvScale, texture);

		return static_resource_cast<SpriteTexture>(gResources()._createResourceHandle(texturePtr));
	}

	SPtr<SpriteTexture> SpriteTexture::_createPtr(const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = bs_core_ptr<SpriteTexture>
			(new (bs_alloc<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), texture));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->initialize();

		return texturePtr;
	}

	SPtr<SpriteTexture> SpriteTexture::_createPtr(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = bs_core_ptr<SpriteTexture>
			(new (bs_alloc<SpriteTexture>()) SpriteTexture(uvOffset, uvScale, texture));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->initialize();

		return texturePtr;
	}

	SPtr<SpriteTexture> SpriteTexture::createEmpty()
	{
		SPtr<SpriteTexture> texturePtr = bs_core_ptr<SpriteTexture>
			(new (bs_alloc<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), HTexture()));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->initialize();

		return texturePtr;
	}

	RTTITypeBase* SpriteTexture::getRTTIStatic()
	{
		return SpriteTextureRTTI::instance();
	}

	RTTITypeBase* SpriteTexture::getRTTI() const
	{
		return SpriteTexture::getRTTIStatic();
	}
}