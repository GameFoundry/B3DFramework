//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Utility-Core-Internal
	 *  @{
	 */

	/** Contains information about a resource dependency, including the dependant resource and number of references to it. */
	struct ResourceDependency
	{
		ResourceDependency()
			:numReferences(0)
		{ }

		HResource resource;
		UINT32 numReferences;
	};

	/** Static class containing various utility methods that do not fit anywhere else. */
	class BS_CORE_EXPORT Utility
	{
	public:
		/**
		 * Finds all resources referenced by the specified object.
		 *
		 * @param[in]	object		Object to search for resource dependencies.
		 * @param[in]	recursive	Determines whether or not child objects will also be searched (if the object has any
		 *							children).
		 * @return					A list of unique, non-null resources.
		 */
		static Vector<ResourceDependency> findResourceDependencies(IReflectable& object, bool recursive = true);

		/**
		 * Finds all components of a specific type on a scene object and any of its children.
		 * 
		 * @param[in]	object		Object which to search for components. All children will be searched as well.
		 * @param[in]	typeId		RTTI type ID of the component type to search for.
		 * @return					A list of all components of the specified type.
		 */
		static Vector<HComponent> findComponents(const HSceneObject& object, UINT32 typeId);

		/** Calculates how deep in the scene object hierarchy is the provided object. Zero means root. */
		static UINT32 getSceneObjectDepth(const HSceneObject& so);

	private:
	};

	/** @} */
}