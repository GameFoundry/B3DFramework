//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIUnits.h"
#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Manages displaying tooltips over GUI elements. */
	class B3D_EXPORT GUITooltipManager : public Module<GUITooltipManager>
	{
	public:
		~GUITooltipManager();

		/**
		 * Shows a tooltip at the specified location. This will replace any previously shown tooltip.
		 *
		 * @param	widget		GUI widget over which to display the tooltip.
		 * @param	position	Position of the tooltip, relative to the parent GUI widget.
		 * @param	text		Text to display on the tooltip.
		 */
		void Show(const GUIWidget& widget, const GUIPhysicalPoint& position, const String& text);

		/**	Hides the currently shown tooltip. */
		void Hide();

	private:
		HSceneObject mTooltipSO;
	};

	/** @} */
} // namespace b3d
