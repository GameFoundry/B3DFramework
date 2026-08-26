//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
namespace b3d
{
	class D3D12BarrierTestSuite;
} // namespace b3d::render

namespace b3d::render
{
	class D3D12BarrierHelper;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** Stores resolved D3D12 enhanced barriers and records them in insertion order. */
	class D3D12BarrierBatch
	{
	public:
		/** Adds an global barrier. */
		void AddGlobalBarrier(const D3D12_GLOBAL_BARRIER& barrier);

		/** Adds an whole-resource buffer barrier. */
		void AddBufferBarrier(const D3D12_BUFFER_BARRIER& barrier);

		/** Adds an subresource texture barrier and returns its storage index. */
		u32 AddTextureBarrier(const D3D12_TEXTURE_BARRIER& barrier);

		/** Returns whether the batch contains no barriers. */
		bool IsEmpty() const;

		/** Records every barrier on @p commandList without clearing the batch. */
		void Record(ID3D12GraphicsCommandList7& commandList) const;

		/** Removes every barrier from the batch. */
		void Clear();

	private:
		friend class ::b3d::D3D12BarrierTestSuite;
		friend class D3D12BarrierHelper;

		/** Replaces an already-added native texture barrier without changing its position. */
		void ReplaceTextureBarrier(u32 barrierIndex, const D3D12_TEXTURE_BARRIER& barrier);

		/** Consecutive barriers of one native type. */
		struct BarrierGroup
		{
			D3D12_BARRIER_TYPE Type = D3D12_BARRIER_TYPE_GLOBAL; /**< Native barrier type. */
			u32 FirstBarrier = 0; /**< First entry in the corresponding typed array. */
			u32 BarrierCount = 0; /**< Number of consecutive entries in the group. */
		};

		/** Extends the latest group of @p type or starts a new ordered group at @p barrierIndex. */
		void AddToGroup(D3D12_BARRIER_TYPE type, u32 barrierIndex);

		TInlineArray<BarrierGroup, 8> mBarrierGroups; /**< Consecutive native barrier groups in API insertion order. */
		TInlineArray<D3D12_GLOBAL_BARRIER, 8> mGlobalBarriers; /**< Barriers that cannot be represented for a specific resource. */
		TInlineArray<D3D12_BUFFER_BARRIER, 8> mBufferBarriers; /**< Whole-resource buffer barriers. */
		TInlineArray<D3D12_TEXTURE_BARRIER, 8> mTextureBarriers; /**< Image layout and access barriers. */
	};

	/** @} */
}
