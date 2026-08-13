//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuTimelineFence.h"

namespace b3d::render
{
	class D3D12GpuDevice;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/**
	 * DirectX 12 implementation of a GPU timeline fence, backed by an ID3D12Fence. The fence is signaled from a
	 * queue (via ID3D12CommandQueue::Signal) when a submit it was attached to completes.
	 */
	class D3D12GpuTimelineFence : public GpuTimelineFence
	{
	public:
		D3D12GpuTimelineFence(D3D12GpuDevice& device);

		u64 GetCompletedValue() const override;

		/** Returns the native fence object, to be signaled via ID3D12CommandQueue::Signal. */
		ID3D12Fence* GetD3D12Fence() const { return mFence.Get(); }

	protected:
		void WaitInternal(u64 value) override;

	private:
		ComPtr<ID3D12Fence> mFence;
	};

	/** @} */
} // namespace b3d::render
