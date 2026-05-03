//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullResource.h"
#include "B3DNullResourceManager.h"

namespace b3d::render
{
	NullResource::NullResource(NullResourceManager* owner, const StringView& name)
		: IGpuResource(owner, name)
	{}
} // namespace b3d::render
