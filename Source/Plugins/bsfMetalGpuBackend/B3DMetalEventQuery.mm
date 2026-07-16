//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalEventQuery.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "Profiling/B3DRenderStats.h"

namespace b3d
{
	namespace render
	{
		struct MetalEventQuery::Impl
		{
			id<MTLSharedEvent> Event = nil;
		};

		MetalEventQuery::MetalEventQuery(MetalGpuDevice& gpuDevice)
			: mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
		{
			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
			id<MTLDevice> device = gpuDevice.GetMetalDevice();
			if (device != nil)
				mImpl->Event = [device newSharedEvent];
		}

		MetalEventQuery::~MetalEventQuery()
		{
			if (mImpl)
			{
#if !__has_feature(objc_arc)
				[mImpl->Event release];
#endif
				mImpl->Event = nil;
			}

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
		}

		void MetalEventQuery::Begin(GpuCommandBuffer& commandBuffer)
		{
			if (mImpl->Event == nil)
				return;

			auto& metalCB = static_cast<MetalGpuCommandBuffer&>(commandBuffer);

			// Command-buffer event commands cannot be encoded while a Metal encoder is active. The
			// command buffer therefore closes compute/blit encoders immediately, and defers signals
			// from a render pass until its encoder ends, matching Vulkan's render-pass behavior.
			const u64 nextValue = mExpectedValue.load(std::memory_order_relaxed) + 1;
			if (metalCB.EncodeSignalEvent(mImpl->Event, nextValue))
				mExpectedValue.store(nextValue, std::memory_order_release);
		}

		bool MetalEventQuery::IsReady() const
		{
			if (mImpl->Event == nil)
				return false;

			const u64 target = mExpectedValue.load(std::memory_order_acquire);
			return target != 0 && mImpl->Event.signaledValue >= target;
		}
	} // namespace render
} // namespace b3d
