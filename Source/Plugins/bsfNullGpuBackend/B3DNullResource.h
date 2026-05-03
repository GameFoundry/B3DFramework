//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d::render
{
	class NullResourceManager;

	/** @addtogroup NullGpuBackend
	 *  @{
	 */

	/** Null-backend GPU resource. Inherits the cross-backend lifetime model from IGpuResource with no extras. */
	class NullResource : public IGpuResource
	{
	public:
		NullResource(NullResourceManager* owner, const StringView& name = "");
	};

	/** @} */
} // namespace b3d::render
