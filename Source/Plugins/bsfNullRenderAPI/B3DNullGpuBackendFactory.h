//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/B3DGpuBackendManager.h"

namespace b3d
{
	/** @addtogroup NullGpuBackend 
	 *  @{
	 */

	/**	Handles creation of the NullGpuBackend. */
	class NullGpuBackendFactory : public GpuBackendFactory
	{
	public:
		static constexpr const char* SystemName = "bsfNullRenderAPI";

		void Create() override;
		const char* Name() const override { return SystemName; }
	};

	/** @} */
} // namespace b3d
