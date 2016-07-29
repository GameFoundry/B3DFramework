//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsAABox.h"

namespace BansheeEngine
{
	/** @addtogroup Utility-Editor
	 *  @{
	 */

	/**	Contains miscellaneous helper methods. */
	class BS_ED_EXPORT EditorUtility
	{
	public:
		/**
		 * Calculates world space bounds of the specified scene object. This will consider components with bounds like
		 * Renderable.
		 */
		static AABox calculateBounds(const HSceneObject& object);

		/**
		 * Calculates world space bounds of the specified scene objects. This will consider components with bounds like
		 * Renderable.
		 */
		static AABox calculateBounds(const Vector<HSceneObject>& objects);

		/** Calculates world space center of the specified scene objects. */
		static Vector3 calculateCenter(const Vector<HSceneObject>& objects);

	private:
		/**
		 * Retrieves all components containing meshes on the specified object and outputs their bounds.
		 *
		 * @param[in]	object	Object to calculate bounds for.
		 * @param[in]	bounds	Output bounds, if successful.
		 * @return				True if a mesh component was found, otherwise false (bounds will not be updated).
		 */
		static bool calculateMeshBounds(const HSceneObject& object, AABox& bounds);
	};

	/** @} */
}