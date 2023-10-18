//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsIGUIVectorPathBuilder.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Builds a vector path that draws a rectangle (optionally rounded) with a border. */
	class B3D_EXPORT GUIBackgroundVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetStateRule& style) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIBackgroundVectorPathBuilder* Get()
		{
			static const GUIBackgroundVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a checkmark. */
	class B3D_EXPORT GUICheckmarkVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetStateRule& style) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIBackgroundVectorPathBuilder* Get()
		{
			static const GUIBackgroundVectorPathBuilder instance;
			return &instance;
		}
	};

	/** @} */
} // namespace bs
