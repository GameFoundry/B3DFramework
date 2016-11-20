//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsInputFwd.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup Input-Internal
	 *  @{
	 */

	/**
	 * Contains relative and absolute position of an input axis. Relative state represents the difference between current 
	 * and last state.
	 */
	struct RawAxisState
	{
		RawAxisState()
		 :rel(0.0f), abs(0.0f)
		{ }

		float rel;
		float abs;
	};

	/**
	 * Represents a specific way of acquiring low-level input. Input class (which provides a higher level input) must have 
	 * at least one RawInputHandler attached. Raw input handlers receive input as sent by the hardware without OS 
	 * modifications.
	 */
	class BS_CORE_EXPORT RawInputHandler
	{
	public:
		RawInputHandler()
			:mMouseSmoothingEnabled(false) 
		{}

		virtual ~RawInputHandler() {}

		/**
		 * Triggered when user presses a button. Parameters include device index, button code of the pressed button, 
		 * and a timestamp of the button press event.
		 */
		Event<void(UINT32, ButtonCode, UINT64)> onButtonDown;

		/**
		 * Triggered when user releases a button. Parameters include device index, button code of the released button, 
		 * and a timestamp of the button release event.
		 */
		Event<void(UINT32, ButtonCode, UINT64)> onButtonUp;

		/**
		 * Triggered whenever the specified axis state changes. Parameters include device index, axis state data, and axis 
		 * type.
		 */
		Event<void(UINT32, const RawAxisState&, UINT32)> onAxisMoved;

		/** Called once per frame. Capture input here if needed. */
		virtual void _update() {}

		/**
		 * Called whenever the active window changes.
		 *
		 * @param[in]	win	Newly active window.
		 */
		virtual void _inputWindowChanged(const RenderWindow& win) {}

		/** Enables or disables mouse smoothing. Smoothing makes the changes to mouse axes more gradual. */
		void setMouseSmoothing(bool enabled) { mMouseSmoothingEnabled = enabled; }

	protected:
		bool mMouseSmoothingEnabled;
	};

	/** @} */
}