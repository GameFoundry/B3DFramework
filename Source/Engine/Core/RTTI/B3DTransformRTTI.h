//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Math/B3DTransform.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template<>
	struct RTTIPlainType<Transform> : RTTIPlainTypeHelper<Transform, TID_Transform, 255, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(Transform& object, Processor& processor)
		{
			processor(object.mPosition);
			processor(object.mRotation);
			processor(object.mScale);
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
