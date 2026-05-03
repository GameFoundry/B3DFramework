//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalResource.h"
#include "B3DMetalResourceManager.h"

namespace b3d::render
{
	MetalResource::MetalResource(MetalResourceManager* owner, const StringView& name)
		: IGpuResource(owner, name)
	{}
} // namespace b3d::render
