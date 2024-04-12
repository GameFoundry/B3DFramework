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

		/** Scans the provided hierarchy for any prefab instances, and updates them to the latest prefab data. */
		static void UpdateAllInstancesFromPrefabs(const HSceneObject& sceneObject);

		/**
		 * Updates the provided scene object hierarchy with latest data from the associated provided prefab. Provided scene object
		 * hierarchy must be a part of an instance of the provided prefab. If the provided object is not the instance hierarchy root,
		 * root will be found and updated.
		 */
		static void UpdateInstanceFromPrefab(const HSceneObject& sceneObject);

		/**
		 * Updates the provided scene object hierarchy with latest data from the provided prefab. Provided scene object
		 * hierarchy must be a root object of an instance of the provided prefab. Returns null if nothing was updated.
		 */
		static HSceneObject UpdateInstanceFromPrefab(const HSceneObject& instance, const Prefab& prefab);

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

		/**
		 * Iterates over the provided scene object hierarchy and records a map of prefab object id -> game object id for each scene object and
		 * component in the hierarchy.
		 *
		 * @param		sceneObject			Scene object at which to start iterating
		 * @param		visitChildPrefabs	If false, iteration into child scene objects will stop if they belong to another prefab. Otherwise
		 *									we iterate until leaf of the hierarchy is reached.
		 * @return							Generated prefab object id -> game object id map.
		 */
		static UnorderedMap<UUID, UUID> GetPrefabToInstanceIdMap(const HSceneObject& sceneObject, bool visitChildPrefabs);
	private:
		/** Scans the provided hierarchy for any prefab instances, and updates them to the latest prefab data. */
		static void UpdateAllInstancesFromPrefabsRecursive(const HSceneObject& instanceRoot, FrameUnorderedMap<UUID, HPrefab>& inOutPrefabCache, FrameVector<UUID>& inOutParentPrefabChain);
	};

	/** @} */
} // namespace bs
