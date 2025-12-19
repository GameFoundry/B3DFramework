//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuCommandBuffer.h"

#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "Profiling/B3DProfilerGPU.h"

using namespace b3d;

namespace b3d { namespace render
{
GpuCommandBufferPool::GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mInformation(createInformation)
{
	// Process messages related to this command buffer pool on this thread. Mostly these are command buffer resets once they are done executing.
	Scheduler* const scheduler = Scheduler::Get();
	if (B3D_ENSURE(scheduler))
	{
		mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
	}
}

void GpuCommandBufferPool::Destroy()
{
	if (mIsDestroyed)
		return;

	mIsDestroyed = true;
}

GpuCommandBuffer::GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mUsage(queueType), mOwnerThread(ownerThread), mInformation(createInformation)
{ }


GpuCommandBuffer::~GpuCommandBuffer()
{
	OnDestroyed(mIsSubmitted);
}

#if B3D_PROFILING_ENABLED
SPtr<GpuCommandBufferProfiler> GpuCommandBuffer::BeginProfiling(const ProfilerString& profilingScopeName)
{
	if(!B3D_ENSURE(mProfiler == nullptr))
		return nullptr;

	mProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*this);
	mProfilingScopeName = profilingScopeName;

	return mProfiler;
}

void GpuCommandBuffer::EndProfiling()
{
	if(!B3D_ENSURE(mProfiler != nullptr))
		return;

	GetGpuProfiler().ResolveProfileWhenReady(mProfilingScopeName, mProfiler);

	mProfiler = nullptr;
	mProfilingScopeName.clear();
}
#endif

bool GpuCommandBuffer::CopyTexture(const SPtr<Texture>& source, const SPtr<Texture>& destination, const TextureCopyInformation& copyInformation)
{
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
	{
		B3D_LOG(Error, Texture, "Copy operation failed. Source or destination texture is null.");
		return false;
	}

	const TextureProperties& sourceProperties = source->GetProperties();
	const TextureProperties& destinationProperties = destination->GetProperties();

	if(copyInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, Texture, "Copy operation failed. Face count is zero.");
		return false;
	}

	if(destinationProperties.Type != sourceProperties.Type)
	{
		B3D_LOG(Error, Texture, "Source and destination textures must be of same type.");
		return false;
	}

	if(sourceProperties.Format != destinationProperties.Format)
	{
		B3D_LOG(Error, Texture, "Source and destination texture formats must match.");
		return false;
	}

	if(destinationProperties.SampleCount > 1 && sourceProperties.SampleCount != destinationProperties.SampleCount)
	{
		B3D_LOG(Error, Texture, "When copying to a multisampled texture, source texture must have the same number of samples.");
		return false;
	}

	if((copyInformation.SourceFace + copyInformation.FaceCount) > sourceProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid source face index.");
		return false;
	}

	if((copyInformation.DestinationFace + copyInformation.FaceCount) > destinationProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid destination face index.");
		return false;
	}

	if(copyInformation.SourceMip > sourceProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Source mip level out of range. Valid range is [0, {0}].", sourceProperties.MipMapCount);
		return false;
	}

	if(copyInformation.DestinationMip > destinationProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Destination mip level out of range. Valid range is [0, {0}].", destinationProperties.MipMapCount);
		return false;
	}

