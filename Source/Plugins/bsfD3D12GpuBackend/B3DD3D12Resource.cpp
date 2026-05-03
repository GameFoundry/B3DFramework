//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Resource.h"
#include "B3DD3D12ResourceManager.h"

namespace b3d::render
{
	D3D12Resource::D3D12Resource(D3D12ResourceManager* owner, const StringView& name)
		: IGpuResource(owner, name)
	{}
} // namespace b3d::render
