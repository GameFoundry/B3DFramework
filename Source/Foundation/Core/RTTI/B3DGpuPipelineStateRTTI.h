//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RenderAPI/B3DGpuPipelineState.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(RenderTargetBlendStateInformation, TID_RenderTargetBlendStateInformation)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(BlendStateInformation, TID_BlendStateInformation)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(RasterizerStateInformation, TID_RasterizerStateInformation);
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(DepthStencilStateInformation, TID_DepthStencilStateInformation);

	/** @} */
	/** @endcond */
} // namespace b3d
