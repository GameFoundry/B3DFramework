//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DSpriteImage.h"
#include "RTTI/B3DSpriteImageRTTI.h"
#include "Image/B3DTexture.h"
#include "Resources/B3DBuiltinResources.h"
#include "CoreObject/B3DCoreObjectSync.h"

using namespace b3d;

template<bool IsRenderProxy>
Size2I TSpriteImageAllocation<IsRenderProxy>::GetSize() const
{
	if(!IsValid(mTexture))
		return Size2I::kZero;

	const TextureProperties& atlasTextureProperties = mTexture->GetProperties();

	return Size2I(
		Math::RoundToI32((float)atlasTextureProperties.Width * mUVRange.Width),
		Math::RoundToI32((float)atlasTextureProperties.Height * mUVRange.Height));
}

SpriteImageAllocation::~SpriteImageAllocation()
{
	SPtr<SpriteImage> owner = mOwner.lock();
	if(owner == nullptr)
		return;

	owner->DeallocateImage(this);
}

template class TSpriteImageAllocation<true>;
template class TSpriteImageAllocation<false>;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteImageAllocation, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mOwner)
		B3D_SYNC_BLOCK_ENTRY(mTexture)
		B3D_SYNC_BLOCK_ENTRY(mUVRange)
	B3D_SYNC_BLOCK_END
}

SPtr<SpriteImageAllocation> SpriteImageAllocation::Create(const WeakSPtr<SpriteImage>& owner, const HTexture& atlasTexture, const Area2& uvRange)
{
	SpriteImageAllocation* allocation = new(B3DAllocate<SpriteImageAllocation>()) SpriteImageAllocation(owner, atlasTexture, uvRange);
	SPtr<SpriteImageAllocation> allocationShared = B3DMakeSharedFromExisting<SpriteImageAllocation>(allocation);
	allocationShared->SetShared(allocationShared);
	allocationShared->Initialize();

	return allocationShared;
}

SPtr<render::RenderProxy> SpriteImageAllocation::CreateRenderProxy() const
{
	const SPtr<render::SpriteImage> owner = B3DGetRenderProxy(mOwner.lock());
	const SPtr<render::Texture> atlasTexture = B3DGetRenderProxy(mTexture);

	render::SpriteImageAllocation* const renderProxy = new(B3DAllocate<render::SpriteImageAllocation>()) render::SpriteImageAllocation(owner, atlasTexture, mUVRange);

	SPtr<render::SpriteImageAllocation> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteImageAllocation>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteImageAllocation::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
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
Area2 TSpriteImage<IsRenderProxy>::EvaluateAnimation(const SpriteImageAllocationType& allocation, float t) const
{
	const Area2& uvRange = allocation.GetUVRange();

	if(mInformation.AnimationPlayback == SpriteAnimationPlayback::None)
		return uvRange;

	u32 row;
	u32 column;
	GetAnimationFrame(t, row, column);

	Area2 output;

	// Note: These could be pre-calculated
	output.Width = uvRange.Width / (float)mInformation.Animation.ColumnCount;
	output.Height = uvRange.Height / (float)mInformation.Animation.RowCount;

	output.X = uvRange.X + (float)column * output.Width;
	output.Y = uvRange.Y + (float)row * output.Height;

	return output;
}

template<bool IsRenderProxy>
Size2UI TSpriteImage<IsRenderProxy>::GetAnimationFrameSize() const
{
	const Size2I size = GetDefaultAllocatedImage().GetSize();

	return Size2UI(
		size.Width / Math::Max(1U, mInformation.Animation.ColumnCount),
		size.Height / Math::Max(1U, mInformation.Animation.RowCount));
}

template class TSpriteImage<true>;
template class TSpriteImage<false>;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteImage, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mInformation)
	B3D_SYNC_BLOCK_END
}

void SpriteImage::Destroy()
{
	mDefaultAllocatedImage = nullptr;

	Resource::Destroy();
}

void SpriteImage::DeallocateImage(SpriteImageAllocation* allocation)
{
	auto found = std::find(mScaledAllocatedImages.begin(), mScaledAllocatedImages.end(), allocation);
	if(found != mScaledAllocatedImages.end())
		mScaledAllocatedImages.SwapAndErase(found);
}

void SpriteImage::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
}

SPtr<render::RenderProxy> SpriteImage::CreateRenderProxy() const
{
	render::SpriteImage* const renderProxy = new(B3DAllocate<render::SpriteImage>()) render::SpriteImage(mInformation, B3DGetRenderProxy(mDefaultAllocatedImage));

	SPtr<render::SpriteImage> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteImage>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteImage::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

RTTIType* SpriteImage::GetRttiStatic()
{
	return SpriteImageRTTI::Instance();
}

RTTIType* SpriteImage::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d { namespace render
{
SpriteImageAllocation::SpriteImageAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange)
	:TSpriteImageAllocation(owner, atlasTexture, uvRange)
{ }

void SpriteImageAllocation::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteImageAllocation::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

void SpriteImage::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteImage::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
