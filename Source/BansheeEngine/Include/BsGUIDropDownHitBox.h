//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElementContainer.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Helper class used for detecting when a certain area is in focus, and getting notified when that state changes. */
	class BS_EXPORT GUIDropDownHitBox : public GUIElementContainer
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new drop down hit box that will detect mouse input over certain area.
		 * You must call setBounds() to define the area.
		 *
		 * @param[in]	captureMouseOver	If true mouse over/out/move events will be captured by this control and wont be
		 *									passed to other GUI elements.
		 * @param[in]	captureMousePresses	If true mouse clicks will be captured by this control and wont be passed
		 *									to other GUI elements.
		 */
		static GUIDropDownHitBox* create(bool captureMouseOver, bool captureMousePresses);

		/**
		 * Creates a new drop down hit box that will detect mouse input over certain area. You must call setBounds() to 
		 * define the area.
		 *
		 * @param[in]	captureMouseOver	If true mouse over/out/move events will be captured by this control and wont be
		 *									passed to other GUI elements.
		 * @param[in]	captureMousePresses	If true mouse clicks will be captured by this control and wont be passed to 
		 *									other GUI elements.
		 * @param[in]	options				Options that allow you to control how is the element positioned and sized.
		 *									This will override any similar options set by style.
		 */
		static GUIDropDownHitBox* create(bool captureMouseOver, bool captureMousePresses, const GUIOptions& options);

		/** Sets a single rectangle bounds in which the hitbox will capture mouse events. */
		void setBounds(const Rect2I& bounds);

		/** Sets complex bounds consisting of multiple rectangles in which the hitbox will capture mouse events. */
		void setBounds(const Vector<Rect2I>& bounds);

		/** Triggered when hit box loses focus (for example user clicks outside of its bounds). */
		Event<void()> onFocusLost;

		/** Triggered when hit box gains focus (for example user clicks inside of its bounds). */
		Event<void()> onFocusGained;

	private:
		GUIDropDownHitBox(bool captureMouseOver, bool captureMousePresses, const GUIDimensions& dimensions);

		/** @copydoc GUIElementContainer::updateClippedBounds */
		void updateClippedBounds() override;

		/** @copydoc GUIElementContainer::_commandEvent */
		bool _commandEvent(const GUICommandEvent& ev) override;

		/** @copydoc GUIElementContainer::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIElementContainer::_isInBounds */
		bool _isInBounds(const Vector2I position) const override;

		Vector<Rect2I> mBounds;
		bool mCaptureMouseOver;
		bool mCaptureMousePresses;
	};

	/** @} */
}