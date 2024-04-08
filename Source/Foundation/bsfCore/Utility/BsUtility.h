//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @addtogroup Utility-Core-Internal
	 *  @{
	 */

	/** Contains information about a resource dependency, including the dependant resource and number of references to it. */
	struct ResourceDependency
	{
		ResourceDependency() = default;

		HResource Resource;
		u32 NumReferences = 0;
	};

	/** Static class containing various utility methods that do not fit anywhere else. */
	class B3D_CORE_EXPORT Utility
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
		static Vector<ResourceDependency> FindResourceDependencies(IReflectable& object, bool recursive = true);

		/**
		 * Finds all components of a specific type on a scene object and any of its children.
		 *
		 * @param[in]	object		Object which to search for components. All children will be searched as well.
		 * @param[in]	typeId		RTTI type ID of the component type to search for.
		 * @return					A list of all components of the specified type.
		 */
		static Vector<HComponent> FindComponents(const HSceneObject& object, u32 typeId);

		/** Calculates how deep in the scene object hierarchy is the provided object. Zero means root. */
		static u32 GetSceneObjectDepth(const HSceneObject& so);
	};

	/** Provides extra information and maintains state during serialization of various RTTI types in the core. */
	struct B3D_CORE_EXPORT CoreSerializationContext : SerializationContext
	{
		/**
		 * By default deserialization will persist existing IDs for game objects. Setting this to false will generate brand new IDs on deserialization.
		 *
		 * e.g. When loading a scene or a prefab resource, we wish to persist the original IDs in its internal game object collection.
		 * But when instantiating a copy of a prefab in the scene, we want to to have new unique IDs.
		 */
		bool PreserveGameObjectIds = true; 
		bool IsGameObjectDeserializationActive = false;
		SPtr<GameObjectCollection> GameObjectCollection; /**< If deserializing game objects, collection to place them in. */
		UnorderedMap<UUID, UUID> GameObjectIdRemapping; /**< If provided, game object IDs will be remapped to provided values. */

		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;
	};

	/** @} */
} // namespace bs
