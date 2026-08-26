//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class D3D12BarrierHelper;
	struct D3D12FramebufferAttachment;

	/** Resource usage for one framebuffer attachment, accumulates framebuffer and/or shader usage while preparing a render pass. */
	struct D3D12RenderPassAttachmentUsage
	{
		D3D12Image* Image = nullptr; /**< Image bound by the attachment. */
		GpuTextureSubresourceRange Range; /**< Subresources covered by this attachment aspect. */
		GpuResourceUseFlags AttachmentUsage; /**< Attachment role and fixed-function stages. */
		GpuAccessFlags AttachmentAccess; /**< Access performed through the framebuffer attachment. */
		GpuImageLayout AttachmentLayout = GpuImageLayout::Undefined; /**< Layout used when the aspect isn't sampled. */
		TOptional<GpuImageLayout> ShaderReadLayout; /**< Layout supporting simultaneous attachment and shader reads, if available. */
		GpuResourceUseFlags ShaderUsage; /**< Shader stages that sample this attachment aspect. */
	};

	/** Resource usage for framebuffer attachments, accumulates framebuffer and/or shader usage while preparing a render pass. */
	struct D3D12RenderPassResourceUsage
	{
		D3D12RenderPassResourceUsage() = default;
		D3D12RenderPassResourceUsage(const D3D12FramebufferAttachment* attachments, u32 attachmentCount, RenderSurfaceMask readOnlyMask);

		TInlineArray<D3D12RenderPassAttachmentUsage, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> Attachments;
	};

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<D3D12BarrierHelper>;

	/** D3D12-specific resource tracker. Adds render-target integration to the core tracker. */
	class D3D12ResourceTracker : public TGpuResourceTracker<D3D12BarrierHelper>
	{
	public:
		/** Tracks a logical buffer use and write serialization for its shared physical page. */
		void TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset = 0);

		/** Tracks shader reads of @p image. If the shader use overlaps any usage in @p renderPassUsage, the use is folded into that entry rather than tracking it (assuming it will be tracked when attachment is tracked). Otherwise the use is tracked immediately. */
		 void TrackSampledImageUsage(D3D12Image* image, const GpuTextureSubresourceRange& subresourceRange, GpuResourceUseFlags useFlags, D3D12BarrierHelper& barrierHelper, D3D12RenderPassResourceUsage* renderPassUsage = nullptr);

		/**
		 * Registers each attachment of a render target as used on the associated command buffer, queuing any required
		 * transitions into @p barrierHelper (execute them before the pass records its work).
		 */
		void TrackRenderTargetUsage(const D3D12RenderPassResourceUsage& renderPassUsage, D3D12BarrierHelper& barrierHelper);

		/** Clears framebuffer-use flags for every tracked subresource of @p image when a render pass ends. */
		void ClearRenderTargetFlagsForImage(D3D12Image* image);

		/** Clears shader-use flags for every subresource touched during the current render pass. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();
	};

	/** @} */
} // namespace b3d::render
