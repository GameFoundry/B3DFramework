//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12BarrierUtility.h"
#include "B3DD3D12TextureLayout.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/B3DGpuTextureSubresource.h"

namespace b3d
{
	class D3D12BarrierTestSuite;
} // namespace b3d::render

namespace b3d::render
{
	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** Builds and records one batch of D3D12 enhanced barriers. */
	class D3D12BarrierBatch
	{
	public:
		/** Adds a conservative whole-resource buffer barrier. */
		void AddBufferBarrier(ID3D12Resource* resource, const GpuBarrierScope& scope);

		/**
		 * Adds or merges a texture barrier. A native UNDEFINED before layout discards previous contents.
		 * Returns the original logical layout retained by a merged barrier.
		 */
		GpuImageLayout AddTextureBarrier(ID3D12Resource* resource, const GpuTextureSubresourceRange& range, const GpuBarrierScope& scope, GpuImageLayout logicalBeforeLayout, GpuImageLayout logicalAfterLayout, const D3D12TextureLayout& nativeBeforeLayout, const D3D12TextureLayout& nativeAfterLayout);

		/** Returns whether the batch contains no barriers. */
		bool IsEmpty() const;

		/** Records every barrier on @p commandList without clearing the batch. */
		void Record(ID3D12GraphicsCommandList7& commandList) const;

		/** Removes every barrier from the batch. */
		void Clear();

	private:
		friend class ::b3d::D3D12BarrierTestSuite;

		/** Logical and native state retained while an exact texture-subresource barrier is being merged. */
		struct TextureBarrierEntry
		{
			D3D12_TEXTURE_BARRIER Barrier{}; /**< Native barrier rebuilt after every merge. */
			GpuBarrierScope Scope; /**< Combined logical source and destination scopes. */
			GpuImageLayout LogicalBeforeLayout = GpuImageLayout::Undefined; /**< Original logical layout. */
			GpuImageLayout LogicalAfterLayout = GpuImageLayout::Undefined; /**< Latest required logical layout. */
			GpuTextureAspectFlags Aspects{}; /**< Aspect represented by the native plane. */
			bool EmitsNativeBarrier = false; /**< Whether Record() needs to emit this entry. */
		};

		/** Rebuilds the native barrier from the entry's merged logical scope and final layout. */
		static void RebuildTextureBarrier(TextureBarrierEntry& entry);

		TInlineArray<D3D12_BUFFER_BARRIER, 8> mBufferBarriers; /**< Whole-resource buffer barriers. */
		TInlineArray<TextureBarrierEntry, 8> mTextureBarriers; /**< Image layout and access barriers. */
	};

	/** @} */
}
