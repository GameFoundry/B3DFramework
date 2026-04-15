//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "ECS/B3DEntity.h"

namespace b3d::ecs
{
	class Registry;

	/** @addtogroup ECS
	 *  @{
	 */

	/** Interface for objects that own an ECS entity. */
	class B3D_EXPORT IECSEntityOwner
	{
	public:
		virtual ~IECSEntityOwner() = default;

		/** Returns the ECS registry this entity belongs to, or null if not part of a registry. */
		virtual Registry* GetECSRegistry() const = 0;

		/** Returns the ECS entity handle. */
		virtual Entity GetECSEntity() const = 0;

		/** Creates an ECS entity in the provided registry and adds default fragments. Asserts if entity already exists. */
		virtual void CreateECSEntity(Registry* registry) = 0;
	};

	/** @} */
} // namespace b3d::ecs
