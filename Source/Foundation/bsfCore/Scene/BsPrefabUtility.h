//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsGameObject.h"

namespace bs
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Contains information required for linking a game object with an object within a prefab it is linked to. */
	struct PrefabLinkInformation
	{
		PrefabLinkInformation(const UUID& prefabObjectId = UUID::kEmpty, const UUID& prefabResourceId = UUID::kEmpty)
			: PrefabObjectId(prefabObjectId), PrefabResourceId(prefabResourceId)
		{ }

		UUID PrefabObjectId; /**< Id of the game object in the prefab. */
		UUID PrefabResourceId; /**< Id of the prefab resource. */
	};

	/** Performs various prefab specific operations. */
	class B3D_CORE_EXPORT PrefabUtility
	{
	public:
		/**
		 * Remove any instance specific changes to the object or its hierarchy from the provided prefab instance and
		 * restore it to the exact copy of the linked prefab.
		 */
		static void RevertToPrefab(const HSceneObject& sceneObject);

		/**
		 * Updates all of the objects belonging to the same prefab instance as the provided object (if any). The update
		 * will apply any changes from the linked prefab to the hierarchy (if any).
		 */
		static void UpdateFromPrefab(const HSceneObject& sceneObject);

		/**
		 * Assigns the provided prefab resource ID to the provided scene object hierarchy recursively. If a scene object
		 * that is part of another prefab is reached, iteration stops. Prefab instance IDs are assigned to their corresponding
		 * game object IDs (i.e. objects reference themselves).
		 */
		static void AssignPrefabResourceId(const HSceneObject& sceneObject, const UUID& newPrefabResourceId);

		/**
		 * Assigns prefab object and resource IDs to the provided scene object hierarchy. The object IDs are retrieved from the provided
		 * @p prefabRoot hierarchy, which must exactly match @p instanceRoot hierarchy (i.e. @p instanceRoot should be a clone of @p prefabRoot).
		 * All objects will be assigned @p prefabResourceId as the prefab resource ID.
		 */
		static void AssignPrefabInstanceIds(const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabResourceId);

		/**
		 * Clears all prefab IDs in the provided object and its children (includes both the prefab object and prefab resource IDs).
		 *
		 * @note	If any of its children belong to another prefab they will not be cleared.
		 */
		static void ClearPrefabIds(const HSceneObject& sceneObject);

		/**
		 * Updates the internal prefab delta data by recording the difference between the current values in the provided
		 * prefab instance and its prefab.
		 *
		 * @note
		 * If the provided object contains any child prefab instances, this will be done recursively for them as well.
		 */
		static void RecordPrefabDelta(const HSceneObject& sceneObject);

		/**
		 * Iterates over the provided scene object hierarchy and records a map of game object id -> { prefab object id, prefab resource id } for each
		 * scene object and component in the hierarchy.
		 *
		 * @param		sceneObject			Scene object at which to start iterating
		 * @param		visitChildPrefabs	If false, iteration into child scene objects will stop if they belong to another prefab. Otherwise
		 *									we iterate until leaf of the hierarchy is reached.
		 * @return							Generated game object id -> { prefab object id, prefab resource id } map.
		 */
		static UnorderedMap<UUID, PrefabLinkInformation> GetInstanceToPrefabLinkInformationMap(const HSceneObject& sceneObject, bool visitChildPrefabs);
	};

	/** @} */
} // namespace bs
