//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RTTI/B3DTransformRTTI.h"
#include "Scene/B3DSceneObjectFragments.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template<>
	struct RTTIPlainType<ecs::LocalTransform> : RTTIPlainTypeHelper<ecs::LocalTransform, TID_LocalTransform, 255, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(ecs::LocalTransform& object, Processor& processor)
		{
			RTTIPlainType<Transform>::RTTIEnumerateFields(object, processor);
		}
	};

	template<>
	struct RTTIPlainType<ecs::WorldTransform> : RTTIPlainTypeHelper<ecs::WorldTransform, TID_WorldTransform, 255, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(ecs::WorldTransform& object, Processor& processor)
		{
			RTTIPlainType<Transform>::RTTIEnumerateFields(object, processor);
		}
	};

	template<>
	struct RTTIPlainType<ecs::HierarchyDepth> : RTTIPlainTypeHelper<ecs::HierarchyDepth, TID_HierarchyDepth, 255, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(ecs::HierarchyDepth& object, Processor& processor)
		{
			processor(object.Depth);
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
