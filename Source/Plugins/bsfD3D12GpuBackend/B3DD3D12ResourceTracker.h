//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class D3D12BarrierHelper;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<D3D12BarrierHelper>;

	/**
	 * A single render-pass attachment resolved to the tracked image + subresource it binds. Built by the command
	 * buffer from the framebuffer, so the tracker never needs to know which kind of render target produced it.
	 */
	struct D3D12RenderTargetAttachment
	{
		D3D12Image* Image = nullptr; /**< Tracked image backing the attachment. */
		TextureSurface Surface;      /**< Face/mip selection of the attachment within the image. */
		u32 ColorIndex = 0;          /**< Color attachment slot (its RenderSurfaceMask bit index); unused when IsDepthStencil. */
		bool IsDepthStencil = false; /**< True for the depth/stencil attachment. */
	};

	/**
	 * D3D12-specific resource tracker. Inherits the backend-agnostic tracking machinery from TGpuResourceTracker and
	 * adds the render-target glue plus buffer native-state upkeep.
	 *
	 * Images are tracked with real GpuImageLayouts (they map almost 1:1 onto D3D12_RESOURCE_STATES), so the shared
	 * layout-change detection drives the per-subresource transitions - including read->read transitions like
	 * COPY_SOURCE -> SRV that carry no Vulkan-style hazard. Buffers have no layout in the core model, so
	 * TrackBufferUsage is shadowed here to keep the native buffer state in sync via the barrier helper.
	 */
	class D3D12ResourceTracker : public TGpuResourceTracker<D3D12BarrierHelper>
	{
	public:
		/**
		 * Registers each attachment of a render target as used on the associated command buffer, queuing any required
		 * transitions into @p barrierHelper (execute them before the pass records its work).
		 */
		void TrackRenderTargetUsage(const D3D12RenderTargetAttachment* attachments, u32 attachmentCount, RenderSurfaceMask readOnlyMask, D3D12BarrierHelper& barrierHelper);

		/**
		 * Returns the attachments that must be treated as read-only this pass because a shader also samples them,
		 * unioned with @p explicitReadOnlyMask's depth/stencil bits.
		 */
		RenderSurfaceMask GetRenderTargetReadOnlyMask(const D3D12RenderTargetAttachment* attachments, u32 attachmentCount, RenderSurfaceMask explicitReadOnlyMask) const;

		/** Clears the framebuffer-use flags of every subresource of @p image. Called for each attachment when a render pass ends. */
		void ClearRenderTargetFlagsForImage(D3D12Image* image);

		/** Clears the shader-use flags of every subresource touched during the current render pass. Called when a render pass ends. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();

		/**
		 * Shadow of the base TrackBufferUsage that additionally keeps the buffer's native D3D12 state in sync with
		 * the requested usage (buffers carry no core layout, so read->read state changes would otherwise go
		 * unnoticed). All D3D12 call sites must use this overload.
		 */
		void TrackBufferUsage(D3D12Buffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset = 0);

		/**
		 * Shadow of the base TrackImageUsage that additionally keeps each tracked subresource's native D3D12 state
		 * in sync with its required layout. The first use of a subresource on a command buffer seeds the shared
		 * tracker's layout without invoking the barrier hooks (Vulkan reconciles the image's global layout at submit
		 * time instead); D3D12 advances native states at record time, so the initial transition is emitted here.
		 * All D3D12 call sites must use this overload.
		 */
		void TrackImageUsage(D3D12Image* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper);
	};

	/** @} */
} // namespace b3d::render
