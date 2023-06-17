//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteTexture.h"
#include "Private/RTTI/BsSpriteTextureRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreThread/BsCoreObjectSync.h"

using namespace bs;

Rect2 SpriteTextureBase::Evaluate(float t) const
{
	if(mPlayback == SpriteAnimationPlayback::None)
		return Rect2(mUVOffset.X, mUVOffset.Y, mUVScale.X, mUVScale.Y);

	u32 row;
	u32 column;
	GetAnimationFrame(t, row, column);

	Rect2 output;

	// Note: These could be pre-calculated
	output.Width = mUVScale.X / mAnimation.NumColumns;
	output.Height = mUVScale.Y / mAnimation.NumRows;

	output.X = mUVOffset.X + column * output.Width;
	output.Y = mUVOffset.Y + row * output.Height;

	return output;
}

void SpriteTextureBase::GetAnimationFrame(float t, u32& row, u32& column) const
{
	if(mPlayback == SpriteAnimationPlayback::None)
	{
		row = 0;
		column = 0;

		return;
	}

	// Note: Duration could be pre-calculated
	float duration = 0.0f;
	if(mAnimation.Fps > 0)
		duration = mAnimation.Count / (float)mAnimation.Fps;

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
	u32 frame = 0;

	if(mAnimation.Count > 0)
		frame = Math::Clamp(Math::FloorToPosInt(pct * mAnimation.Count), 0U, mAnimation.Count - 1);

	row = frame / mAnimation.NumColumns;
	column = frame % mAnimation.NumColumns;
}

template <bool Core>
template <class P>
void TSpriteTexture<Core>::RttiEnumFields(P p)
{
	p(mUVOffset);
	p(mUVScale);
	p(mAnimation);
	p(mPlayback);
	p(mAtlasTexture);
}

SpriteTexture::SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
	: TSpriteTexture(uvOffset, uvScale, texture)
{}

const HSpriteTexture& SpriteTexture::Dummy()
{
	return BuiltinResources::Instance().GetDummySpriteTexture();
}

bool SpriteTexture::CheckIsLoaded(const HSpriteTexture& tex)
{
	return tex != nullptr && tex.IsLoaded(false) && tex->GetTexture() != nullptr && tex->GetTexture().IsLoaded(false);
}

void SpriteTexture::SetTexture(const HTexture& texture)
{
	RemoveResourceDependency(mAtlasTexture);
	mAtlasTexture = texture;
	AddResourceDependency(mAtlasTexture);

	MarkDependenciesDirty();
}

u32 SpriteTexture::GetWidth() const
{
	return Math::RoundToI32(mAtlasTexture->GetProperties().Width * mUVScale.X);
}

u32 SpriteTexture::GetHeight() const
{
	return Math::RoundToI32(mAtlasTexture->GetProperties().Height * mUVScale.Y);
}

u32 SpriteTexture::GetFrameWidth() const
{
	return GetWidth() / std::max(1U, mAnimation.NumColumns);
}

u32 SpriteTexture::GetFrameHeight() const
{
	return GetHeight() / std::max(1U, mAnimation.NumRows);
}

void SpriteTexture::MarkCoreDirtyInternal()
{
	MarkCoreDirty();
}

void SpriteTexture::Initialize()
{
	AddResourceDependency(mAtlasTexture);

	Resource::Initialize();
}

SPtr<ct::CoreObject> SpriteTexture::CreateCore() const
{
	SPtr<ct::Texture> texturePtr;
	if(mAtlasTexture.IsLoaded())
		texturePtr = mAtlasTexture->GetCore();

	ct::SpriteTexture* spriteTexture = new(B3DAllocate<ct::SpriteTexture>()) ct::SpriteTexture(mUVOffset, mUVScale, std::move(texturePtr), mAnimation, mPlayback);

	SPtr<ct::SpriteTexture> spriteTexPtr = B3DMakeSharedFromExisting<ct::SpriteTexture>(spriteTexture);
	spriteTexPtr->SetShared(spriteTexPtr);

	return spriteTexPtr;
}

CoreSyncData SpriteTexture::SyncToCore(FrameAlloc* allocator)
{
	u32 size = CoreSyncGetSize(*this);

	u8* buffer = allocator->Alloc(size);
	Bitstream stream(buffer, size);
	B3DCoreSyncWrite(*this, stream);

	return CoreSyncData(buffer, size);
}

void SpriteTexture::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mAtlasTexture.IsLoaded())
		dependencies.push_back(mAtlasTexture.Get());
}

SPtr<ct::SpriteTexture> SpriteTexture::GetCore() const
{
	return std::static_pointer_cast<ct::SpriteTexture>(mCoreSpecific);
}

HSpriteTexture SpriteTexture::Create(const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = CreatePtrInternal(texture);

	return B3DStaticResourceCast<SpriteTexture>(GetResources().CreateResourceHandleInternal(texturePtr));
}

HSpriteTexture SpriteTexture::Create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = CreatePtrInternal(uvOffset, uvScale, texture);

	return B3DStaticResourceCast<SpriteTexture>(GetResources().CreateResourceHandleInternal(texturePtr));
}

SPtr<SpriteTexture> SpriteTexture::CreatePtrInternal(const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = B3DMakeCoreFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), texture));

	texturePtr->SetShared(texturePtr);
	texturePtr->Initialize();

	return texturePtr;
}

SPtr<SpriteTexture> SpriteTexture::CreatePtrInternal(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = B3DMakeCoreFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(uvOffset, uvScale, texture));

	texturePtr->SetShared(texturePtr);
	texturePtr->Initialize();

	return texturePtr;
}

SPtr<SpriteTexture> SpriteTexture::CreateEmpty()
{
	SPtr<SpriteTexture> texturePtr = B3DMakeCoreFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), HTexture()));

	texturePtr->SetShared(texturePtr);

	return texturePtr;
}

RTTITypeBase* SpriteTexture::GetRttiStatic()
{
	return SpriteTextureRTTI::Instance();
}

RTTITypeBase* SpriteTexture::GetRtti() const
{
	return SpriteTexture::GetRttiStatic();
}

namespace bs { namespace ct
{
SpriteTexture::SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, SPtr<Texture> texture, const SpriteSheetGridAnimation& anim, SpriteAnimationPlayback playback)
	: TSpriteTexture(uvOffset, uvScale, texture)
{
	mAnimation = anim;
	mPlayback = playback;
}

void SpriteTexture::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());
	B3DCoreSyncRead(*this, stream);
}
}}
