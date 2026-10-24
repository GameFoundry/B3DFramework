//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Image/B3DTexture.h"

namespace b3d::render
{
	/** Command buffer pools, access states and barriers shared by the GPU tests that record their own barriers. */
	class GpuBarrierTestUtility
	{
	public:
		/** Creates a command buffer pool for @p queueType on the calling thread. */
		static TShared<GpuCommandBufferPool> CreateCommandBufferPool(GpuDevice& device, GpuQueueType queueType = GQT_GRAPHICS)
		{
			return device.CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(queueType));
		}

		/**
		 * Returns the resting state of @p texture on @p queueType, with its contents discarded. The source of the first barrier of a new
		 * texture whose contents the next access overwrites.
		 */
		static GpuAccessState GetDiscardedRestingState(const Texture& texture, GpuQueueType queueType = GQT_GRAPHICS)
		{
			GpuAccessState state = GpuBackendUtility::GetRestingState(texture, queueType);
			state.Layout = GpuImageLayout::Undefined;

			return state;
		}

		/** Returns the state of a texture a copy writes. @p compression describes the encoding the copy leaves the texture in. */
		static GpuAccessState GetTransferWriteState(GpuCompressionHint compression = GpuCompressionHint::Unknown)
		{
			return GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination, compression);
		}

		/** Returns the state of a texture a copy reads. */
		static GpuAccessState GetTransferReadState()
		{
			return GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		}

		/**
		 * Returns the state of a color attachment, or of a depth/stencil attachment if @p depth is true, that a render pass reads and
		 * writes. @p compression describes the encoding the render pass leaves the texture in.
		 */
		static GpuAccessState GetAttachmentState(bool depth, GpuCompressionHint compression = GpuCompressionHint::Unknown)
		{
			if(depth)
				return GpuAccessState(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::DepthStencilAttachment, compression);

			return GpuAccessState(GpuStageFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::ColorAttachment, compression);
		}

		/** Returns a barrier that makes the host writes of @p buffer visible to copies that read it. */
		static GpuBufferBarrier CreateUploadBarrier(const TShared<GpuBuffer>& buffer)
		{
			return GpuBufferBarrier(buffer, GpuAccessState(GpuStageFlag::Host, GpuAccessFlag::Write), GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Read));
		}

		/** Returns a barrier that makes the copy writes of @p buffer visible to host reads. */
		static GpuBufferBarrier CreateReadbackBarrier(const TShared<GpuBuffer>& buffer)
		{
			return GpuBufferBarrier(buffer, GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Write), GpuAccessState(GpuStageFlag::Host, GpuAccessFlag::Read));
		}
	};
} // namespace b3d::render
