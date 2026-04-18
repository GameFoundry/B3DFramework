//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DSize2.h"

namespace b3d
{
	struct GUIStyleSheetRules;

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Provides an interface that allows you to customize how GUIElement vector paths are constructed. */
	class B3D_EXPORT IGUIVectorPathBuilder
	{
	public:
		virtual ~IGUIVectorPathBuilder() = default;

		/**
		 * Constructs a vector path for a GUI element.
		 *
		 * @param		size		Size of the path to be displayed on the screen, in pixels.
		 * @param		rule		Current style sheet rule used by the GUI element.
		 * @return					New vector path.
		 */
		virtual HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& rule) const = 0;
	};

	/** @} */
} // namespace b3d
