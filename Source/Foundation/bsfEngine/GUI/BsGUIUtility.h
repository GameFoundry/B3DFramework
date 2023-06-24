//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Helper class that performs various operations related to GUI. */
	class B3D_EXPORT GUIUtility
	{
	public:
		/**
		 * Calculates optimal size of a GUI element. This is the size that allows the GUI element to properly display all of
		 * its content.
		 */
		static Vector2I CalcOptimalSize(const GUIElementBase* elem);

		/**
		 * Calculates the size of elements in a layout of the specified size.
		 *
		 * @param[in]	width				Width of the layout.
		 * @param[in]	height				Height of the layout.
		 * @param[in]	layout				Parent layout of the children to calculate the area for.
		 * @param[in]	updateOptimalSizes	Optimization (doesn't change the results). Set to false if
		 *									GUIElementBase::_updateOptimalLayoutSizes was already called and optimal sizes
		 *									are up to date to avoid recalculating them. (Normally that is true if this is
		 *									being called during GUI layout update)
		 */
		static Vector2I CalcActualSize(u32 width, u32 height, GUILayout* layout, bool updateOptimalSizes = true);

	private:
		/**
		 * Calculates the size of elements in a layout of the specified size. Assumes the layout and all its children
		 * have updated optimal sizes.
		 *
		 * @param[in]	width				Width of the layout.
		 * @param[in]	height				Height of the layout.
		 * @param[in]	layout				Parent layout of the children to calculate the area for.
		 */
		static Vector2I CalcActualSizeInternal(u32 width, u32 height, GUILayout* layout);
	};

	/** @} */
} // namespace bs
