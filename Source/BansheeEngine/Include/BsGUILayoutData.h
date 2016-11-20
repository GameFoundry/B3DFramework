//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRect2I.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**
	 * Contains all attributes that are output by GUI layouts and assigned to GUI elements. This includes element position,
	 * size and depth.
	 */
	struct BS_EXPORT GUILayoutData
	{
		GUILayoutData()
			:depth(0), depthRangeMin(-1), depthRangeMax(-1)
		{ 
			setPanelDepth(0);
		}

		/**	Set widget part of element depth (Most significant part). */
		void setWidgetDepth(UINT8 widgetDepth)
		{
			UINT32 shiftedDepth = widgetDepth << 24;

			depth = shiftedDepth | (depth & 0x00FFFFFF);
		}

		/** Set panel part of element depth. Less significant than widget depth but more than custom element depth. */
		void setPanelDepth(INT16 panelDepth)
		{
			UINT32 signedDepth = ((INT32)panelDepth + 32768) << 8;

			depth = signedDepth | (depth & 0xFF0000FF);;
		}

		/**	Retrieve widget part of element depth (Most significant part). */
		UINT8 getWidgetDepth() const
		{
			return (depth >> 24) & 0xFF;
		}

		/** Retrieve panel part of element depth. Less significant than widget depth but more than custom element depth. */
		INT16 getPanelDepth() const
		{
			return (((INT32)depth >> 8) & 0xFFFF) - 32768;
		}

		/**	Returns a clip rectangle that is relative to the current bounds. */
		Rect2I getLocalClipRect() const
		{
			Rect2I localClipRect = clipRect;
			localClipRect.x -= area.x;
			localClipRect.y -= area.y;

			return localClipRect;
		}

		Rect2I area;
		Rect2I clipRect;
		UINT32 depth;
		UINT16 depthRangeMin;
		UINT16 depthRangeMax;
	};

	/** @} */
}