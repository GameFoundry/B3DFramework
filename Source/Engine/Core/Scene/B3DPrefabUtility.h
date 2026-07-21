//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DGameObject.h"

namespace b3d
{
	/** @addtogroup Scene-Internal
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

	/** Allows various prefab utilities to retrieve and cache prefabs while performing operations. */
	struct PrefabCache
	{
		/** Attempts to find a prefab with the provided ID in the cache. If not found prefab will be loaded and added to cache. */
		HPrefab FindOrLoadPrefab(const UUID& prefabId);

		/** Adds a previously loaded prefab to cache. */
		void AddToCache(const HPrefab& prefab);

		/** Checks does the prefab exist in cache. */
		bool ExistsInCache(const UUID& prefabId);

	private:
		FrameUnorderedMap<UUID, HPrefab> mPrefabs;
	};

	/** @} */

	/** @addtogroup Scene
	 *  @{
	 */

	/** Performs various prefab specific operations. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Scene), Assembly(Editor), Static) PrefabUtility
	{
	public:
		/**
		 * Remove any instance specific changes to the object or its hierarchy from the provided prefab instance and
		 * restore it to the exact copy of the linked prefab.
		 */
		B3D_SCRIPT_EXPORT()
		static void RevertToPrefab(const HSceneObject& sceneObject);

		/**
		 * Updates the provided prefab resource so it stores a copy of the provided scene hierarchy. Provided
		 * scene hierarchy is made to be an instance of the prefab, if not already an instance of the provided
		 * prefab, or any other prefab. All other currently loaded instances of @p prefab will be updated to
		 * match the updated hierarchy.
		 */
		B3D_SCRIPT_EXPORT()
		static void UpdatePrefab(B3D_NO_RREF const HPrefab& prefabToUpdate, const HSceneObject& sceneObjectToUpdateWith);

		/** @name Internal
		 *  @{
		 */

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
		 * Iterates the hierarchy in @p root, and if a scene object or component is found in @p remappingTable, assigns it the prefab object ID
		 * from the remapping table, and provided @p prefabId as the prefab resource ID.
		 */
		static void RemapPrefabInstanceIds(const HSceneObject& root, const UnorderedMap<UUID, UUID>& remappingTable, const UUID& prefabId);

		/**
		 * Iterates the hierarchy in @p root, and if a scene object or component is found in @p remappingTable, assigns it the prefab object ID
		 * and prefab resource ID from the remapping table.
		 */
		static void RemapPrefabInstanceIds(const HSceneObject& root, const UnorderedMap<UUID, PrefabLinkInformation>& remappingTable);

		/**
		 * Clears all prefab IDs in the provided object and its children (includes both the prefab object and prefab resource IDs).
		 *
		 * @note	If any of its children belong to another prefab they will not be cleared.
		 */
		static void ClearPrefabIds(const HSceneObject& sceneObject);

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

		/** @} */
	private:
		friend class Prefab;
		friend class PrefabUpdateHelper;

		/**
		 * Scans the provided hierarchy for any prefab instances, loads their prefab resources and checks if prefab resources
		 * have any new changes since the instance was created. If they do, the instance is updated with latest information
		 * from the prefab resource.
		 *
		 * Returns true if any changes were made.
		 */
		static bool UpdateNestedPrefabInstances(const HSceneObject& sceneObject);

		/** Scans the provided hierarchy for any prefab instances, and updates them to the latest prefab data. */
		static bool UpdateNestedPrefabInstancesRecursive(const HSceneObject& root, PrefabCache& inOutPrefabCache);

		/**
		 * Updates the provided scene object hierarchy with latest data from the provided prefab. Provided scene object
		 * hierarchy must be a root object of an instance of the provided prefab. Returns null if nothing was updated.
		 */
		static HSceneObject UpdateInstanceFromPrefab(const HSceneObject& instance, const Prefab& prefab);
	};

	/** @} */
} // namespace b3d
