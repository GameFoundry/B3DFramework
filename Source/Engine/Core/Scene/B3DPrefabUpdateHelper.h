//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**
	 * Helper class that updates a prefab from scene object hierarchy, and makes sure the scene object hierarchy has correct
	 * prefab links to the prefabs that were updated.
	 */
	class B3D_EXPORT PrefabUpdateHelper
	{
	public:
		/**
		 * Updates the provided prefab resource so it stores a copy of the provided scene hierarchy. Provided
		 * scene hierarchy is made to be an instance of the prefab, if not already an instance of the provided
		 * prefab, or any other prefab. All other currently loaded instances of @p prefab will be updated to
		 * match the updated hierarchy.
		 *
		 * @code
		 * At the highest level this method performs the following actions:
		 * 1. Updates the provided prefab
		 * 2. Ensures that parent prefab instances of @p sceneObjectToUpdateWith are updated, and prefab links are kept consistent
		 *   - This means iterating bottom up from the prefab we updated, to the root instance
		 *   - This means this method may update objects that are parents of @p sceneObjectToUpdateWith
		 *   - This is needed to ensure prefab object IDs are kept valid throughout the hierarchy
		 *   - Prefab object ID remapping table is needed for the last prefab instance referencing an object in @p sceneObjectToUpdateWith (See below)
		 * 3. Updates all loaded instances of the prefab we updated
		 * @endcode
		 *
		 * @code
		 * #2 requires further explanation. Suppose we have a scene and two prefabs such as this:
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB2***
		 *   PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 * We then add an new object as a child of PFB2 Instance:
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *         Optional object [Game Object ID = OBI3, Prefab Object ID = None, Prefab Resource ID = None]
		 *
		 * We update PFB1 with PFB1 Instance:
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *       Optional object [Game Object ID = OB13, Prefab Object ID = OB13, Prefab Resource ID = PFB2]
		 *
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *         Optional object [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
		 *
		 * If we then update PFB2 with PFB2 Instance:
		 *   ***PFB2***
		 *   PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *     Optional object [Game Object ID = OB22, Prefab Object ID = OB22, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *       Optional object [Game Object ID = OB13, Prefab Object ID = OB22, Prefab Resource ID = PFB2]
		 *
		 * When updating PFB1, we had to assign prefab object ID to OBI3 (From None, to OB13)
		 * When updating PFB2, we also had to update PFB1, and change prefab object ID of OB13 from OB13 to OB22.
		 * @encode
		 *
		 * @code
		 * #2 is also required when adding nested prefabs. Suppose we have a scene such as this:
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB2***
		 *   PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB3***
		 *   PFB3 Instance [Game Object ID = OB31, Prefab Object ID = OB31, Prefab Resource ID = PFB3]
		 *
		 * We then add an instance of PFB3 as a child to PFB2 Instance
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *         PFB3 Instance [Game Object ID = OBI3, Prefab Object ID = OB31, Prefab Resource ID = PFB2]
		 *
		 * And the proceed to update PFB2 with PFB2 Instance:
		 *   ***PFB2***
		 *   PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *     PFB3 Instance [Game Object ID = OB22, Prefab Object ID = OB31, Prefab Resource ID = PFB3]
		 *
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *       PFB3 Instance [Game Object ID = OB13, Prefab Object ID = OB22, Prefab Resource ID = PFB2]
		 *
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *         PFB3 Instance [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
		 *
		 * When updating PFB2, we had to update both PFB1 and Scene:
		 *  - PFB1 has been updated with the new version of PFB2, adding PFB3 Instance [Game Object ID = OB13, Prefab Object ID = OB33, Prefab Resource ID = PFB2]
		 *  - Scene has been updated so that PFB3 Instance now links to PFB1, i.e. PFB3 Instance [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
		 *   - To do this, internally we had to track the PFB2 update that mapped OBI3 to OB22, and then PFB1 update that mapped OB22 -> OB13. Knowing this we can remap OBI3 -> OB22 -> OB13.
		 *
		 * This means the code below must keep careful track of the prefab object IDs as we're updating the prefab hierarchy chain.
		 * @endcode
		 */
		static void UpdatePrefab(const HPrefab& prefabToUpdate, const HSceneObject& sceneObjectToUpdateWith);

	private:
		/** Contains a reference to a scene object in some prefab's internal hierarchy and the prefab the scene object is part of. */
		struct ObjectInPrefab
		{
			ObjectInPrefab(HPrefab prefab, HGameObject instanceInPrefab)
				: Prefab(prefab), InstanceInPrefab(instanceInPrefab)
			{ }

			HPrefab Prefab;
			HGameObject InstanceInPrefab;
		};

		/** Looks up a counterpart of the provided game object in the specified prefab. Under the hood loads the prefab as required. */
		static TOptional<ObjectInPrefab> FindInstanceInPrefab(const GameObjectHandle& gameObject, const UUID& prefabResourceId);

		/** Looks up a counterpart of the provided scene object in the prefab it is an instance of. Under the hood loads the prefab as required. */
		static TOptional<ObjectInPrefab> FindInstanceInPrefab(const HSceneObject& sceneObject);

		/** Looks up a counterpart of the provided component in the prefab it is an instance of. Under the hood loads the prefab as required. */
		static TOptional<ObjectInPrefab> FindInstanceInPrefab(const HComponent& component);

		/**
		 * Iterates the provided hierarchy and for each object visits the prefab resource as referenced by the object. If the visited object
		 * itself contains a prefab link, its prefab resource is visited as well, recursively. Returns the game object ID within the last prefab that
		 * is referencing the object, excluding the prefabs that own the object (cases where object links to its own prefab).
		 *
		 * @code
		 * e.g. if we have this hierarchy:
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *         Optional object [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
		 *
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *       Optional object [Game Object ID = OB13, Prefab Object ID = OB13, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB2***
		 *   PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 * For Scene/Optional object, this method would return { OBI3 -> OB13 }. As PFB1 is the last object
		 * that links to Optional object.
		 *
		 * Another example with a nested prefab:
		 *   ***Scene***
		 *   Root
		 *     PFB1 Instance [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *       PFB2 Instance [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
		 *         PFB3 Instance [Game Object ID = OBI3, Prefab Object ID = OB31, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB1***
		 *   PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
		 *     PFB2 Instance [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB2***
		 *   PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
		 *
		 *   ***PFB3***
		 *   PFB3 Instance [Game Object ID = OB31, Prefab Object ID = OB31, Prefab Resource ID = PFB3]
		 *
		 * For Scene/PFB3 Instance this method would return { OBI3 -> OBI3 }. Even though PFB3 is the last prefab to reference the object,
		 * it's a self reference. Instead we return the parent - which in this case is the scene itself as neither PFB1 or PFB2 reference
		 * the object. This ensures that if we ever update PFB2 or PFB3 with the object, the prefab object ID of OBI3 is the one that
		 * gets remapped.
		 * @endcode
		 */
		static UnorderedMap<UUID, UUID> FindInstanceIdsThatNeedRemapping(const HSceneObject& root);
	};

	/** @} */
} // namespace b3d
