//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Manages displaying tooltips over GUI elements. */
	class BS_EXPORT GUITooltipManager : public Module<GUITooltipManager>
	{
	public:
		~GUITooltipManager();

		/**
		 * Shows a tooltip at the specified location. This will replace any previously shown tooltip.
		 *
		 * @param[in]	widget		GUI widget over which to display the tooltip.
		 * @param[in]	position	Position of the tooltip, relative to the parent GUI widget.
		 * @param[in]	text		Text to display on the tooltip.
		 */
		void show(const GUIWidget& widget, const Vector2I& position, const String& text);

		/**	Hides the currently shown tooltip. */
		void hide();

	private:
		HSceneObject mTooltipSO;
	};

	/** @} */
}