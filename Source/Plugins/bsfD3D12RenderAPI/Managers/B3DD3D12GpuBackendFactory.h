//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Managers/B3DGpuBackendManager.h"

namespace b3d
{
	/** @addtogroup D3D12RenderAPI
	 *  @{
	 */

	/**	Handles creation of the D3D12 GPU backend. */
	class D3D12GpuBackendFactory : public GpuBackendFactory
	{
	public:
		static constexpr const char* SystemName = "bsfD3D12RenderAPI";

		void Create() override;
		const char* Name() const override { return SystemName; }
	};

	/** @} */
} // namespace b3d
