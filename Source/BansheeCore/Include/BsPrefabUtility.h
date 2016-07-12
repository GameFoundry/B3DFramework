//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGameObject.h"

namespace BansheeEngine
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Performs various prefab specific operations. */
	class BS_CORE_EXPORT PrefabUtility
	{
	private:
		/**	Contains saved Component instance data. */
		struct ComponentProxy
		{
			GameObjectInstanceDataPtr instanceData;
			UINT32 linkId;
		};

		/** Contains saved SceneObject instance data, as well as saved instance data for all its children and components. */
		struct SceneObjectProxy
		{
			GameObjectInstanceDataPtr instanceData;
			UINT32 linkId;

			Vector<ComponentProxy> components;
			Vector<SceneObjectProxy> children;
		};

	public:
		/**
		 * Remove any instance specific changes to the object or its hierarchy from the provided prefab instance and 
		 * restore it to the exact copy of the linked prefab.
		 *
		 * @param[in]	so	Object to revert.
		 */
		static void revertToPrefab(const HSceneObject& so);

		/**
		 * Updates all of the objects belonging to the same prefab instance as the provided object (if any). The update
		 * will apply any changes from the linked prefab to the hierarchy (if any).
		 *
		 * @param[in]	so	Object to update.
		 */
		static void updateFromPrefab(const HSceneObject& so);

		/**
		 * Generates prefab "link" ID that can be used for tracking which game object in a prefab instance corresponds to
		 * an object in the prefab.
		 *
		 * @note	If any children of the provided object belong to another prefab they will not have IDs generated.
		 */
		static UINT32 generatePrefabIds(const HSceneObject& sceneObject, UINT32 startingId);

		/**
		 * Clears all prefab "link" IDs in the provided object and its children.
		 *
		 * @param[in]	sceneObject		Prefab instance to clear the link IDs from.
		 * @param[in]	recursive		If true, all children of the provided scene object will be cleared as well.
		 * @param[in]	clearRoot		If true, the root provided object will have its link ID cleared. If false the root
		 *								object's components will have their IDs cleared but not the scene object itself.
		 *								(Child scene objects will have their link IDs cleared in case @p recursive is true.)
		 *
		 * @note	If any of its children belong to another prefab they will not be cleared.
		 */
		static void clearPrefabIds(const HSceneObject& sceneObject, bool recursive = true, bool clearRoot = true);

		/**
		 * Updates the internal prefab diff data by recording the difference between the current values in the provided
		 * prefab instance and its prefab.
		 *
		 * @note	
		 * If the provided object contains any child prefab instances, this will be done recursively for them as well.
		 */
		static void recordPrefabDiff(const HSceneObject& sceneObject);

	private:
		/**
	     * Traverses the object hierarchy, finds all child objects and components and records their instance data, as well
		 * as their original place in the hierarchy. Instance data essentially holds the object's "identity" and by 
		 * restoring it we ensure any handles pointing to the object earlier will still point to the new version.
		 *
		 * @param[in]	so					Object to traverse and record.
		 * @param[out]	output				Contains the output hierarchy of instance data.
		 * @param[out]	linkedInstanceData	A map of link IDs to instance data. Objects without link IDs will not be 
		 *									included here.
		 *
		 * @note	Does not recurse into child prefab instances.
		 */
		static void recordInstanceData(const HSceneObject& so, SceneObjectProxy& output, 
			UnorderedMap<UINT32, GameObjectInstanceDataPtr>& linkedInstanceData);

		/**
		 * Restores instance data in the provided hierarchy, using link ids to determine what data maps to which objects. 
		 *
		 * @param[in]	so					Object to traverse and restore the instance data.
		 * @param[in]	proxy				Hierarchy containing instance data for all objects and components, returned by
		 *									recordInstanceData() method.				
		 * @param[in]	linkedInstanceData	A map of link IDs to instance data, returned by recordInstanceData() method.
		 *
		 * @note	Does not recurse into child prefab instances.
		 */
		static void restoreLinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy, 
			UnorderedMap<UINT32, GameObjectInstanceDataPtr>& linkedInstanceData);

		/**
		 * Restores instance data in the provided hierarchy, but only for objects without a link id. Since the objects do
		 * not have a link ID we rely on their sequential order to find out which instance data belongs to which object.
		 *
		 * @param[in]	so		Object to traverse and restore the instance data.
		 * @param[in]	proxy	Hierarchy containing instance data for all objects and components, returned by
		 *						recordInstanceData() method.
		 *
		 * @note	Does not recurse into child prefab instances.
		 */
		static void restoreUnlinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy);
	};

	/** @} */
}