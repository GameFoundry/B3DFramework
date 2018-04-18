//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsCGUIWidget.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	GUI widget that renders a tooltip overlaid over other GUI elements. */
	class BS_EXPORT GUITooltip : public CGUIWidget
	{
	public:
		/**	Name of the style used for tooltip's background frame. */
		static String getFrameStyleName();

		/**
		 * Creates a new tooltip widget.
		 *
		 * @param[in]	parent			Parent scene object to attach the tooltip to.
		 * @param[in]	overlaidWidget	Widget over which to overlay the tooltip.
		 * @param[in]	position		Position of the tooltip, relative to the overlaid widget position.
		 * @param[in]	text			Text to display in the tooltip.
		 */
		GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const Vector2I& position, const String& text);
		~GUITooltip();

	private:
		static const UINT32 TOOLTIP_WIDTH;
		static const UINT32 CURSOR_SIZE;
	};

	/** @} */
}