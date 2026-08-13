//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuTimelineFence.h"
#include "B3DD3D12GpuDevice.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuTimelineFence::D3D12GpuTimelineFence(D3D12GpuDevice& device)
{
	const HRESULT hr = device.GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	if (FAILED(hr))
		B3D_LOG(Error, LogRenderBackend, "Failed to create D3D12 timeline fence.");
}

u64 D3D12GpuTimelineFence::GetCompletedValue() const
{
	// A fence that failed to create reports everything as complete so waits can't deadlock.
	if (mFence == nullptr)
		return ~u64(0);

	return mFence->GetCompletedValue();
}

void D3D12GpuTimelineFence::WaitInternal(u64 value)
{
	if (mFence == nullptr || mFence->GetCompletedValue() >= value)
		return;

	const HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (event == nullptr)
	{
		// Fall back to the base class' polling wait.
		GpuTimelineFence::WaitInternal(value);
		return;
	}

	if (SUCCEEDED(mFence->SetEventOnCompletion(value, event)))
		WaitForSingleObject(event, INFINITE);

	CloseHandle(event);
}
