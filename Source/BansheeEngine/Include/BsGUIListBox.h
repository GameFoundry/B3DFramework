//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIButtonBase.h"
#include "BsImageSprite.h"
#include "BsTextSprite.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** List box GUI element which when active opens a drop down selection with provided elements. */
	class BS_EXPORT GUIListBox : public GUIButtonBase
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new listbox with the provided elements.
		 *
		 * @param[in]	elements		Elements to display in the list box.
		 * @param[in]	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIListBox* create(const Vector<HString>& elements, bool multiselect = false, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new listbox with the provided elements.
		 *
		 * @param[in]	elements		Elements to display in the list box.
		 * @param[in]	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIListBox* create(const Vector<HString>& elements, bool multiselect, 
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new single-select listbox with the provided elements.
		 *
		 * @param[in]	elements		Elements to display in the list box.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIListBox* create(const Vector<HString>& elements, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);

		/**	Checks whether the listbox supports multiple selected elements at once. */
		bool isMultiselect() const { return mIsMultiselect; }

		/**	Changes the list box elements. */
		void setElements(const Vector<HString>& elements);

		/**	Makes the element with the specified index selected. */
		void selectElement(UINT32 idx);

		/**	Deselect element the element with the specified index. Only relevant for multi-select list boxes. */
		void deselectElement(UINT32 idx);

		/**	Returns states of all element in the list box (enabled or disabled). */
		const Vector<bool>& getElementStates() const { return mElementStates; }

		/**
		 * Sets states for all list box elements. Only valid for multi-select list boxes. Number of states must match number
		 * of list box elements.
		 */
		void setElementStates(const Vector<bool>& states);

		/**
		 * Triggered whenever user selects or deselects an element in the list box. Returned index maps to the element in
		 * the elements array that the list box was initialized with.
		 */
		Event<void(UINT32, bool)> onSelectionToggled;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIButtonBase::_getElementType */
		virtual ElementType _getElementType() const override { return ElementType::ListBox; }

		/** @} */
	protected:
		~GUIListBox();

	private:
		GUIListBox(const String& styleName, const Vector<HString>& elements, bool isMultiselect, 
			const GUIDimensions& dimensions);

		/** @copydoc GUIButtonBase::mouseEvent */
		virtual bool _mouseEvent(const GUIMouseEvent& ev) override;

		/**	Triggered when user clicks on an element. */
		void elementSelected(UINT32 idx);

		/**	Opens the list box drop down menu. */
		void openListBox();

		/**	Closes the list box drop down menu. */
		void closeListBox();

		/** Called when the list box drop down menu is closed by external influence. */
		void onListBoxClosed();

		/**	Updates visible contents depending on selected element(s). */
		void updateContents();

	private:
		Vector<HString> mElements;
		Vector<bool> mElementStates;
		GameObjectHandle<GUIDropDownMenu> mDropDownBox;

		bool mIsMultiselect;
	};

	/** @} */
}