	u32 sourceWidth, sourceHeight, sourceDepth;
	PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, copyInformation.SourceMip, sourceWidth, sourceHeight, sourceDepth);

	u32 destinationWidth, destinationHeight, destinationDepth;
	PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);

	if(copyInformation.DestinationPosition.X < 0 || copyInformation.DestinationPosition.X >= (i32)destinationWidth ||
	   copyInformation.DestinationPosition.Y < 0 || copyInformation.DestinationPosition.Y >= (i32)destinationHeight ||
	   copyInformation.DestinationPosition.Z < 0 || copyInformation.DestinationPosition.Z >= (i32)destinationDepth)
	{
		B3D_LOG(Error, Texture, "Destination position falls outside the destination texture.");
		return false;
	}

	bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
		copyInformation.SourceVolume.GetHeight() == 0 ||
		copyInformation.SourceVolume.GetDepth() == 0;

	u32 destinationRight = (u32)copyInformation.DestinationPosition.X;
	u32 destinationBottom = (u32)copyInformation.DestinationPosition.Y;
	u32 destinationBack = (u32)copyInformation.DestinationPosition.Z;
	if(!copyEntireSurface)
	{
		if(copyInformation.SourceVolume.Left >= sourceWidth || copyInformation.SourceVolume.Right > sourceWidth ||
		   copyInformation.SourceVolume.Top >= sourceHeight || copyInformation.SourceVolume.Bottom > sourceHeight ||
		   copyInformation.SourceVolume.Front >= sourceDepth || copyInformation.SourceVolume.Back > sourceDepth)
		{
			B3D_LOG(Error, Texture, "Source volume falls outside the source texture.");
			return false;
		}

		destinationRight += copyInformation.SourceVolume.GetWidth();
		destinationBottom += copyInformation.SourceVolume.GetHeight();
		destinationBack += copyInformation.SourceVolume.GetDepth();
	}
	else
	{
		destinationRight += sourceWidth;
		destinationBottom += sourceHeight;
		destinationBack += sourceDepth;
	}

	if(destinationRight > destinationWidth || destinationBottom > destinationHeight || destinationBack > destinationDepth)
	{
		B3D_LOG(Error, Texture, "Destination volume falls outside the destination texture.");
		return false;
	}

	const bool sourceHasMultipleSamples = sourceProperties.SampleCount > 1;
	const bool destinationHasMultipleSamples = destinationProperties.SampleCount > 1;

	if((sourceProperties.Usage & TU_DEPTHSTENCIL) != 0 || (destinationProperties.Usage & TU_DEPTHSTENCIL) != 0)
	{
		B3D_LOG(Error, RenderBackend, "Texture copy/resolve isn't supported for depth-stencil textures.");
		return false;
	}

	bool needsResolve = sourceHasMultipleSamples && !destinationHasMultipleSamples;
	bool isMSCopy = sourceHasMultipleSamples || destinationHasMultipleSamples;
	if(!needsResolve && isMSCopy)
	{
		if(sourceProperties.SampleCount != destinationProperties.SampleCount)
		{
			B3D_LOG(Error, RenderBackend, "When copying textures their multisample counts must match. Ignoring copy.");
			return false;
		}
	}

	return true;
}

bool GpuCommandBuffer::BlitTexture(const SPtr<Texture>& source, const SPtr<Texture>& destination, const TextureBlitInformation& blitInformation)
{
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Source or destination texture is null.");
		return false;
	}

	const TextureProperties& sourceProperties = source->GetProperties();
	const TextureProperties& destinationProperties = destination->GetProperties();

	if(blitInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, Texture, "Blit operation failed. Face count is zero.");
		return false;
	}

	if((blitInformation.SourceFace + blitInformation.FaceCount) > sourceProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Source face out of valid range.");
		return false;
	}

	if((blitInformation.DestinationFace + blitInformation.FaceCount) > destinationProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Destination face out of valid range.");
		return false;
	}

	if(blitInformation.SourceMip > sourceProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Source mip level out of valid range. Valid range is [0, {0}].", sourceProperties.MipMapCount);
		return false;
	}

	if(blitInformation.DestinationMip > destinationProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Destination mip level out of range. Valid range is [0, {0}].", destinationProperties.MipMapCount);
		return false;
	}

	if((sourceProperties.Usage & TU_DEPTHSTENCIL) != 0 || (destinationProperties.Usage & TU_DEPTHSTENCIL) != 0)
	{
		B3D_LOG(Error, RenderBackend, "Texture blit isn't supported for depth-stencil textures.");
		return false;
	}

	return true;
}
}}
