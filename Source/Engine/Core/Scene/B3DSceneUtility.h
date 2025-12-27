//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DGameObject.h"

namespace b3d
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Performs various scene specific operations. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Scene), API(Editor), Static) SceneUtility
	{
	public:
		/**
		 * Traverses the object hierarchy, finds all child objects and components and records their instance data. Instance data essentially holds the object's "identity"
		 * and by restoring it we ensure any handles pointing to the object earlier will still point to the new version.
		 *
		 * @param	root	Root object to traverse and record.
		 * @return			A map of object IDs to instance data. 
		 */
		static UnorderedMap<UUID, SPtr<GameObjectInstanceData>> RecordSceneObjectHierarchyInstanceData(const HSceneObject& root);

		/**
		 * Restores instance data in the provided hierarchy, using object ids to determine what data maps to which objects.
		 *
		 * @param	root			Root object to traverse and restore.
		 * @param	instanceData	A map of object IDs to instance data, as output by RecordSceneObjectHierarchyInstanceData() method.
		 */
		static void RestoreSceneObjectHierarchyInstanceData(const HSceneObject& root, const UnorderedMap<UUID, SPtr<GameObjectInstanceData>>& instanceData);
	};

	/** @} */
} // namespace b3d
