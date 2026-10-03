//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class D3D12BarrierHelper;
	class D3D12ResourceTracker;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<D3D12ResourceTracker, D3D12BarrierHelper>;

	/** D3D12-specific resource tracker. Adds physical buffer-page tracking to the core tracker. */
	class D3D12ResourceTracker : public TGpuResourceTracker<D3D12ResourceTracker, D3D12BarrierHelper>
	{
	public:
		/**
		 * Encoding of GpuImageLayout::ShaderReadOnly in GpuImageNativeState::Layout, for images that can rest (see TGpuResourceTracker).
		 * Images that can rest use the queue-independent layouts.
		 */
		static constexpr u32 kRestingNativeLayout = D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;

		/** Tracks a logical buffer use and write serialization for its shared physical page. */
		void TrackBufferAccess(IGpuBufferResource* buffer, GpuStageFlags stages, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset = 0);
	};

	/** @} */
} // namespace b3d::render
