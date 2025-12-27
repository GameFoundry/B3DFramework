//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIWidget.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	GUI widget that renders a tooltip overlaid over other GUI elements. */
	class B3D_EXPORT GUITooltip : public GUIWidget
	{
	public:
		/**
		 * Creates a new tooltip widget.
		 *
		 * @param	parent			Parent scene object to attach the tooltip to.
		 * @param	overlaidWidget	Widget over which to overlay the tooltip.
		 * @param	position		Position of the tooltip, relative to the overlaid widget position.
		 * @param	text			Text to display in the tooltip.
		 */
		GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const GUIPhysicalPoint& position, const String& text);
		~GUITooltip() = default;

	protected:
		void OnCreated() override;

	private:
		static constexpr const char* kBackgroundStyleClass = "TooltipFrame";
		static const GUILogicalUnit kTooltipWidth;
		static const GUILogicalUnit kCursorSize;

		GUIPhysicalPoint mPosition;
		String mText;
	};

	/** @} */
} // namespace b3d
