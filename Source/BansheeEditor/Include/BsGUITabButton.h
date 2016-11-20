//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIToggle.h"
#include "BsGUIToggleGroup.h"
#include "BsImageSprite.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/**
	 * Specialization of a GUIToggle element used for displaying tabs in a GUITabbedTitleBar. Aside from being toggleable
	 * these buttons also track drag and drop operations.
	 */
	class GUITabButton : public GUIToggle
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI tab button element.
		 *
		 * @param[in]	toggleGroup		A toggle group that forms a link between related tab buttons.
		 * @param[in]	index			Unique index that can be used for identifying a tab.
		 * @param[in]	text			Label to display in the button.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITabButton* create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, const HString& text, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI tab button element.
		 *
		 * @param[in]	toggleGroup		A toggle group that forms a link between related tab buttons.
		 * @param[in]	index			Unique index that can be used for identifying a tab.
		 * @param[in]	text			Label to display in the button.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITabButton* create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, const HString& text, 
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);
	
		/**
		 * Creates a new GUI tab button element.
		 *
		 * @param[in]	toggleGroup		A toggle group that forms a link between related tab buttons.
		 * @param[in]	index			Unique index that can be used for identifying a tab.
		 * @param[in]	content			Content to display in the button.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITabButton* create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, const GUIContent& content, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI tab button element.
		 *
		 * @param[in]	toggleGroup		A toggle group that forms a link between related tab buttons.
		 * @param[in]	index			Unique index that can be used for identifying a tab.
		 * @param[in]	content			Content to display in the button.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITabButton* create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, const GUIContent& content, 
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**	Returns the unique index for this tab button. */
		UINT32 getIndex() const { return mIndex; }

		/** @copydoc GUIToggle::toggleOn */
		void toggleOn() override;

		/** @copydoc GUIToggle::toggleOff */
		void toggleOff() override;

		/** Changes the button state to dragged or not dragged, resulting primarily in a visual change. */
		void _setDraggedState(bool active);

		/**
		 * Triggered when the user starts dragging the tab button. Reported parameters are the unique index of the tab and
		 * pointer position relative to parent GUIWidget.
		 */
		Event<void(UINT32, const Vector2I&)> onDragged; 

		/**
		 * Triggered when the user ends dragging the tab button. Reported parameters are the unique index of the tab and
		 * pointer position relative to parent GUIWidget.
		 */
		Event<void(UINT32, const Vector2I&)> onDragEnd;
	protected:
		GUITabButton(const String& styleName, const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, const GUIContent& content, const GUIDimensions& dimensions);

		/** @copydoc GUIElement::_mouseEvent */
		virtual bool _mouseEvent(const GUIMouseEvent& ev) override;

		UINT32 mIndex;
		Vector2I mDragStartPosition;
		bool mDraggedState;
		GUIElementState mInactiveState;

		static const UINT32 DRAG_MIN_DISTANCE;
	};

	/** @} */
}