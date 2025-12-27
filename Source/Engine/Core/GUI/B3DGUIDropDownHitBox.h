//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElementContainer.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Helper class used for detecting when a certain area is in focus, and getting notified when that state changes. */
	class B3D_EXPORT GUIDropDownHitBox : public GUIElementContainer
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new drop down hit box that will detect mouse input over certain area.
		 * You must call setBounds() to define the area.
		 *
		 * @param	captureMouseOver	If true mouse over/out/move events will be captured by this control and wont be
		 *								passed to other GUI elements.
		 * @param	captureMousePresses	If true mouse clicks will be captured by this control and wont be passed
		 *								to other GUI elements.
		 */
		static GUIDropDownHitBox* Create(bool captureMouseOver, bool captureMousePresses);

		/**
		 * Creates a new drop down hit box that will detect mouse input over certain area. You must call setBounds() to
		 * define the area.
		 *
		 * @param	captureMouseOver	If true mouse over/out/move events will be captured by this control and wont be
		 *								passed to other GUI elements.
		 * @param	captureMousePresses	If true mouse clicks will be captured by this control and wont be passed to
		 *								other GUI elements.
		 * @param	options				Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 */
		static GUIDropDownHitBox* Create(bool captureMouseOver, bool captureMousePresses, const GUIOptions& options);

		/** Sets a single rectangle bounds in which the hitbox will capture mouse events. */
		void SetBounds(const GUIPhysicalArea& bounds);

		/** Sets complex bounds consisting of multiple rectangles in which the hitbox will capture mouse events. */
		void SetBounds(const Vector<GUIPhysicalArea>& bounds);

		/** Triggered when hit box loses focus (for example user clicks outside of its bounds). */
		Event<void()> OnFocusLost;

		/** Triggered when hit box gains focus (for example user clicks inside of its bounds). */
		Event<void()> OnFocusGained;

	private:
		GUIDropDownHitBox(bool captureMouseOver, bool captureMousePresses, const GUISizeConstraints& dimensions);

		bool DoOnCommandEvent(const GUICommandEvent& ev) override;
		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;
		bool IsInInteractionBounds(const GUIPhysicalPoint& position) const override;

		Vector<GUIPhysicalArea> mBounds;
		bool mCaptureMouseOver;
		bool mCaptureMousePresses;
	};

	/** @} */
} // namespace b3d
