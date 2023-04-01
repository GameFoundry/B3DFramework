//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RenderAPI/BsGpuPipelineState.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(RenderTargetBlendStateInformation)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(BlendStateInformation, TID_BlendStateInformation)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(RasterizerStateInformation, TID_RasterizerStateInformation);
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(DepthStencilStateInformation, TID_DepthStencilStateInformation);

	/** @} */
	/** @endcond */
} // namespace bs
