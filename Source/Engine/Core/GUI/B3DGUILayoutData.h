//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGUIUnits.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Contains all information that is calculated during GUI element update layout pass. */
	struct GUILayoutData
	{
		GUILayoutData()
		{
			SetPanelDepth(0);
		}

		/**	Set widget part of element depth (Most significant part). */
		void SetWidgetDepth(u8 widgetDepth)
		{
			u32 shiftedDepth = widgetDepth << 24;

			Depth = shiftedDepth | (Depth & 0x00FFFFFF);
		}

		/** Set panel part of element depth. Less significant than widget depth but more than custom element depth. */
		void SetPanelDepth(i16 panelDepth)
		{
			u32 signedDepth = ((i32)panelDepth + 32768) << 8;

			Depth = signedDepth | (Depth & 0xFF0000FF);
		}

		/**	Retrieve widget part of element depth (Most significant part). */
		u8 GetWidgetDepth() const
		{
			return (Depth >> 24) & 0xFF;
		}

		/** Retrieve panel part of element depth. Less significant than widget depth but more than custom element depth. */
		i16 GetPanelDepth() const
		{
			return (((i32)Depth >> 8) & 0xFFFF) - 32768;
		}

		GUILogicalPoint RelativePosition{kZeroTag}; /**< Coordinates relative to the parent GUI element. Set during UpdateLayout pass. */
		GUILogicalSize Size{kZeroTag}; /**< Size of the GUI element in pixels. */
		u32 Depth = 0;
		u16 DepthRangeMin = -1;
		u16 DepthRangeMax = -1;
	};

	/** @} */
} // namespace b3d
