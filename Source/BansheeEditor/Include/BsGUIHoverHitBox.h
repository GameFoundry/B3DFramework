//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElementContainer.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/** Helper class used for detecting when mousing over a certain and getting notified when that state changes. */
	class BS_ED_EXPORT GUIHoverHitBox : public GUIElementContainer
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Creates a new hover hit box that will detect mouse hover/out events over certain area. */
		static GUIHoverHitBox* create();

		/** Creates a new hover hit box that will detect mouse hover/out events over certain area. */
		static GUIHoverHitBox* create(const GUIOptions& options);

		/**	Triggered when pointer hovers over the hit box. */
		Event<void()> onHover;

		/**	Triggered when pointer that was previously hovering leaves the hit box. */
		Event<void()> onOut;

	private:
		GUIHoverHitBox(const GUIDimensions& dimensions);

		/** @copydoc GUIElementContainer::updateClippedBounds */
		void updateClippedBounds() override;

		/** @copydoc GUIElementContainer::_mouseEvent */
		virtual bool _mouseEvent(const GUIMouseEvent& ev) override;
	};

	/** @} */
}