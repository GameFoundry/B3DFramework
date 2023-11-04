//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteTexture.h"
#include "Private/RTTI/BsSpriteTextureRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreObject/BsCoreObjectSync.h"

using namespace bs;

Rect2 SpriteImageBase::EvaluateAnimation(float t) const
{
	if(mInformation.AnimationPlayback == SpriteAnimationPlayback::None)
		return mInformation.UVRange;

	u32 row;
	u32 column;
	GetAnimationFrame(t, row, column);

	Rect2 output;

	// Note: These could be pre-calculated
	output.Width = mInformation.UVRange.Width / (float)mInformation.Animation.ColumnCount;
	output.Height = mInformation.UVRange.Height / (float)mInformation.Animation.RowCount;

	output.X = mInformation.UVRange.X + (float)column * output.Width;
	output.Y = mInformation.UVRange.Y + (float)row * output.Height;

	return output;
}

void SpriteImageBase::GetAnimationFrame(float t, u32& outRow, u32& outColumn) const
{
	if(mInformation.AnimationPlayback == SpriteAnimationPlayback::None)
	{
		outRow = 0;
		outColumn = 0;

		return;
	}

	// Note: Duration could be pre-calculated
	float duration = 0.0f;
	if(mInformation.Animation.FramesPerSecond > 0)
		duration = mInformation.Animation.FrameCount / (float)mInformation.Animation.FramesPerSecond;

	switch(mInformation.AnimationPlayback)
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

	const float percent = t / duration;
	u32 frame = 0;

	if(mInformation.Animation.FrameCount > 0)
		frame = Math::Clamp(Math::FloorToPosInt(percent * mInformation.Animation.FrameCount), 0U, mInformation.Animation.FrameCount - 1);

	outRow = frame / mInformation.Animation.ColumnCount;
	outColumn = frame % mInformation.Animation.ColumnCount;
}

template<bool IsRenderProxy>
Size2UI TSpriteImage<IsRenderProxy>::GetSize() const
{
	const TextureType& atlasTexture = GetAtlasTexture();
	const TextureProperties& atlasTextureProperties = atlasTexture->GetProperties();

	return Size2UI(
		Math::RoundToI32(atlasTextureProperties.Width * mInformation.UVRange.Width),
		Math::RoundToI32(atlasTextureProperties.Height * mInformation.UVRange.Height));
}

template<bool IsRenderProxy>
Size2UI TSpriteImage<IsRenderProxy>::GetAnimationFrameSize() const
{
	const Size2UI size = GetSize();

	return Size2UI(
		size.Width / Math::Max(1U, mInformation.Animation.ColumnCount),
		size.Height / Math::Max(1U, mInformation.Animation.RowCount));
}

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(SpriteImage, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mInformation)
	B3D_SYNC_BLOCK_END
}

bool SpriteImage::CheckIsLoaded(const HSpriteImage& image)
{
	return image != nullptr && image.IsLoaded(false) && image->GetAtlasTexture() != nullptr && image->GetAtlasTexture().IsLoaded(false);
}

void SpriteImage::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
}

SPtr<ct::RenderProxy> SpriteImage::CreateRenderProxy() const
{
	ct::SpriteImage* const renderProxy = new(B3DAllocate<ct::SpriteImage>()) ct::SpriteImage(mInformation);

	SPtr<ct::SpriteImage> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteImage>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteImage::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

RTTITypeBase* SpriteImage::GetRttiStatic()
{
	return SpriteImageRTTI::Instance();
}

RTTITypeBase* SpriteImage::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs { namespace ct
{
void SpriteImage::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteImage::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}

Rect2 SpriteTextureBase::Evaluate(float t) const
{
	if(mPlayback == SpriteAnimationPlayback::None)
		return Rect2(mUVOffset.X, mUVOffset.Y, mUVScale.X, mUVScale.Y);

	u32 row;
	u32 column;
	GetAnimationFrame(t, row, column);

	Rect2 output;

	// Note: These could be pre-calculated
	output.Width = mUVScale.X / mAnimation.ColumnCount;
	output.Height = mUVScale.Y / mAnimation.RowCount;

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
	if(mAnimation.FramesPerSecond > 0)
		duration = mAnimation.FrameCount / (float)mAnimation.FramesPerSecond;

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

	if(mAnimation.FrameCount > 0)
		frame = Math::Clamp(Math::FloorToPosInt(pct * mAnimation.FrameCount), 0U, mAnimation.FrameCount - 1);

	row = frame / mAnimation.ColumnCount;
	column = frame % mAnimation.ColumnCount;
}

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(SpriteTexture, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mUVOffset)
		B3D_SYNC_BLOCK_ENTRY(mUVScale)
		B3D_SYNC_BLOCK_ENTRY(mAnimation)
		B3D_SYNC_BLOCK_ENTRY(mPlayback)
		B3D_SYNC_BLOCK_ENTRY(mAtlasTexture)
	B3D_SYNC_BLOCK_END
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
	return GetWidth() / std::max(1U, mAnimation.ColumnCount);
}

u32 SpriteTexture::GetFrameHeight() const
{
	return GetHeight() / std::max(1U, mAnimation.RowCount);
}

void SpriteTexture::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
}

void SpriteTexture::Initialize()
{
	AddResourceDependency(mAtlasTexture);

	Resource::Initialize();
}

SPtr<ct::RenderProxy> SpriteTexture::CreateRenderProxy() const
{
	SPtr<ct::Texture> atlasRenderProxy = B3DGetRenderProxy(mAtlasTexture);
	ct::SpriteTexture* const renderProxy = new(B3DAllocate<ct::SpriteTexture>()) ct::SpriteTexture(mUVOffset, mUVScale, std::move(atlasRenderProxy), mAnimation, mPlayback);

	SPtr<ct::SpriteTexture> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteTexture>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteTexture::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

void SpriteTexture::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mAtlasTexture.IsLoaded())
		dependencies.push_back(mAtlasTexture.Get());
}

HSpriteTexture SpriteTexture::Create(const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = CreatePtrInternal(texture);

	return B3DStaticResourceCast<SpriteTexture>(GetResources().CreateResourceHandle(texturePtr));
}

HSpriteTexture SpriteTexture::Create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = CreatePtrInternal(uvOffset, uvScale, texture);

	return B3DStaticResourceCast<SpriteTexture>(GetResources().CreateResourceHandle(texturePtr));
}

SPtr<SpriteTexture> SpriteTexture::CreatePtrInternal(const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = B3DMakeSharedFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), texture));

	texturePtr->SetShared(texturePtr);
	texturePtr->Initialize();

	return texturePtr;
}

SPtr<SpriteTexture> SpriteTexture::CreatePtrInternal(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = B3DMakeSharedFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(uvOffset, uvScale, texture));

	texturePtr->SetShared(texturePtr);
	texturePtr->Initialize();

	return texturePtr;
}

SPtr<SpriteTexture> SpriteTexture::CreateEmpty()
{
	SPtr<SpriteTexture> texturePtr = B3DMakeSharedFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), HTexture()));

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

void SpriteTexture::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteTexture::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
