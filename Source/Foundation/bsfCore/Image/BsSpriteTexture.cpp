//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteTexture.h"
#include "Private/RTTI/BsSpriteTextureRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	Rect2 SpriteTextureBase::evaluate(float t) const
	{
		if(mPlayback == SpriteAnimationPlayback::None)
			return Rect2(mUVOffset.x, mUVOffset.y, mUVScale.x, mUVScale.y);

		UINT32 row;
		UINT32 column;
		getAnimationFrame(t, row, column);

		Rect2 output;

		// Note: These could be pre-calculated
		output.width = mUVScale.x / mAnimation.numColumns;
		output.height = mUVScale.y / mAnimation.numRows;

		output.x = mUVOffset.x + column * output.width;
		output.y = mUVOffset.y + row * output.height;

		return output;
	}

	void SpriteTextureBase::getAnimationFrame(float t, UINT32& row, UINT32& column) const
	{
		if(mPlayback == SpriteAnimationPlayback::None)
		{
			row = 0;
			column = 0;

			return;
		}

		// Note: Duration could be pre-calculated
		float duration = 0.0f;
		if (mAnimation.fps > 0)
			duration = mAnimation.count / (float)mAnimation.fps;

		switch(mPlayback)
		{
		default:
		case SpriteAnimationPlayback::Normal:
			t = Math::clamp(t, 0.0f, duration);
			break;
		case SpriteAnimationPlayback::Loop:
			t = Math::repeat(t, duration);
			break;
		case SpriteAnimationPlayback::PingPong:
			t = Math::pingPong(t, duration);
			break;
		}

		const float pct = t / duration;
		UINT32 frame = 0;
		
		if(mAnimation.count > 0)
			frame = Math::clamp(Math::floorToPosInt(pct * mAnimation.count), 0U, mAnimation.count - 1);

		row = frame / mAnimation.numColumns;
		column = frame % mAnimation.numColumns;
	}

	template <bool Core>
	template <class P>
	void TSpriteTexture<Core>::rttiEnumFields(P p)
	{
		p(mUVOffset);
		p(mUVScale);
		p(mAnimation);
		p(mPlayback);
		p(mAtlasTexture);
	}

	SpriteTexture::SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
		:TSpriteTexture(uvOffset, uvScale, texture)
	{ }

	const HSpriteTexture& SpriteTexture::dummy()
	{
		return BuiltinResources::instance().getDummySpriteTexture();
	}

	bool SpriteTexture::checkIsLoaded(const HSpriteTexture& tex)
	{
		return tex != nullptr && tex.isLoaded(false) && tex->getTexture() != nullptr && tex->getTexture().isLoaded(false);
	}

	void SpriteTexture::setTexture(const HTexture& texture)
	{
		removeResourceDependency(mAtlasTexture);
		mAtlasTexture = texture;
		addResourceDependency(mAtlasTexture);

		markDependenciesDirty();
	}

	UINT32 SpriteTexture::getWidth() const
	{
		return Math::roundToInt(mAtlasTexture->getProperties().getWidth() * mUVScale.x);
	}

	UINT32 SpriteTexture::getHeight() const
	{
		return Math::roundToInt(mAtlasTexture->getProperties().getHeight() * mUVScale.y);
	}

	UINT32 SpriteTexture::getFrameWidth() const
	{
		return getWidth() / std::max(1U, mAnimation.numColumns);
	}

	UINT32 SpriteTexture::getFrameHeight() const
	{
		return getHeight() / std::max(1U, mAnimation.numRows);
	}

	void SpriteTexture::_markCoreDirty()
	{
		markCoreDirty();
	}

	void SpriteTexture::initialize()
	{
		addResourceDependency(mAtlasTexture);

		Resource::initialize();
	}

	SPtr<ct::CoreObject> SpriteTexture::createCore() const
	{
		SPtr<ct::Texture> texturePtr;
		if(mAtlasTexture.isLoaded())
			texturePtr = mAtlasTexture->getCore();

		ct::SpriteTexture* spriteTexture = new (bs_alloc<ct::SpriteTexture>()) ct::SpriteTexture(mUVOffset, mUVScale,
			std::move(texturePtr), mAnimation, mPlayback);

		SPtr<ct::SpriteTexture> spriteTexPtr = bs_shared_ptr<ct::SpriteTexture>(spriteTexture);
		spriteTexPtr->_setThisPtr(spriteTexPtr);

		return spriteTexPtr;
	}

	CoreSyncData SpriteTexture::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = csync_size(*this);

		UINT8* buffer = allocator->alloc(size);
		Bitstream stream(buffer, size);
		csync_write(*this, stream);

		return CoreSyncData(buffer, size);
	}

	void SpriteTexture::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mAtlasTexture.isLoaded())
			dependencies.push_back(mAtlasTexture.get());
	}

	SPtr<ct::SpriteTexture> SpriteTexture::getCore() const
	{
		return std::static_pointer_cast<ct::SpriteTexture>(mCoreSpecific);
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

	namespace ct
	{
		SpriteTexture::SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, SPtr<Texture> texture,
			const SpriteSheetGridAnimation& anim, SpriteAnimationPlayback playback)
			:TSpriteTexture(uvOffset, uvScale, texture)
		{
			mAnimation = anim;
			mPlayback = playback;
		}

		void SpriteTexture::syncToCore(const CoreSyncData& data)
		{
			Bitstream stream(data.getBuffer(), data.getBufferSize());
			csync_read(*this, stream);
		}
	}
}
