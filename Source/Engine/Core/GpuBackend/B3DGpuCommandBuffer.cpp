//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "Profiling/B3DProfilerGPU.h"

using namespace b3d;

namespace b3d { namespace render
{
#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuDrawAccessValidator::BeginRenderPass()
{
	mPreviousDrawAccesses.clear();
	mBindingsUsed = false;
}

void GpuDrawAccessValidator::ClearBindings()
{
	mBindings.clear();
	mBindingsUsed = false;
	mBindingsHaveWrites = false;
}

void GpuDrawAccessValidator::AddResource(const void* resource, GpuAccessFlags access)
{
	if(resource == nullptr)
		return;

	mBindings[resource] |= access;
	mBindingsHaveWrites |= access.IsSet(GpuAccessFlag::Write);
	mBindingsUsed = false;
}

void GpuDrawAccessValidator::AddParameterSet(const GpuParameterSet& parameters, const GpuPipelineParameterSetLayout& layout)
{
	const TShared<GpuPipelineParameterSetLayout> parameterLayout = parameters.GetLayout();
	for(u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
	{
		const GpuParameterType type = (GpuParameterType)typeIndex;
		if(type == GpuParameterType::Sampler)
			continue;

		for(u32 bindingIndex = 0; bindingIndex < layout.GetBindingCount(type); bindingIndex++)
		{
			const UniformInformation& uniform = *layout.TryGetUniformInformation(type, bindingIndex);
			const UniformInformation* parameter = parameterLayout->TryGetUniformInformation(uniform.Slot);
			if(parameter == nullptr || parameter->Type != type)
				continue;

			for(u32 arrayIndex = 0; arrayIndex < std::min(uniform.ArraySize, parameter->ArraySize); arrayIndex++)
			{
				switch(type)
				{
				case GpuParameterType::UniformBuffer:
					AddResource(parameters.GetUniformBuffer(uniform.Slot, arrayIndex).get(), GpuAccessFlag::Read);
					break;
				case GpuParameterType::StorageBuffer:
					AddResource(parameters.GetStorageBuffer(uniform.Slot, arrayIndex).get(), GpuObjectParameterTypeInformation::IsReadWriteBuffer(uniform.ObjectType) ? GpuAccessFlag::Read | GpuAccessFlag::Write : GpuAccessFlags(GpuAccessFlag::Read));
					break;
				case GpuParameterType::SampledTexture:
					AddResource(parameters.GetSampledTexture(uniform.Slot, arrayIndex).get(), GpuAccessFlag::Read);
					break;
				case GpuParameterType::StorageTexture:
					AddResource(parameters.GetStorageTexture(uniform.Slot, arrayIndex).get(), GpuAccessFlag::Read | GpuAccessFlag::Write);
					break;
				default:
					break;
				}
			}
		}
	}
}

bool GpuDrawAccessValidator::ValidateDraw()
{
	if(mBindingsUsed)
		return !mBindingsHaveWrites;

	for(const auto& [resource, access] : mBindings)
	{
		const auto previousAccess = mPreviousDrawAccesses.find(resource);
		if(previousAccess != mPreviousDrawAccesses.end() && (previousAccess->second | access).IsSet(GpuAccessFlag::Write))
			return false;
	}

	for(const auto& [resource, access] : mBindings)
		mPreviousDrawAccesses[resource] |= access;

	mBindingsUsed = true;
	return true;
}

#endif

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

GpuCommandBuffer::GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mQueueType(queueType), mOwnerThread(ownerThread), mInformation(createInformation)
{ }


GpuCommandBuffer::~GpuCommandBuffer()
{
	OnDestroyed(mState == GpuCommandBufferState::Executing);
}

#if B3D_BUILD_TYPE_DEVELOPMENT
bool GpuCommandBuffer::ValidateDrawAccesses()
{
	if(mDrawAccessValidator.ValidateDraw())
		return true;

	B3D_LOG(Error, LogRenderBackend, "Draw rejected: shader resources written by a draw cannot be accessed by another draw in the same render pass.");
	return false;
}

#endif

void GpuCommandBuffer::SetPushConstants(u32 /*offsetInBytes*/, u32 sizeInBytes, const void* /*data*/)
{
	EnsureValidThread();
	if(sizeInBytes == 0)
		return;

	B3D_LOG(Error, LogRenderBackend, "Push constants are unsupported by this GPU backend.");
}

#if B3D_PROFILING_ENABLED
TShared<GpuCommandBufferProfiler> GpuCommandBuffer::BeginProfiling(const ProfilerString& profilingScopeName)
{
	if(!B3D_ENSURE(mProfiler == nullptr))
		return nullptr;

	if(!mGpuDevice.GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return nullptr;

	mProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*this);
	mProfilingScopeName = profilingScopeName;

	return mProfiler;
}

void GpuCommandBuffer::EndProfiling()
{
	if(mProfiler == nullptr && !mGpuDevice.GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return;

	if(!B3D_ENSURE(mProfiler != nullptr))
		return;

	GetGpuProfiler().ResolveProfileWhenReady(mProfilingScopeName, mProfiler);

	mProfiler = nullptr;
	mProfilingScopeName.clear();
}
#endif

bool GpuCommandBuffer::CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation)
{
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
	{
		B3D_LOG(Error, LogTexture, "Copy operation failed. Source or destination texture is null.");
		return false;
	}

	const TextureProperties& sourceProperties = source->GetProperties();
	const TextureProperties& destinationProperties = destination->GetProperties();

	if(copyInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, LogTexture, "Copy operation failed. Face count is zero.");
		return false;
	}

	if(destinationProperties.Type != sourceProperties.Type)
	{
		B3D_LOG(Error, LogTexture, "Source and destination textures must be of same type.");
		return false;
	}

	if(sourceProperties.Format != destinationProperties.Format)
	{
		B3D_LOG(Error, LogTexture, "Source and destination texture formats must match.");
		return false;
	}

	if(destinationProperties.SampleCount > 1 && sourceProperties.SampleCount != destinationProperties.SampleCount)
	{
		B3D_LOG(Error, LogTexture, "When copying to a multisampled texture, source texture must have the same number of samples.");
		return false;
	}

	if((copyInformation.SourceFace + copyInformation.FaceCount) > sourceProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Invalid source face index.");
		return false;
	}

	if((copyInformation.DestinationFace + copyInformation.FaceCount) > destinationProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Invalid destination face index.");
		return false;
	}

	if(copyInformation.SourceMip > sourceProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Source mip level out of range. Valid range is [0, {0}].", sourceProperties.MipMapCount);
		return false;
	}

	if(copyInformation.DestinationMip > destinationProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Destination mip level out of range. Valid range is [0, {0}].", destinationProperties.MipMapCount);
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
		B3D_LOG(Error, LogTexture, "Destination position falls outside the destination texture.");
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
			B3D_LOG(Error, LogTexture, "Source volume falls outside the source texture.");
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
		B3D_LOG(Error, LogTexture, "Destination volume falls outside the destination texture.");
		return false;
	}

	const bool sourceHasMultipleSamples = sourceProperties.SampleCount > 1;
	const bool destinationHasMultipleSamples = destinationProperties.SampleCount > 1;

	if(sourceProperties.Usage.IsSet(TextureUsageFlag::DepthStencil) || destinationProperties.Usage.IsSet(TextureUsageFlag::DepthStencil))
	{
		B3D_LOG(Error, LogRenderBackend, "Texture copy/resolve isn't supported for depth-stencil textures.");
		return false;
	}

	bool needsResolve = sourceHasMultipleSamples && !destinationHasMultipleSamples;
	bool isMSCopy = sourceHasMultipleSamples || destinationHasMultipleSamples;
	if(!needsResolve && isMSCopy)
	{
		if(sourceProperties.SampleCount != destinationProperties.SampleCount)
		{
			B3D_LOG(Error, LogRenderBackend, "When copying textures their multisample counts must match. Ignoring copy.");
			return false;
		}
	}

	return true;
}

bool GpuCommandBuffer::BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation)
{
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Source or destination texture is null.");
		return false;
	}

	const TextureProperties& sourceProperties = source->GetProperties();
	const TextureProperties& destinationProperties = destination->GetProperties();

	if(blitInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, LogTexture, "Blit operation failed. Face count is zero.");
		return false;
	}

	if((blitInformation.SourceFace + blitInformation.FaceCount) > sourceProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Source face out of valid range.");
		return false;
	}

	if((blitInformation.DestinationFace + blitInformation.FaceCount) > destinationProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Destination face out of valid range.");
		return false;
	}

	if(blitInformation.SourceMip > sourceProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Source mip level out of valid range. Valid range is [0, {0}].", sourceProperties.MipMapCount);
		return false;
	}

	if(blitInformation.DestinationMip > destinationProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Destination mip level out of range. Valid range is [0, {0}].", destinationProperties.MipMapCount);
		return false;
	}

	if(sourceProperties.Usage.IsSet(TextureUsageFlag::DepthStencil) || destinationProperties.Usage.IsSet(TextureUsageFlag::DepthStencil))
	{
		B3D_LOG(Error, LogRenderBackend, "Texture blit isn't supported for depth-stencil textures.");
		return false;
	}

	return true;
}
}}
