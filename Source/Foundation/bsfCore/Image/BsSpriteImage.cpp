//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteImage.h"
#include "Private/RTTI/BsSpriteImageRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreObject/BsCoreObjectSync.h"

using namespace bs;

Area2 SpriteImageBase::EvaluateAnimation(float t) const
{
	if(mInformation.AnimationPlayback == SpriteAnimationPlayback::None)
		return mInformation.UVRange;

	u32 row;
	u32 column;
	GetAnimationFrame(t, row, column);

	Area2 output;

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

template class TSpriteImage<true>;
template class TSpriteImage<false>;

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

RTTIType* SpriteImage::GetRttiStatic()
{
	return SpriteImageRTTI::Instance();
}

RTTIType* SpriteImage::GetRtti() const
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
