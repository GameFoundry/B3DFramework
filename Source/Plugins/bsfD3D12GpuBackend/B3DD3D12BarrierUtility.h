//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Maps the engine's logical resource-use/access/layout model onto the fused D3D12 resource-state model.
		 *
		 * Unlike Vulkan (which tracks image layout and cache-access masks independently), D3D12 collapses both into a
		 * single @c D3D12_RESOURCE_STATES value per (sub)resource. GpuImageLayout maps onto that state space almost
		 * 1:1, so images track real layouts through the shared resource tracker and D3D12BarrierHelper translates
		 * them (plus the access flags that disambiguate e.g. depth read vs write) into native states here. Buffers
		 * carry no layout in the core model; their states are derived from usage or stage/access flags instead.
		 */
		class D3D12BarrierUtility
		{
		public:
			/**
			 * Translates a logical resource usage into the matching D3D12 resource state.
			 *
			 * @param usage			How the resource is (or will be) used. Stage bits are honored to pick between
			 *						PIXEL and NON_PIXEL shader-resource states.
			 * @param access		Whether the access is a read or a write. Determines UAV vs SRV and depth read vs write.
			 * @param isTexture		True for image resources, false for buffers. Buffers never take attachment/SRV
			 *						states that only apply to images (e.g. depth), and take VERTEX_AND_CONSTANT_BUFFER
			 *						for uniform usage.
			 * @return				The equivalent D3D12 resource state. Falls back to COMMON if usage is Undefined.
			 */
			static D3D12_RESOURCE_STATES GetResourceState(GpuResourceUseFlags usage, GpuAccessFlags access, bool isTexture);

			/**
			 * Translates an image layout into the matching D3D12 resource state. Used for barriers that carry an explicit
			 * destination layout (render-pass attachment transitions, present, etc.). Returns COMMON for Undefined.
			 *
			 * @param layout		Destination image layout.
			 * @param access		Read/write intent, used to disambiguate depth read vs write and general read vs UAV.
			 */
			static D3D12_RESOURCE_STATES GetResourceStateFromLayout(GpuImageLayout layout, GpuAccessFlags access);

			/**
			 * Translates pipeline-stage + access flags into the matching D3D12 buffer resource state. Used by the
			 * barrier helper's low-level hooks, which only see stage masks (the shared tracker converts usage flags
			 * to stages before invoking them).
			 */
			static D3D12_RESOURCE_STATES GetBufferStateFromStages(GpuStageFlags stages, GpuAccessFlags access);

			/** Returns true if the state represents a read-only (or read-combinable) D3D12 state. UAV/RT/DEPTH_WRITE/COPY_DEST are not. */
			static bool IsReadOnlyState(D3D12_RESOURCE_STATES state);
		};

		/** @} */
	} // namespace render
} // namespace b3d
