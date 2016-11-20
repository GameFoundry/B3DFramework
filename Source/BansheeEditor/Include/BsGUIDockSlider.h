//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIButtonBase.h"
#include "BsDockManager.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * GUI element used by the dock manager to display a slider between two different dock containers. The slider can be
	 * vertical or horizontal and will process and report mouse drag events.
	 *
	 * @note	It's up to the caller to actually handle the drag events.
	 */
	class GUIDockSlider : public GUIButtonBase
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI dock slider element.
		 *
		 * @param[in]	horizontal		Should the slider be rendered horizontal or vertical.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIDockSlider* create(bool horizontal, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI dock slider element.
		 *
		 * @param[in]	horizontal		Should the slider be rendered horizontal or vertical.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIDockSlider* create(bool horizontal, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/** Triggered while the user is dragging the slider (pointer down and being moved) */
		Event<void(const Vector2I&)> onDragged; 
	protected:
		/** @copydoc GUIButtonBase::_mouseEvent */
		virtual bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIButtonBase::_hasCustomCursor */
		bool _hasCustomCursor(const Vector2I position, CursorType& type) const override;
	private:
		Vector2I mLastDragPosition;
		bool mHorizontal;
		bool mDragInProgress;

		GUIDockSlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions);
	};

	/** @} */
}