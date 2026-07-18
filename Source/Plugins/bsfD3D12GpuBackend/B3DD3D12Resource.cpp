//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Resource.h"
#include "B3DD3D12ResourceManager.h"

using namespace b3d;
using namespace b3d::render;

template<class TBase>
D3D12GpuDevice& TD3D12Resource<TBase>::GetDevice() const
{
	return mOwner->GetDevice();
}

namespace b3d::render
{
	template class TD3D12Resource<IGpuResource>;
	template class TD3D12Resource<IGpuBufferResource>;
	template class TD3D12Resource<IGpuImageResource>;
} // namespace b3d::render
