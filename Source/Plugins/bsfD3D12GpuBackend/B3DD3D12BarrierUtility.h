//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12TextureLayout.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Image/B3DPixelUtility.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Native synchronization and access scope for one side of an enhanced barrier. */
		struct D3D12BarrierScope
		{
			D3D12_BARRIER_SYNC Sync = D3D12_BARRIER_SYNC_NONE; /**< Native execution synchronization scope. */
			D3D12_BARRIER_ACCESS Access = D3D12_BARRIER_ACCESS_NO_ACCESS; /**< Native memory access scope. */
		};

		/** Maps the engine's logical synchronization model onto D3D12 enhanced barriers. */
		class D3D12BarrierUtility
		{
		public:
			/** Converts a logical buffer use into a native enhanced-barrier scope. */
			static D3D12BarrierScope GetBufferScope(GpuStageFlags stages, GpuAccessFlags access, D3D12_RESOURCE_FLAGS resourceFlags);

			/** Converts a logical texture use into a native enhanced-barrier scope. */
			static D3D12BarrierScope GetTextureScope(GpuStageFlags stages, GpuAccessFlags access, GpuImageLayout layout, GpuTextureAspectFlags aspects);

			/** Returns a conservative native access scope compatible with @p layout. */
			static D3D12BarrierScope GetTextureLayoutScope(GpuImageLayout layout, const D3D12TextureLayout& nativeLayout, GpuTextureAspectFlags aspects, GpuStageFlags preferredStages = GpuStageFlag::None);

			/**
			 * Converts a core image layout into an aspect-aware enhanced-barrier layout for @p queueType. Shader-read
			 * layouts remain queue-neutral when @p allowConcurrentQueueReads is true.
			 */
			static D3D12TextureLayout GetTextureLayout(GpuImageLayout layout, GpuQueueType queueType, GpuTextureAspectFlags aspects = GpuTextureAspectFlag::Color, bool allowConcurrentQueueReads = false);

			/** Returns whether @p layout can be used by the specified queue type. */
			static bool IsTextureLayoutSupportedOnQueue(const D3D12TextureLayout& layout, GpuTextureAspectFlags aspects, GpuQueueType queueType);

			/** Returns whether the specified queue type can transition into or out of @p layout. */
			static bool CanTransitionTextureLayoutOnQueue(const D3D12TextureLayout& layout, GpuTextureAspectFlags aspects, GpuQueueType queueType);

		private:
			/** Converts a logical buffer use into native access flags allowed by @p resourceFlags. */
			static D3D12_BARRIER_ACCESS GetBufferAccess(GpuStageFlags stages, GpuAccessFlags access, D3D12_RESOURCE_FLAGS resourceFlags);

			/** Converts a logical texture use into native access flags compatible with @p layout and @p aspects. */
			static D3D12_BARRIER_ACCESS GetTextureAccess(GpuStageFlags stages, GpuAccessFlags access, GpuImageLayout layout, GpuTextureAspectFlags aspects);

			/** Translates logical stages directly; use @ref GetTextureSync when native texture access can impose additional synchronization. */
			static D3D12_BARRIER_SYNC GetStageSync(GpuStageFlags stages);

			/** Combines logical stages with the synchronization required by resolved native texture @p access. */
			static D3D12_BARRIER_SYNC GetTextureSync(GpuStageFlags stages, D3D12_BARRIER_ACCESS access);
		};

		/** @} */
	} // namespace render
} // namespace b3d
