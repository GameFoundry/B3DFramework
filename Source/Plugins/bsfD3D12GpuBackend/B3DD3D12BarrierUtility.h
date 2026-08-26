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

		/** Resource properties that affect the native layout selected for a logical texture layout. */
		struct D3D12TextureLayoutOptions
		{
			bool AllowConcurrentQueueReads = false; /**< Whether shader reads may overlap on graphics and compute queues. */
			bool IsPresentable = false; /**< Whether the texture is a swap-chain image that can remain in COMMON for copies. */
		};

		/** Maps the engine's logical synchronization model onto D3D12 enhanced barriers. */
		class D3D12BarrierUtility
		{
		public:
			/** Translates logical execution stages without deriving an access scope. */
			static D3D12_BARRIER_SYNC GetStageSync(GpuStageFlags stages);

			/** Converts a logical buffer use into a native enhanced-barrier scope. */
			static D3D12BarrierScope GetBufferScope(GpuStageFlags stages, GpuAccessFlags access, D3D12_RESOURCE_FLAGS resourceFlags);

			/** Converts a logical texture use into a native enhanced-barrier scope. */
			static D3D12BarrierScope GetTextureScope(GpuStageFlags stages, GpuAccessFlags access, GpuImageLayout layout, GpuTextureAspectFlags aspects);

			/** Builds a native global barrier using buffer access semantics. */
			static D3D12_GLOBAL_BARRIER GetGlobalBufferBarrier(D3D12_RESOURCE_FLAGS resourceFlags, const GpuBarrierScope& scope, GpuStageFlags precedingBarrierDestinationStages);

			/** Builds a whole-resource native buffer barrier. */
			static D3D12_BUFFER_BARRIER GetBufferBarrier(ID3D12Resource* resource, const GpuBarrierScope& scope, GpuStageFlags precedingBarrierDestinationStages);

			/** Builds a native texture barrier for one resolved, single-aspect subresource range. */
			static D3D12_TEXTURE_BARRIER GetTextureBarrier(ID3D12Resource* resource, const GpuTextureSubresourceRange& range, const GpuBarrierScope& scope, GpuImageLayout logicalBeforeLayout, GpuImageLayout logicalAfterLayout, const D3D12TextureLayout& nativeBeforeLayout, const D3D12TextureLayout& nativeAfterLayout, GpuStageFlags precedingBarrierDestinationStages = GpuStageFlag::None);

			/** Returns a conservative native access scope compatible with @p layout. */
			static D3D12BarrierScope GetTextureLayoutScope(GpuImageLayout layout, const D3D12TextureLayout& nativeLayout, GpuTextureAspectFlags aspects, GpuStageFlags preferredStages = GpuStageFlag::None);

			/** Converts a logical image layout and explicit resource properties into an aspect-aware native layout. */
			static D3D12TextureLayout TranslateTextureLayout(GpuImageLayout layout, GpuQueueType queueType, const D3D12TextureLayoutOptions& options);

			/** Returns whether @p layout can be used by the specified queue type. */
			static bool IsTextureLayoutSupportedOnQueue(const D3D12TextureLayout& layout, GpuTextureAspectFlags aspects, GpuQueueType queueType);

			/** Returns whether the specified queue type can transition into or out of @p layout. */
			static bool CanTransitionTextureLayoutOnQueue(const D3D12TextureLayout& layout, GpuTextureAspectFlags aspects, GpuQueueType queueType);

		private:
			/** 
			 * Extends the required 'before' sync scope of barrier represented by @p beforeScope, by chaining the 'after' stages from @p precedingBarrierDestinationStages. 
			 * Potentially expands the sync scope in order to make sure they are valid for the provided access scope.
			 */
			static D3D12_BARRIER_SYNC GetChainedSyncBefore(const D3D12BarrierScope& beforeScope, GpuStageFlags precedingBarrierDestinationStages);

			/** Converts a logical buffer use into native access flags allowed by @p resourceFlags. */
			static D3D12_BARRIER_ACCESS GetBufferAccess(GpuStageFlags stages, GpuAccessFlags access, D3D12_RESOURCE_FLAGS resourceFlags);

			/** Converts a logical texture use into native access flags compatible with @p layout and @p aspects. */
			static D3D12_BARRIER_ACCESS GetTextureAccess(GpuStageFlags stages, GpuAccessFlags access, GpuImageLayout layout, GpuTextureAspectFlags aspects);

			/** Returns the synchronization compatible with resolved native texture @p access. */
			static D3D12_BARRIER_SYNC GetTextureSync(GpuStageFlags stages, D3D12_BARRIER_ACCESS access);
		};

		/** @} */
	} // namespace render
} // namespace b3d
