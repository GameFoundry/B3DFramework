//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIButtonBase.h"
#include "BsGUIToggleGroup.h"
#include "BsImageSprite.h"
#include "BsTextSprite.h"
#include "BsGUIContent.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * GUI element that accepts a drag and drop operation of a specified type.
	 *
	 * @see		DragAndDropManager
	 */
	class BS_ED_EXPORT GUIDropButton : public GUIButtonBase
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI drop button element.
		 *
		 * @param[in]	dragType		Identifier of the drag operation to accept.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIDropButton* create(UINT32 dragType, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI drop button element.
		 *
		 * @param[in]	dragType		Identifier of the drag operation to accept.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIDropButton* create(UINT32 dragType, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);

		/** Triggered when drag and drop operation finishes while over the button and is of the correct type. */
		Event<void(void*)> onDataDropped; 
	protected:
		virtual ~GUIDropButton();

	protected:
		GUIDropButton(UINT32 dragType, const String& styleName, const GUIDimensions& dimensions);

		/** @copydoc GUIButtonBase::_mouseEvent */
		virtual bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIButtonBase::_acceptDragAndDrop */
		virtual bool _acceptDragAndDrop(const Vector2I position, UINT32 typeId) const override;

		UINT32 mDragType;
	};

	/** @} */
}