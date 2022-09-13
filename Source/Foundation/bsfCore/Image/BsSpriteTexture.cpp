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
	Rect2 SpriteTextureBase::Evaluate(float t) const
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

	void SpriteTextureBase::GetAnimationFrame(float t, UINT32& row, UINT32& column) const
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
			t = Math::Clamp(t, 0.0f, duration);
			break;
		case SpriteAnimationPlayback::Loop:
			t = Math::Repeat(t, duration);
			break;
		case SpriteAnimationPlayback::PingPong:
			t = Math::PingPong(t, duration);
			break;
		}

		const float pct = t / duration;
		UINT32 frame = 0;
		
		if(mAnimation.count > 0)
			frame = Math::Clamp(Math::FloorToPosInt(pct * mAnimation.count), 0U, mAnimation.count - 1);

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

	const HSpriteTexture& SpriteTexture::Dummy()
	{
		return BuiltinResources::Instance().GetDummySpriteTexture();
	}

	bool SpriteTexture::CheckIsLoaded(const HSpriteTexture& tex)
	{
		return tex != nullptr && tex.IsLoaded(false) && tex->GetTexture() != nullptr && tex->getTexture().isLoaded(false);
	}

	void SpriteTexture::SetTexture(const HTexture& texture)
	{
		removeResourceDependency(mAtlasTexture);
		mAtlasTexture = texture;
		addResourceDependency(mAtlasTexture);

		markDependenciesDirty();
	}

	UINT32 SpriteTexture::GetWidth() const
	{
		return Math::RoundToInt(mAtlasTexture->GetProperties().GetWidth() * mUVScale.x);
	}

	UINT32 SpriteTexture::GetHeight() const
	{
		return Math::RoundToInt(mAtlasTexture->GetProperties().GetHeight() * mUVScale.y);
	}

	UINT32 SpriteTexture::GetFrameWidth() const
	{
		return GetWidth() / std::max(1U, mAnimation.numColumns);
	}

	UINT32 SpriteTexture::GetFrameHeight() const
	{
		return GetHeight() / std::max(1U, mAnimation.numRows);
	}

	void SpriteTexture::_markCoreDirty()
	{
		markCoreDirty();
	}

	void SpriteTexture::Initialize()
	{
		addResourceDependency(mAtlasTexture);

		Resource::initialize();
	}

	SPtr<ct::CoreObject> SpriteTexture::CreateCore() const
	{
		SPtr<ct::Texture> texturePtr;
		if(mAtlasTexture.IsLoaded())
			texturePtr = mAtlasTexture->GetCore();

		ct::SpriteTexture* spriteTexture = new (bs_alloc<ct::SpriteTexture>()) ct::SpriteTexture(mUVOffset, mUVScale,
			std::move(texturePtr), mAnimation, mPlayback);

		SPtr<ct::SpriteTexture> spriteTexPtr = bs_shared_ptr<ct::SpriteTexture>(spriteTexture);
		spriteTexPtr->_setThisPtr(spriteTexPtr);

		return spriteTexPtr;
	}

	CoreSyncData SpriteTexture::SyncToCore(FrameAlloc* allocator)
	{
		UINT32 size = csync_size(*this);

		UINT8* buffer = allocator->Alloc(size);
		Bitstream Stream(buffer, size);
		csync_write(*this, stream);

		return CoreSyncData(buffer, size);
	}

	void SpriteTexture::GetCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mAtlasTexture.IsLoaded())
			dependencies.push_back(mAtlasTexture.Get());
	}

	SPtr<ct::SpriteTexture> SpriteTexture::GetCore() const
	{
		return std::static_pointer_cast<ct::SpriteTexture>(mCoreSpecific);
	}

	HSpriteTexture SpriteTexture::Create(const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = _createPtr(texture);

		return static_resource_cast<SpriteTexture>(gResources()._createResourceHandle(texturePtr));
	}

	HSpriteTexture SpriteTexture::Create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = _createPtr(uvOffset, uvScale, texture);

		return static_resource_cast<SpriteTexture>(gResources()._createResourceHandle(texturePtr));
	}

	SPtr<SpriteTexture> SpriteTexture::_createPtr(const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = bs_core_ptr<SpriteTexture>
			(new (bs_alloc<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), texture));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->Initialize();

		return texturePtr;
	}

	SPtr<SpriteTexture> SpriteTexture::_createPtr(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
	{
		SPtr<SpriteTexture> texturePtr = bs_core_ptr<SpriteTexture>
			(new (bs_alloc<SpriteTexture>()) SpriteTexture(uvOffset, uvScale, texture));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->Initialize();

		return texturePtr;
	}

	SPtr<SpriteTexture> SpriteTexture::CreateEmpty()
	{
		SPtr<SpriteTexture> texturePtr = bs_core_ptr<SpriteTexture>
			(new (bs_alloc<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), HTexture()));

		texturePtr->_setThisPtr(texturePtr);

		return texturePtr;
	}

	RTTITypeBase* SpriteTexture::getRTTIStatic()
	{
		return SpriteTextureRTTI::Instance();
	}

	RTTITypeBase* SpriteTexture::getRTTI() const
	{
		return SpriteTexture::GetRTTIStatic();
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

		void SpriteTexture::SyncToCore(const CoreSyncData& data)
		{
			Bitstream Stream(data.GetBuffer(), data.getBufferSize());
			csync_read(*this, stream);
		}
	}
}
